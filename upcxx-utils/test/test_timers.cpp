#include <chrono>
#include <thread>
#include <upcxx/upcxx.hpp>

#include "upcxx_utils/log.hpp"
#include "upcxx_utils/timers.hpp"

using namespace upcxx_utils;

int test_timers(int argc, char **argv) {
  upcxx_utils::open_dbg("test_timers");

  {
    BarrierTimer bt("Barrier");
    std::this_thread::sleep_for(std::chrono::milliseconds(rank_me() * 2));
  }

  {
    BaseTimer base("Base");
    base.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(rank_me() * 2));
    base.stop();
    base.done_all();
  }

  Timings::wait_pending();

  {
    // MinSumMax
    auto msm_i = min_sum_max_reduce_all(rank_me()).wait();
    DBG("Got msm_i=", msm_i.min, " ", msm_i.my, " ", msm_i.sum, " ", msm_i.max, " - ", msm_i.avg, "\n");
    assert(msm_i.min == 0);
    assert(msm_i.my == rank_me());
    assert(msm_i.max == rank_n() - 1);
    float sum = 0.0;
    for (int i = 0; i < rank_n(); i++) sum += i;
    assert(msm_i.avg > sum / rank_n() - 0.01 && msm_i.avg < sum / rank_n() + 0.01);
    assert(msm_i.is_valid());
  }
  {
    double factor = 3.14159;
    auto msm_f = min_sum_max_reduce_one(rank_me() * factor, 0).wait();
    DBG("Got msm_f=", msm_f.min, " ", msm_f.my, " ", msm_f.sum, " ", msm_f.max, " - ", msm_f.avg, "\n");
    assert(msm_f.my == rank_me() * factor);
    assert(msm_f.my > rank_me() * factor - 0.01 && msm_f.my < rank_me() * factor + 0.01);
    if (rank_me() == 0) {
      assert(msm_f.is_valid());
      assert(msm_f.min == 0.0);
      assert(msm_f.max > factor * (rank_n() - 1) - 0.01 && msm_f.max < factor * (rank_n() - 1) + 0.01);
    }
  }

  upcxx_utils::close_dbg();

  return 0;
}
