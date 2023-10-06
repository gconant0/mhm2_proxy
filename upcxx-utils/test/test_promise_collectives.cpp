#include <random>
#include <vector>

#include "upcxx/upcxx.hpp"
#include "upcxx_utils/log.hpp"
#include "upcxx_utils/promise_collectives.hpp"

using std::vector;

using upcxx::barrier;
using upcxx::future;
using upcxx::make_future;
using upcxx::when_all;

using upcxx_utils::PromiseBarrier;
using upcxx_utils::roundup_log2;

int test_promise_barrier(int argc, char **argv) {
  barrier();
  upcxx_utils::open_dbg("test_promise_barrier");

  assert(roundup_log2(0) == -1);
  assert(roundup_log2(1) == 0);
  assert(roundup_log2(2) == 1);
  assert(roundup_log2(3) == 2);
  assert(roundup_log2(4) == 2);
  assert(roundup_log2(5) == 3);
  assert(roundup_log2(6) == 3);
  assert(roundup_log2(7) == 3);
  assert(roundup_log2(8) == 3);
  assert(roundup_log2(9) == 4);
  assert(roundup_log2(15) == 4);
  assert(roundup_log2(16) == 4);
  assert(roundup_log2(17) == 5);
  {
    PromiseBarrier pb;
    assert(!pb.get_future().ready());
    pb.fulfill();
    pb.get_future().wait();
  }
  barrier();
  {
    DBG("1s 2s 1e 2e\n");
    barrier();
    PromiseBarrier pb1, pb2;
    barrier();
    assert(!pb1.get_future().ready());
    pb1.fulfill();
    barrier();
    assert(!pb2.get_future().ready());
    pb2.fulfill();
    barrier();
    pb1.get_future().wait();
    barrier();
    pb2.get_future().wait();
    barrier();
  }
  {
    DBG("1s 1e 2s 2e\n");
    barrier();
    PromiseBarrier pb1, pb2;
    barrier();
    assert(!pb1.get_future().ready());
    pb1.fulfill();
    barrier();
    pb1.get_future().wait();
    barrier();
    assert(!pb2.get_future().ready());
    pb2.fulfill();
    barrier();
    pb2.get_future().wait();
    barrier();
  }
  {
    DBG("1s 2s 2e 1e\n");
    barrier();
    PromiseBarrier pb1, pb2;
    barrier();
    assert(!pb1.get_future().ready());
    pb1.fulfill();
    barrier();
    assert(!pb2.get_future().ready());
    pb2.fulfill();
    barrier();
    pb2.get_future().wait();
    barrier();
    pb1.get_future().wait();
    barrier();
  }

  {
    DBG("2s 1s 2e 1e\n");
    barrier();
    PromiseBarrier pb1, pb2;
    barrier();
    assert(!pb2.get_future().ready());
    pb2.fulfill();
    barrier();
    assert(!pb1.get_future().ready());
    pb1.fulfill();
    barrier();
    pb2.get_future().wait();
    barrier();
    pb1.get_future().wait();
    barrier();
  }
  {
    DBG("2s 2e 1s 1e\n");
    barrier();
    PromiseBarrier pb1, pb2;
    barrier();
    assert(!pb2.get_future().ready());
    pb2.fulfill();
    barrier();
    pb2.get_future().wait();
    barrier();
    assert(!pb1.get_future().ready());
    pb1.fulfill();
    barrier();
    pb1.get_future().wait();
    barrier();
  }
  {
    DBG("2s 1s 1e 2e\n");
    barrier();
    PromiseBarrier pb1, pb2;
    barrier();
    assert(!pb2.get_future().ready());
    pb2.fulfill();
    barrier();
    assert(!pb1.get_future().ready());
    pb1.fulfill();
    barrier();
    pb1.get_future().wait();
    barrier();
    pb2.get_future().wait();
    barrier();
  }

  std::mt19937 g(rank_n());  // seed all ranks the same
  {
    DBG("fulfill all barrier wait all same order\n");
    int iterations = 1000;
    vector<PromiseBarrier> pbs(iterations);
    vector<intrank_t> fulfill_order(iterations);
    vector<intrank_t> &wait_order = fulfill_order;
    for (int i = 0; i < iterations; i++) {
      fulfill_order[i] = i;
      wait_order[i] = i;
      assert(!pbs[i].get_future().ready());
    }
    std::shuffle(fulfill_order.begin(), fulfill_order.end(), g);
    barrier();
    // initiate all
    for (int i = 0; i < iterations; i++) {
      assert(!pbs[fulfill_order[i]].get_future().ready());
      pbs[fulfill_order[i]].fulfill();
    }
    // wait all
    future<> all_fut = make_future();
    for (int i = 0; i < iterations; i++) {
      auto fut = pbs[wait_order[i]].get_future();
      all_fut = when_all(all_fut, fut);
    }
    all_fut.wait();
    barrier();
  }

  {
    DBG("fulfill all barrier wait all different order\n");
    int iterations = 1000;
    vector<PromiseBarrier> pbs(iterations);
    vector<intrank_t> fulfill_order(iterations);
    vector<intrank_t> wait_order(iterations);
    for (int i = 0; i < iterations; i++) {
      fulfill_order[i] = i;
      wait_order[i] = i;
      assert(!pbs[i].get_future().ready());
    }
    std::shuffle(fulfill_order.begin(), fulfill_order.end(), g);
    std::shuffle(wait_order.begin(), wait_order.end(), g);
    barrier();
    // initiate all
    for (int i = 0; i < iterations; i++) {
      assert(!pbs[fulfill_order[i]].get_future().ready());
      pbs[fulfill_order[i]].fulfill();
    }
    barrier();
    // wait all
    future<> all_fut = make_future();
    for (int i = 0; i < iterations; i++) {
      auto fut = pbs[wait_order[i]].get_future();
      all_fut = when_all(all_fut, fut);
    }
    all_fut.wait();
    barrier();
  }

  {
    DBG("fulfill all barrier wait each same order\n");
    int iterations = 1000;
    vector<PromiseBarrier> pbs(iterations);
    vector<intrank_t> fulfill_order(iterations);
    vector<intrank_t> &wait_order = fulfill_order;
    for (int i = 0; i < iterations; i++) {
      fulfill_order[i] = i;
      wait_order[i] = i;
    }
    std::shuffle(fulfill_order.begin(), fulfill_order.end(), g);
    barrier();
    // initiate all
    for (int i = 0; i < iterations; i++) {
      assert(!pbs[fulfill_order[i]].get_future().ready());
      pbs[fulfill_order[i]].fulfill();
    }
    barrier();
    // wait each
    for (int i = 0; i < iterations; i++) {
      pbs[wait_order[i]].get_future().wait();
    }
    barrier();
  }

  {
    DBG("fulfill all barrier wait each different order\n");
    int iterations = 1000;
    vector<PromiseBarrier> pbs(iterations);
    vector<intrank_t> fulfill_order(iterations);
    vector<intrank_t> wait_order(iterations);
    for (int i = 0; i < iterations; i++) {
      fulfill_order[i] = i;
      wait_order[i] = i;
    }
    std::shuffle(fulfill_order.begin(), fulfill_order.end(), g);
    std::shuffle(wait_order.begin(), wait_order.end(), g);
    barrier();
    // initiate all
    for (int i = 0; i < iterations; i++) {
      assert(!pbs[fulfill_order[i]].get_future().ready());
      pbs[fulfill_order[i]].fulfill();
    }
    barrier();
    // wait each
    for (int i = 0; i < iterations; i++) {
      pbs[wait_order[i]].get_future().wait();
    }
    barrier();
  }

  {
    DBG("fulfill then wait each same order\n");
    int iterations = 1000;
    vector<PromiseBarrier> pbs(iterations);
    vector<intrank_t> fulfill_order(iterations);
    vector<intrank_t> &wait_order = fulfill_order;
    for (int i = 0; i < iterations; i++) {
      fulfill_order[i] = i;
      wait_order[i] = i;
    }
    std::shuffle(fulfill_order.begin(), fulfill_order.end(), g);
    barrier();
    // initiate all
    for (int i = 0; i < iterations; i++) {
      assert(!pbs[fulfill_order[i]].get_future().ready());
      pbs[fulfill_order[i]].fulfill();
      pbs[wait_order[i]].get_future().wait();
    }
    barrier();
  }

  DBG("fulfill then wait each different order would deadlock\n");

  {
    DBG("fulfill all then wait all same order\n");
    int iterations = 1000;
    vector<PromiseBarrier> pbs(iterations);
    vector<intrank_t> fulfill_order(iterations);
    vector<intrank_t> &wait_order = fulfill_order;
    for (int i = 0; i < iterations; i++) {
      fulfill_order[i] = i;
      wait_order[i] = i;
    }
    std::shuffle(fulfill_order.begin(), fulfill_order.end(), g);
    barrier();
    // initiate all
    future<> all_fut = make_future();
    for (int i = 0; i < iterations; i++) {
      assert(!pbs[fulfill_order[i]].get_future().ready());
      pbs[fulfill_order[i]].fulfill();
      auto fut = pbs[wait_order[i]].get_future();
      all_fut = when_all(all_fut, fut);
    }
    all_fut.wait();
    barrier();
  }

  {
    DBG("fulfill all then wait all different order\n");
    int iterations = 1000;
    vector<PromiseBarrier> pbs(iterations);
    vector<intrank_t> fulfill_order(iterations);
    vector<intrank_t> wait_order(iterations);
    for (int i = 0; i < iterations; i++) {
      fulfill_order[i] = i;
      wait_order[i] = i;
    }
    std::shuffle(fulfill_order.begin(), fulfill_order.end(), g);
    std::shuffle(wait_order.begin(), wait_order.end(), g);
    barrier();
    // initiate all
    future<> all_fut = make_future();
    for (int i = 0; i < iterations; i++) {
      assert(!pbs[fulfill_order[i]].get_future().ready());
      pbs[fulfill_order[i]].fulfill();
      auto fut = pbs[wait_order[i]].get_future();
      all_fut = when_all(all_fut, fut);
    }
    all_fut.wait();
    barrier();
  }

  upcxx_utils::close_dbg();
  barrier();
  return 0;
}

int test_promise_collectives(int argc, char **argv) {
  test_promise_barrier(argc, argv);
  return 0;
}