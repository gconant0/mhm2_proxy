#include <upcxx/upcxx.hpp>

#include <upcxx_utils/limit_outstanding.hpp>
#include <upcxx_utils/log.hpp>

//GCC
#include <cassert>
#include "math.h"

using upcxx::when_all;
using LimitedFutureQueue = std::deque<upcxx::future<> >;

LimitedFutureQueue &upcxx_utils::_get_outstanding_queue() {
  static LimitedFutureQueue outstanding_queue;
  return outstanding_queue;
}

upcxx::future<> upcxx_utils::collapse_outstanding_futures(int limit, LimitedFutureQueue &outstanding_queue, int max_check) {
  DBG("limit=", limit, " outstanding=", outstanding_queue.size(), " max_check=", max_check, "\n");
  upcxx::future<> returned_future = upcxx::make_future();
  if (outstanding_queue.size() >= limit) {
    // reduce to limit when over
    while (outstanding_queue.size() > limit) {
      auto fut = outstanding_queue.front();
      outstanding_queue.pop_front();
      if (!fut.is_ready()) returned_future = upcxx::when_all(fut, returned_future);
    }
    DBG("limit=", limit, " outstanding=", outstanding_queue.size(), " max_check=", max_check, "\n");
    if (limit == 0) {
      assert(outstanding_queue.empty());
    } else {
      assert(outstanding_queue.size() <= limit);
      int i = 0;
      while (i < max_check && !returned_future.is_ready() && i < outstanding_queue.size()) {
        // find a ready future in the queue to swap with
        auto &test_fut = outstanding_queue[i++];
        if (test_fut.is_ready()) {
          std::swap(test_fut, returned_future);
          assert(returned_future.is_ready());
          break;
        }
      }
    }
  }
  DBG("limit=", limit, " outstanding=", outstanding_queue.size(), " max_check=", max_check, ", ret=", returned_future.is_ready(),
      "\n");
  return returned_future;
}

void upcxx_utils::add_outstanding_future(upcxx::future<> fut, LimitedFutureQueue &outstanding_queue) {
  if (!fut.is_ready()) outstanding_queue.push_back(fut);
}

upcxx::future<> upcxx_utils::limit_outstanding_futures(int limit, LimitedFutureQueue &outstanding_queue) {
  return collapse_outstanding_futures(limit, outstanding_queue, std::min(limit / 4, (int)16));
}

upcxx::future<> upcxx_utils::limit_outstanding_futures(upcxx::future<> fut, int limit, LimitedFutureQueue &outstanding_queue) {
  if (limit < 0) limit = upcxx::local_team().rank_n() * 2;
  DBG("limit=", limit, " outstanding=", outstanding_queue.size(), "\n");
  if (limit == 0) {
    if (outstanding_queue.empty()) return fut;
    return upcxx::when_all(collapse_outstanding_futures(limit, outstanding_queue), fut);
  }
  if (fut.is_ready()) {
    if (outstanding_queue.size() <= limit) return fut;
  } else {
    outstanding_queue.push_back(fut);
  }
  return limit_outstanding_futures(limit, outstanding_queue);
}

upcxx::future<> upcxx_utils::flush_outstanding_futures_async(LimitedFutureQueue &outstanding_queue) {
  DBG(" outstanding=", outstanding_queue.size(), "\n");
  upcxx::future<> all_fut = collapse_outstanding_futures(0, outstanding_queue);
  return all_fut;
}

void upcxx_utils::flush_outstanding_futures(LimitedFutureQueue &outstanding_queue) {
  while (!outstanding_queue.empty()) flush_outstanding_futures_async(outstanding_queue).wait();
  assert(outstanding_queue.empty());
}
