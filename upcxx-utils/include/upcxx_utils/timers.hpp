#pragma once

#include <chrono>
#include <ctime>
#include <upcxx/upcxx.hpp>

//GCC
#include <cassert>
#include "math.h"

#include "colors.h"
#include "log.hpp"
#include "version.h"

using upcxx::barrier;
using upcxx::intrank_t;
using upcxx::make_future;
using upcxx::op_fast_add;
using upcxx::op_fast_max;
using upcxx::op_fast_min;
using upcxx::progress;
using upcxx::rank_me;
using upcxx::rank_n;
using upcxx::reduce_all;
using upcxx::reduce_one;
using upcxx::to_future;
using upcxx::when_all;
using upcxx::world;

using std::make_shared;
using std::ostringstream;
using std::shared_ptr;
using std::string;
using std::stringstream;
using timepoint_t = std::chrono::time_point<std::chrono::high_resolution_clock>;
using std::chrono::seconds;
using duration_seconds = std::chrono::duration<double>;

namespace upcxx_utils {

template <typename T>
class MinSumMax {
 public:
  MinSumMax(T my_val = 0) { reset(my_val); }

  // construct a MinSumMax from another type and factor
  template <typename T2>
  MinSumMax(const T2 &other, T factor)
      : my(other.my * factor)
      , min(other.min * factor)
      , sum(other.sum * factor)
      , max(other.max * factor)
      , avg(other.avg * factor) {}

  void reset(T my_val) {
    my = my_val;
    min = my_val;
    sum = my_val;
    max = my_val;
    avg = 0.0;
  }

  string to_string() const {
    if (!is_valid()) return "NOT VALID MinSumMax";
    ostringstream oss;
    oss << std::setprecision(2) << std::fixed;
    double bal = (max != 0 ? avg / max : 1.0);
    oss << min << "/" << my << "/" << avg << "/" << max << ", bal=" << bal;
    auto s = oss.str();
    return s;
  }


  bool is_valid() const { return min <= my && min <= max && min <= avg && max >= my && max >= avg && sum >= fabs(avg); }

  struct op_MinSumMax {
    template <typename Ta, typename Tb>
    MinSumMax operator()(Ta a, Tb &&b) const {
      a.min = upcxx::op_fast_min(a.min, std::forward<T>(b.min));
      a.sum = upcxx::op_fast_add(a.sum, std::forward<T>(b.sum));
      a.max = upcxx::op_fast_max(a.max, std::forward<T>(b.max));
      return static_cast<MinSumMax &&>(a);
    };
  };
  void apply_avg(intrank_t n) { avg = n > 0 ? ((double)sum) / n : sum; }

  void apply_avg(const upcxx::team &team) { apply_avg(team.rank_n()); }

  T my, min, sum, max;
  double avg;
};
template <typename T>
ostream &operator<<(ostream &os, const MinSumMax<T> &msm) {
  return os << msm.to_string();
}

template <typename T>
upcxx::future<> min_sum_max_reduce_one(const MinSumMax<T> *msm_in, MinSumMax<T> *msm_out, int count, intrank_t root = 0,
                                       const upcxx::team &team = world()) {
  using MSM = MinSumMax<T>;
  typename MSM::op_MinSumMax op;
  vector<T> orig_vals(count);
  for (int i = 0; i < count; i++) {
    orig_vals[i] = msm_in[i].my;
  }
  return upcxx::reduce_one(msm_in, msm_out, count, op, root, team)
      .then([msm_out, count, &team, orig_vals = std::move(orig_vals)]() {
        for (auto i = 0; i < count; i++) {
          msm_out[i].apply_avg(team);
          msm_out[i].my = orig_vals[i];
        }
      });
}

template <typename T>
upcxx::future<MinSumMax<T>> min_sum_max_reduce_one(const T my, intrank_t root = 0, const upcxx::team &team = world()) {
  using MSM = MinSumMax<T>;
  auto sh_msm = make_shared<MSM>(my);
  return min_sum_max_reduce_one(sh_msm.get(), sh_msm.get(), 1, root, team).then([sh_msm, &team]() { return *sh_msm; });
}

template <typename T>
upcxx::future<> min_sum_max_reduce_all(const MinSumMax<T> *msm_in, MinSumMax<T> *msm_out, int count,
                                       const upcxx::team &team = world()) {
  using MSM = MinSumMax<T>;
  typename MSM::op_MinSumMax op;
  vector<T> orig_vals(count);
  for (int i = 0; i < count; i++) {
    orig_vals[i] = msm_in[i].my;
  }
  return upcxx::reduce_all(msm_in, msm_out, count, op, team).then([msm_out, count, &team, orig_vals = std::move(orig_vals)]() {
    for (auto i = 0; i < count; i++) {
      msm_out[i].apply_avg(team);
      msm_out[i].my = orig_vals[i];
    }
  });
}

template <typename T>
upcxx::future<MinSumMax<T>> min_sum_max_reduce_all(const T my, const upcxx::team &team = world()) {
  using MSM = MinSumMax<T>;
  auto sh_msm = make_shared<MSM>(my);
  return min_sum_max_reduce_all(sh_msm.get(), sh_msm.get(), 1, team).then([sh_msm, &team]() { return *sh_msm; });
}

// Reduce compile time by declaring extern templates of common types
// template instantiations happen in src/CMakeList and timers-extern-template.in.cpp

#define MACRO_MIN_SUM_MAX(TYPE, MODIFIER)                                                                                          \
  MODIFIER class MinSumMax<TYPE>;                                                                                                  \
  MODIFIER upcxx::future<> min_sum_max_reduce_one<TYPE>(const MinSumMax<TYPE> *, MinSumMax<TYPE> *, int, intrank_t,                \
                                                        const upcxx::team &);                                                      \
  MODIFIER upcxx::future<> min_sum_max_reduce_all<TYPE>(const MinSumMax<TYPE> *, MinSumMax<TYPE> *, int, const upcxx::team &team); \
  MODIFIER upcxx::future<MinSumMax<TYPE>> min_sum_max_reduce_one<TYPE>(const TYPE, intrank_t, const upcxx::team &);                \
  MODIFIER upcxx::future<MinSumMax<TYPE>> min_sum_max_reduce_all<TYPE>(const TYPE, const upcxx::team &team);

MACRO_MIN_SUM_MAX(float, extern template);
MACRO_MIN_SUM_MAX(double, extern template);
MACRO_MIN_SUM_MAX(int64_t, extern template);
MACRO_MIN_SUM_MAX(uint64_t, extern template);
MACRO_MIN_SUM_MAX(int, extern template);

class Timings;
using ShTimings = shared_ptr<Timings>;

class Timings {
  std::chrono::time_point<std::chrono::high_resolution_clock> t;
  std::chrono::time_point<std::chrono::high_resolution_clock> before, after;
  static upcxx::future<> &get_last_pending();

 public:
  double before_elapsed, after_elapsed, reduction_elapsed;
  size_t my_count, my_instance;
  MinSumMax<double> before_msm, after_msm, count_msm, instance_msm;

  Timings();

  static upcxx::future<> get_pending();

  static void set_pending(upcxx::future<> fut);

  static void wait_pending();

  string to_string(bool print_count = false, bool print_label = false) const;

  static void set_before(Timings &timings, size_t count, double elapsed, size_t instances = 0);

  // timings must remain in scope until the returned future is ready()
  static upcxx::future<> set_after(
      const upcxx::team &team, Timings &timings,
      std::chrono::time_point<std::chrono::high_resolution_clock> after = std::chrono::high_resolution_clock::now());
  static upcxx::future<> set_after(Timings &timings, std::chrono::time_point<std::chrono::high_resolution_clock> after =
                                                         std::chrono::high_resolution_clock::now()) {
    return set_after(upcxx::world(), timings, after);
  }

  // barrier and reduction
  static Timings barrier(const upcxx::team &team, size_t count, double elapsed, size_t instances = 0);
  static Timings barrier(size_t count, double elapsed, size_t instances = 0) {
    return barrier(upcxx::world(), count, elapsed, instances);
  }

  static void print_barrier_timings(const upcxx::team &team, string label);
  static void print_barrier_timings(string label) { print_barrier_timings(upcxx::world(), label); }

  // no barrier but a future reduction is started
  static upcxx::future<ShTimings> reduce(const upcxx::team &team, size_t count, double elapsed, size_t instances = 0);
  static upcxx::future<ShTimings> reduce(size_t count, double elapsed, size_t instances = 0) {
    return reduce(upcxx::world(), count, elapsed, instances);
  }

  static void print_reduce_timings(const upcxx::team &team, string label);
  static void print_reduce_timings(string label) { print_reduce_timings(upcxx::world(), label); }
};

class BaseTimer {
  // Just times between start & stop, does not print a thing
  // does not time construction / destruction

 private:
  static size_t &instance_count();

 protected:
  timepoint_t t;
  double t_elapsed;
  size_t count;
  string name;

  static void increment_instance();
  static void decrement_instance();
  static size_t get_instance_count();

 public:
  BaseTimer();
  BaseTimer(const string &_name);
  BaseTimer(const BaseTimer &copy) = default;
  BaseTimer(BaseTimer &&move) = default;
  BaseTimer &operator=(const BaseTimer &copy) = default;
  BaseTimer &operator=(BaseTimer &&move) = default;

  virtual ~BaseTimer();

  void clear();

  void start();

  void stop();

  double get_elapsed() const;

  double get_elapsed_since_start() const;

  size_t get_count() const;

  const string &get_name() const;

  void done() const;
  upcxx::future<MinSumMax<double>> done_all_async(const upcxx::team &tm = upcxx::world()) const;
  void done_all(const upcxx::team &tm = upcxx::world()) const;

  string get_final() const;

  upcxx::future<ShTimings> reduce_timings(const upcxx::team &team, size_t my_instances = 0) const;
  upcxx::future<ShTimings> reduce_timings(size_t my_instances = 0) const { return reduce_timings(upcxx::world(), my_instances); }

  static upcxx::future<ShTimings> reduce_timings(const upcxx::team &team, size_t my_count, double my_elapsed,
                                                 size_t my_instances = 0);
  static upcxx::future<ShTimings> reduce_timings(size_t my_count, double my_elapsed, size_t my_instances = 0) {
    return reduce_timings(upcxx::world(), my_count, my_elapsed, my_instances);
  }

  Timings barrier_timings(const upcxx::team &team, size_t my_instances = 0) const;
  Timings barrier_timings(size_t my_instances = 0) const { return barrier_timings(upcxx::world(), my_instances); }

  static Timings barrier_timings(const upcxx::team &team, size_t my_count, double my_elapsed, size_t my_instances = 0);
  static Timings barrier_timings(size_t my_count, double my_elapsed, size_t my_instances = 0) {
    return barrier_timings(upcxx::world(), my_elapsed, my_instances);
  }

  // member functionmust be called after start() and before stop()
  upcxx::future<MinSumMax<double>> reduce_start(const upcxx::team &team = upcxx::world()) { return reduce_timepoint(team, t); }
  static upcxx::future<MinSumMax<double>> reduce_timepoint(const upcxx::team &team = upcxx::world(), timepoint_t my_now = now());

  static timepoint_t now();

  static string now_str();

  static upcxx::future<MinSumMax<double>> reduce_now(const upcxx::team &team = upcxx::world()) {
    return reduce_timepoint(team, now());
  }

  // returns the indent nesting depending on how many nested BaseTimers are active
  static string get_indent(int indent = -1);
};

class StallTimer : public BaseTimer {
  // prints a Warning if called too many times or for too long. use in a while loop that could be indefinite
  double max_seconds;
  int64_t max_count;

 public:
  StallTimer(const string _name, double _max_seconds = 60.0, int64_t _max_count = -1);
  virtual ~StallTimer();
  void check();
};

class IntermittentTimer : public BaseTimer {
  // prints a summary on destruction
  double t_interval;
  string interval_label;
  void start_interval();
  void stop_interval();

 public:
  IntermittentTimer(const string &name, string interval_label = "");

  virtual ~IntermittentTimer();

  void start() {
    BaseTimer::start();
    start_interval();
  }

  void stop() {
    stop_interval();
    BaseTimer::stop();
  }

  inline double get_interval() const { return t_interval; }

  void clear();

  inline void inc_elapsed(double secs) { t_elapsed += secs; }

  void print_out(const upcxx::team &tm = upcxx::world());
};

class ProgressTimer : public BaseTimer {
 private:
  size_t calls;

 public:
  ProgressTimer(const string &name);

  virtual ~ProgressTimer();

  void clear();

  void progress(size_t run_every = 1);

  void discharge(size_t run_every = 1);

  void print_out(const upcxx::team &tm = upcxx::world());
};

class Timer : public BaseTimer {
  // times between construction and destruction
  // reduced load balance calcs on destruction
  const upcxx::team &tm;
  bool exited;
  bool logged;
  void init();

 public:
  Timer(const upcxx::team &tm, const string &name, bool exit_reduction = true);
  Timer(const string &name, bool exit_reduction = true);
  Timer(const Timer &copy) = delete;
  Timer(Timer &&move);
  Timer &operator=(const Timer &copy) = delete;
  Timer &operator=(Timer &&move);

  upcxx::future<> initiate_entrance_reduction();
  upcxx::future<> initiate_exit_reduction();
  virtual ~Timer();
};

class BarrierTimer : public BaseTimer {
  // barrier AND reduced load balance calcs on destruction
  const upcxx::team &_team;
  bool exit_barrier, exited;

 public:
  BarrierTimer(const upcxx::team &team, const string name, bool entrance_barrier = true, bool exit_barrier = true);
  BarrierTimer(const string name, bool entrance_barrier = true, bool exit_barrier = true);
  upcxx::future<> initate_exit_barrier();
  virtual ~BarrierTimer();

 protected:
  upcxx::future<> init(bool entrance_barrier);
};

class _AsyncTimer : public BaseTimer {
 public:
  _AsyncTimer(const upcxx::team &tm, const string &name);
  _AsyncTimer(const _AsyncTimer &copy) = delete;
  _AsyncTimer(_AsyncTimer &&move) = delete;
  void start();
  void stop();
  void report(const string label, MinSumMax<double> msm);
  upcxx::future<> initiate_construct_reduction();
  upcxx::future<> initiate_start_reduction();
  upcxx::future<> initiate_stop_reduction();
  const upcxx::team &tm;
  timepoint_t construct_t;
  timepoint_t start_t;
};

class AsyncTimer {
  // meant to be lambda captured and/or stopped within progress callbacks and returned as a future
  // times the delay between construction and start() and the time between start() and stop()
  // optionally, after stop(), can reduce the construction delay, start delay and start to stop duration across the team
  shared_ptr<_AsyncTimer> timer;

 public:
  AsyncTimer(const upcxx::team &tm, const string &name);
  AsyncTimer(const string &name);
  void start() const;
  void stop() const;
  double get_elapsed() const;
  // these methods may not be called within a progress() callback!
  upcxx::future<> initiate_construct_reduction();
  upcxx::future<> initiate_start_reduction();
  upcxx::future<> initiate_stop_reduction();
};

class ActiveCountTimer {
 protected:
  double total_elapsed;
  size_t total_count;
  size_t active_count;
  size_t max_active;
  string name;
  upcxx::future<> my_fut;

 public:
  ActiveCountTimer(const string _name = "");
  ~ActiveCountTimer();

  void clear();

  timepoint_t begin();

  void end(timepoint_t t);

  inline double get_total_elapsed() const { return total_elapsed; }
  inline size_t get_total_count() const { return total_count; }
  inline size_t get_active_count() const { return active_count; }
  inline size_t get_max_active_count() const { return max_active; }

  void print_barrier_timings(const upcxx::team &team, string label = "");
  void print_barrier_timings(string label = "") { print_barrier_timings(upcxx::world(), label); }

  void print_reduce_timings(const upcxx::team &team, string label = "");
  void print_reduce_timings(string label = "") { print_reduce_timings(upcxx::world(), label); }

  void print_timings(Timings &timings, string label = "");
};

template <typename Base>
class ActiveInstantiationTimerBase : public Base {
 protected:
  ActiveCountTimer &_act;
  timepoint_t _t;

 public:
  ActiveInstantiationTimerBase(ActiveCountTimer &act)
      : Base()
      , _act(act)
      , _t() {
    _t = act.begin();
  }

  ActiveInstantiationTimerBase(ActiveInstantiationTimerBase &&move)
      : Base(std::move(static_cast<Base &>(move)))
      , _act(move._act)
      , _t(move._t) {
    if (this != &move) move._t = {};  // delete timer for moved instance
  }

  virtual ~ActiveInstantiationTimerBase() {
    if (_t != timepoint_t()) _act.end(_t);
  }

  double get_elapsed_since_start() const {
    duration_seconds interval = BaseTimer::now() - this->_t;
    return interval.count();
  }

  // move but not copy
  ActiveInstantiationTimerBase(const ActiveInstantiationTimerBase &copy) = delete;
};

// to be used in inheritence to time all the instances of a class (like the duration of promises)
// to be used with an external ActiveContTimer
template <typename Base>
class ActiveInstantiationTimer : public ActiveInstantiationTimerBase<Base> {
 public:
  ActiveInstantiationTimer(ActiveCountTimer &act)
      : ActiveInstantiationTimerBase<Base>(act) {}
  ActiveInstantiationTimer(const ActiveInstantiationTimer &copy) = delete;
  ActiveInstantiationTimer(ActiveInstantiationTimer &&move)
      : ActiveInstantiationTimerBase<Base>(std::move(static_cast<ActiveInstantiationTimerBase<Base> &>(move))) {}
  virtual ~ActiveInstantiationTimer() {}

  void print_barrier_timings(string label = "") { this->_act.print_barrier_timings(label); }
  void print_barrier_timings(const upcxx::team &team, string label = "") { this->_act.print_barrier_timings(team, label); }
  void print_reduce_timings(string label = "") { this->_act.print_reduce_timings(label); }
  void print_reduce_timings(const upcxx::team &team, string label = "") { this->_act.print_reduce_timings(team, label); }
  void print_timings(Timings &timings, string label = "") { this->_act.print_timings(timings, label); }
  double get_total_elapsed() const { return this->_act.get_total_elapsed(); }
  size_t get_total_count() const { return this->_act.get_total_count(); }
  size_t get_active_count() const { return this->_act.get_active_count(); }
  size_t get_max_active_count() const { return this->_act.get_max_active_count(); }
  void clear() { return this->_act.clear(); }
};

// to be used in inheritence to time all the instances of a class (like the duration of promises)
// hold a static (by templated-class) specific ActiveCountTimer an external ActiveContTimer
// e.g. template <A,...> class my_timed_class : public my_class<A,...>, public InstantiationTimer<A,...> {};
// then when all instances have been destryed, call my_timed_class::print_barrier_timings();
template <typename Base, typename... DistinguishingArgs>
class InstantiationTimer : public ActiveInstantiationTimerBase<Base> {
 protected:
  static ActiveCountTimer &get_ACT() {
    static ActiveCountTimer _act = ActiveCountTimer();
    return _act;
  }

 public:
  InstantiationTimer()
      : ActiveInstantiationTimerBase<Base>(get_ACT()) {}
  // move but not copy this timer
  InstantiationTimer(const InstantiationTimer &copy) = delete;
  InstantiationTimer(InstantiationTimer &&move)
      : ActiveInstantiationTimerBase<Base>(std::move(static_cast<ActiveInstantiationTimerBase<Base> &>(move))) {}

  virtual ~InstantiationTimer() {}

  static void print_barrier_timings(string label = "") { get_ACT().print_barrier_timings(label); }
  static void print_barrier_timings(const upcxx::team &team, string label = "") { get_ACT().print_barrier_timings(team, label); }
  static void print_reduce_timings(string label) { get_ACT().print_reduce_timings(label); }
  static void print_reduce_timings(const upcxx::team &team, string label) { get_ACT().print_reduce_timings(team, label); }
  static void print_timings(Timings &timings, string label = "") { get_ACT().print_timings(timings, label); }
  static size_t get_total_count() { return get_ACT().get_total_count(); }
  static size_t get_active_count() { return get_ACT().get_active_count(); }
  static void clear() { get_ACT().clear(); }
};

//
// speed up compile with standard implementations of the Instantiation Timers
//

struct _upcxx_utils_dummy {};

typedef ActiveInstantiationTimer<_upcxx_utils_dummy> GenericInstantiationTimer;

typedef InstantiationTimer<_upcxx_utils_dummy> SingletonInstantiationTimer;

#ifndef _TIMERS_CPP

// use extern templates (implemented in timers.cpp) to speed up compile
extern template class ActiveInstantiationTimer<_upcxx_utils_dummy>;
extern template class InstantiationTimer<_upcxx_utils_dummy>;

#endif

};  // namespace upcxx_utils
