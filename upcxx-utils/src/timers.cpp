#include <chrono>
#include <ctime>
#include <iomanip>
#include <upcxx/upcxx.hpp>

#define _TIMERS_CPP
#include "upcxx_utils/timers.hpp"

using upcxx::future;

namespace upcxx_utils {

// Reduce compile time by making templates instantiations of common types
// these are each constructed in CMakeLists.txt and timers-extern-template.in.cpp
// extern templates declarations all happen in timers.hpp

/*
 * This is now handled by CMakeLists.txt
 *
   MACRO_MIN_SUM_MAX(float,    template);
   MACRO_MIN_SUM_MAX(double,   template);
   MACRO_MIN_SUM_MAX(int64_t,  template);
   MACRO_MIN_SUM_MAX(uint64_t, template);
   MACRO_MIN_SUM_MAX(int,      template);

 */

//
// Timings
//

future<> &Timings::get_last_pending() {
  static future<> _ = make_future();
  return _;
}

Timings::Timings()
    : t()
    , before_elapsed(0.0)
    , after_elapsed(0.0)
    , reduction_elapsed(0.0)
    , my_count(0)
    , my_instance(0) {}

future<> Timings::get_pending() { return get_last_pending(); }

void Timings::set_pending(future<> fut) { get_last_pending() = when_all(get_last_pending(), fut); }

void Timings::wait_pending() {
  DBG_VERBOSE(__func__, "\n");
  if (upcxx::initialized()) {
    get_last_pending().wait();
    get_last_pending() = make_future();
  }
}

string Timings::to_string(bool print_count, bool print_label) const {
  ostringstream os;
  if (print_label) os << "(min/my/avg/max, bal) ";
  os << std::setprecision(2) << std::fixed;
  // print the timing metrics
  auto &before_max = before_msm.max;
  auto &before_min = before_msm.min;
  auto &before_sum = before_msm.sum;
  if (before_max > 0.0) {
    double bal = (before_max > 0.0 ? before_sum / rank_n() / before_max : 1.0);
    if (before_max > 10.0 && bal < .9) os << KLRED;  // highlight large imbalances
    os << before_min << "/" << before_elapsed << "/" << before_sum / rank_n() << "/" << before_max << " s, " << bal;
    if (before_max > 1.0 && bal < .9) os << KLCYAN;
  } else {
    os << "0/0/0/0 s, 1.00";
  }

  os << std::setprecision(1) << std::fixed;

  auto &after_max = after_msm.max;
  auto &after_min = after_msm.min;
  auto &after_sum = after_msm.sum;
  // print the timings around a barrier if they are significant
  if (after_max >= 0.1) {
    os << (after_max > 1.0 ? KLRED : "") << " barrier " << after_min << "/" << after_elapsed << "/" << after_sum / rank_n() << "/"
       << after_max << " s, " << (after_max > 0.0 ? after_sum / rank_n() / after_max : 0.0) << (after_max > 1.0 ? KLCYAN : "");
  } else if (after_max > 0.0) {
    os << std::setprecision(2) << std::fixed;
    os << " barrier " << after_max << " s";
    os << std::setprecision(1) << std::fixed;
  }

  auto &count_max = count_msm.max;
  auto &count_min = count_msm.min;
  auto &count_sum = count_msm.sum;
  // print the max_count if it is more than 1 or more than 0 if asked to print the count
  if (count_max > (print_count ? 0.0 : 1.00001))
    os << " count " << count_min << "/" << my_count << "/" << count_sum / rank_n() << "/" << count_max << ", "
       << (count_max > 0.0 ? count_sum / rank_n() / count_max : 0.0);

  auto &instance_max = instance_msm.max;
  auto &instance_min = instance_msm.min;
  auto &instance_sum = instance_msm.sum;
  // print the instances if it is both non-zero and not 1 per rank
  if (instance_sum > 0 && ((int)(instance_sum + 0.01)) != rank_n() && ((int)(instance_sum + 0.99)) != rank_n())
    os << " inst " << instance_min << "/" << my_instance << "/" << instance_sum / rank_n() << "/" << instance_max << ", "
       << (instance_max > 0.0 ? instance_sum / rank_n() / instance_max : 0.0);
  // print the reduction timings if they are significant
  if (reduction_elapsed > 0.05)
    os << (reduction_elapsed > .5 ? KLRED : "") << " reduct " << reduction_elapsed << (reduction_elapsed > .5 ? KLCYAN : "");
  return os.str();
}

void Timings::set_before(Timings &timings, size_t count, double elapsed, size_t instances) {
  DBG_VERBOSE("set_before: my_count=", count, " my_elapsed=", elapsed, " instances=", instances, "\n");
  timings.before = std::chrono::high_resolution_clock::now();

  timings.my_count = count;
  timings.count_msm.reset(timings.my_count);

  timings.before_elapsed = elapsed;
  timings.before_msm.reset(elapsed);

  timings.my_instance = instances;
  timings.instance_msm.reset(instances);
}

// timings must remain in scope until the returened future is ready()
future<> Timings::set_after(const upcxx::team &team, Timings &timings,
                            std::chrono::time_point<std::chrono::high_resolution_clock> t_after) {
  timings.after = t_after;
  duration_seconds interval = timings.after - timings.before;
  timings.after_elapsed = interval.count();
  timings.after_msm.reset(timings.after_elapsed);
  DBG_VERBOSE("set_after: ", interval.count(), "\n");

  // time the reductions
  timings.t = t_after;

  assert(&timings.instance_msm == &timings.before_msm + 3);  // memory is in order
  auto fut_msms = min_sum_max_reduce_all(&timings.before_msm, &timings.before_msm, 4, team);
  auto ret = fut_msms.then([&timings]() {
    duration_seconds interval = std::chrono::high_resolution_clock::now() - timings.t;
    timings.reduction_elapsed = interval.count();
    DBG_VERBOSE("Finished reductions:, ", interval.count(), "\n");
  });

  set_pending(when_all(ret, get_pending()));
  return ret;
}

// barrier and reduction
Timings Timings::barrier(const upcxx::team &team, size_t count, double elapsed, size_t instances) {
  DBG("Timings::barrier(", count, ", ", elapsed, ", ", instances, ")\n");
  Timings timings;
  set_before(timings, count, elapsed, instances);
  upcxx::barrier(team);
  progress();  // explicitly make progress after the barrier if the barrier itself was already ready()
  auto fut = set_after(team, timings);
  wait_pending();
  assert(fut.ready());
  return timings;
}

void Timings::print_barrier_timings(const upcxx::team &team, string label) {
  Timings timings = barrier(team, 0, 0, 0);
  wait_pending();
  SLOG_VERBOSE(KLCYAN, "Timing ", label, ":", timings.to_string(), KNORM, "\n");
}

// no barrier but a future reduction is started
future<ShTimings> Timings::reduce(const upcxx::team &team, size_t count, double elapsed, size_t instances) {
  DBG("Timings::reduce(", count, ", ", elapsed, ", ", instances, ")\n");
  auto timings = make_shared<Timings>();
  set_before(*timings, count, elapsed, instances);
  auto future_reduction = set_after(team, *timings, timings->before);  // after == before, so no barrier info will be output
  return when_all(make_future(timings), future_reduction, get_pending());
}

void Timings::print_reduce_timings(const upcxx::team &team, string label) {
  future<ShTimings> fut_timings = reduce(team, 0, 0, 0);
  auto fut = when_all(fut_timings, get_pending()).then([label = std::move(label)](ShTimings shptr_timings) {
    SLOG_VERBOSE(KLCYAN, "Timing ", label, ": ", shptr_timings->to_string(), "\n", KNORM);
  });
  set_pending(fut);
}

//
// BaseTimer
//

size_t &BaseTimer::instance_count() {
  static size_t _ = 0;
  return _;
}

void BaseTimer::increment_instance() { ++instance_count(); }
void BaseTimer::decrement_instance() { instance_count()--; }
size_t BaseTimer::get_instance_count() { return instance_count(); }

BaseTimer::BaseTimer()
    : t()
    , name()
    , t_elapsed(0.0)
    , count(0) {}

BaseTimer::BaseTimer(const string &_name)
    : t()
    , name(_name)
    , t_elapsed(0.0)
    , count(0) {}

BaseTimer::~BaseTimer() {}

void BaseTimer::clear() {
  t = timepoint_t();
  t_elapsed = 0.0;
  count = 0;
}

void BaseTimer::start() {
  assert(t == timepoint_t());
  t = now();
}

void BaseTimer::stop() {
  double elapsed = get_elapsed_since_start();
  t = timepoint_t();  // reset to 0
  // DBG("stop(", name, ", inst=", get_instance_count(), "): ", elapsed, " s, ", now_str(), "\n");
  t_elapsed += elapsed;
  count++;
}

double BaseTimer::get_elapsed() const { return t_elapsed; }

double BaseTimer::get_elapsed_since_start() const {
  assert(t != timepoint_t());
  duration_seconds interval = now() - t;
  return interval.count();
}

size_t BaseTimer::get_count() const { return count; }

const string &BaseTimer::get_name() const { return name; }

void BaseTimer::done() const {
  assert(t == timepoint_t());
  SLOG_VERBOSE(KLCYAN, "Timing ", name, ": ", std::setprecision(2), std::fixed, t_elapsed, " s ", KNORM, "\n");
  DBG(name, " took ", std::setprecision(2), std::fixed, t_elapsed, " s ", "\n");
}

future<MinSumMax<double>> BaseTimer::done_all_async(const upcxx::team &tm) const {
  assert(t == timepoint_t());
  auto msm_fut = upcxx_utils::min_sum_max_reduce_one(t_elapsed, 0, tm);
  DBG(name, " took ", t_elapsed, " \n");
  auto name_copy = name;
  msm_fut = msm_fut.then([name_copy](MinSumMax<double> msm) {
    SLOG_VERBOSE(KLCYAN, "Timing ", name_copy, ": ", msm, KNORM, "\n");
    return msm;
  });
  Timings::set_pending(msm_fut.then([](MinSumMax<double>) {}));
  return msm_fut;
}
void BaseTimer::done_all(const upcxx::team &tm) const { done_all_async(tm).wait(); }

string BaseTimer::get_final() const {
  ostringstream os;
  os << name << ": " << std::setprecision(2) << std::fixed << t_elapsed << " s";
  if (count > 1) os << " " << count << " count";
  return os.str();
}

future<MinSumMax<double>> BaseTimer::reduce_timepoint(const upcxx::team &team, timepoint_t timepoint) {
  duration_seconds secs = timepoint.time_since_epoch();
  DBG_VERBOSE("reduce_timepoint ", secs.count(), " since epoch\n");
  future<MinSumMax<double>> fut_msm = min_sum_max_reduce_one<double>(secs.count(), 0, team);
  return fut_msm.then([&team](MinSumMax<double> msm) {
    duration_seconds interval;
    if (team.rank_me()) return msm;
    // translate to seconds since the first rank entered
    msm.my = msm.my - msm.min;
    msm.max = msm.max - msm.min;
    msm.sum = msm.sum - msm.min * team.rank_n();
    msm.min = 0.0;
    msm.apply_avg(team);
    return msm;
  });
}

future<ShTimings> BaseTimer::reduce_timings(const upcxx::team &team, size_t my_instances) const {
  return reduce_timings(team, count, t_elapsed, my_instances);
}

future<ShTimings> BaseTimer::reduce_timings(const upcxx::team &team, size_t my_count, double my_elapsed, size_t my_instances) {
  return Timings::reduce(team, my_count, my_elapsed, my_instances);
}

Timings BaseTimer::barrier_timings(const upcxx::team &team, size_t my_instances) const {
  return barrier_timings(team, count, t_elapsed, my_instances);
}

Timings BaseTimer::barrier_timings(const upcxx::team &team, size_t my_count, double my_elapsed, size_t my_instances) {
  return Timings::barrier(team, my_count, my_elapsed, my_instances);
}

timepoint_t BaseTimer::now() { return std::chrono::high_resolution_clock::now(); }

string BaseTimer::now_str() {
  std::time_t result = std::time(nullptr);
  char buffer[100];
  size_t sz = strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&result));
  return string(sz > 0 ? buffer : "BAD TIME");
}

//
// StallTimer
//

StallTimer::StallTimer(const string _name, double _max_seconds, int64_t _max_count)
    : BaseTimer(_name)
    , max_seconds(_max_seconds)
    , max_count(_max_count) {
  start();
}

StallTimer::~StallTimer() { stop(); }

void StallTimer::check() {
  stop();
  bool print = false;
  if (max_seconds > 0.0 && t_elapsed > max_seconds) {
    print = true;
  } else if (max_count > 0 && count > max_count) {
    print = true;
  }
  if (print) {
    WARN("StallTimer - ", name, " on ", rank_me(), " stalled for ", t_elapsed, " s and ", count, " iterations\n");
    max_seconds *= 2.0;
    max_count *= 2;
  }
  start();
}

//
// IntermittentTimer
//

IntermittentTimer::IntermittentTimer(const string &_name, string _interval_label)
    : BaseTimer(_name)
    , t_interval(0.0)
    , interval_label(_interval_label) {}

IntermittentTimer::~IntermittentTimer() {}

void IntermittentTimer::clear() {
  ((BaseTimer *)this)->clear();
  t_interval = 0.0;
  interval_label = "";
}

void IntermittentTimer::start_interval() { t_interval = get_elapsed_since_start(); }

void IntermittentTimer::stop_interval() {
  t_interval = get_elapsed_since_start() - t_interval;
  if (!interval_label.empty()) {
    ostringstream oss;
    oss << KBLUE << std::left << std::setw(40) << interval_label << std::setprecision(2) << std::fixed << t_interval << " s"
        << KNORM << "\n";
    SLOG(oss.str());
  }
}

void IntermittentTimer::print_out(const upcxx::team &tm) {
  future<ShTimings> fut_shptr_timings = reduce_timings(tm);
  auto fut =
      when_all(Timings::get_pending(), fut_shptr_timings).then([&name = this->name, &count = this->count](ShTimings shptr_timings) {
        if (shptr_timings->count_msm.max > 0.0)
          SLOG_VERBOSE(KLCYAN, "Timing ", name, ": ", count, " intervals, ", shptr_timings->to_string(true), "\n", KNORM);
      });
  Timings::set_pending(fut);
  count = 0;
  t_elapsed = 0.0;
}

//
// ProgressTimer
//

ProgressTimer::ProgressTimer(const string &_name)
    : BaseTimer(_name)
    , calls(0) {}

ProgressTimer::~ProgressTimer() {}

void ProgressTimer::clear() {
  ((BaseTimer *)this)->clear();
  calls = 0;
}

void ProgressTimer::progress(size_t run_every) {
  if (run_every > 1 && ++calls % run_every != 0) return;
  start();
  upcxx::progress();
  stop();
  // DBG("ProgressTimer(", name, ") - ", t_elapsed, "\n");
}

void ProgressTimer::discharge(size_t run_every) {
  if (run_every != 1 && ++calls % run_every != 0) return;
  start();
  upcxx::discharge();
  upcxx::progress();
  stop();
  // DBG("ProgressTimer(", name, ").discharge() - ", t_elapsed, "\n");
}

void ProgressTimer::print_out(const upcxx::team &tm) {
  future<ShTimings> fut_shptr_timings = reduce_timings(tm);
  auto fut = when_all(Timings::get_pending(), fut_shptr_timings).then([&name = this->name](ShTimings shptr_timings) {
    if (shptr_timings->count_msm.max > 0.0)
      SLOG_VERBOSE(KLCYAN, "Timing ", name, ": ", shptr_timings->to_string(true), KNORM, "\n");
  });
  Timings::set_pending(fut);
  count = 0;
  t_elapsed = 0.0;
}

//
// Timer
//
Timer::Timer(const upcxx::team &tm, const string &_name, bool exit_reduction)
    : tm(tm)
    , exited(exit_reduction)
    , logged(false)
    , BaseTimer(_name) {
  init();
}
Timer::Timer(const string &_name, bool exit_reduction)
    : tm(upcxx::world())
    , exited(exit_reduction)
    , logged(false)
    , BaseTimer(_name) {
  init();
}
void Timer::init() {
  increment_instance();
  auto fut = when_all(Timings::get_pending(), make_future(now_str())).then([name = this->name](string now) {});
  Timings::set_pending(fut);
  start();
}
Timer::Timer(Timer &&move)
    : tm(move.tm)
    , exited(move.exited)
    , BaseTimer((BaseTimer &)move) {
  move.exited = true;
  move.logged = true;
}
Timer &Timer::operator=(Timer &&move) {
  Timer mv(std::move(move));
  std::swap(*this, mv);
  return *this;
}

Timer::~Timer() {
  if (!exited)
    initiate_exit_reduction();
  else if (!logged) {
    stop();
    LOG(KLCYAN, "Timing ", name, ":", get_elapsed(), KNORM, "\n");
  }
}

future<> Timer::initiate_entrance_reduction() {
  DBG_VERBOSE("Tracking entrance of ", name, "\n");
  auto fut_msm = reduce_timepoint(tm, now());

  auto fut = when_all(Timings::get_pending(), fut_msm).then([name = this->name](MinSumMax<double> msm) {
    DBG_VERBOSE("got reduction: ", msm.to_string(), "\n");
    SLOG_VERBOSE(KLCYAN, "Timing (entrance) ", name, ":", msm.to_string(), KNORM, "\n");
  });
  Timings::set_pending(fut);
  return fut;
}

future<> Timer::initiate_exit_reduction() {
  stop();
  future<ShTimings> fut_shptr_timings = reduce_timings(tm);
  auto fut = when_all(Timings::get_pending(), fut_shptr_timings).then([name = this->name](ShTimings shptr_timings) {
    SLOG_VERBOSE(KLCYAN, "Timing ", name, " exit: ", shptr_timings->to_string(), KNORM, "\n");
  });
  Timings::set_pending(fut);
  decrement_instance();
  exited = true;
  logged = true;
  return fut;
}

//
// BarrierTimer
//

BarrierTimer::BarrierTimer(const upcxx::team &team, const string _name, bool _entrance_barrier, bool _exit_barrier)
    : _team(team)
    , exit_barrier(_exit_barrier)
    , exited(false)
    , BaseTimer(_name) {
  init(_entrance_barrier);
}
BarrierTimer::BarrierTimer(const string _name, bool _entrance_barrier, bool _exit_barrier)
    : _team(upcxx::world())
    , exit_barrier(_exit_barrier)
    , exited(false)
    , BaseTimer(_name) {
  init(_entrance_barrier);
}

future<> BarrierTimer::init(bool _entrance_barrier) {
  increment_instance();
  if (!_entrance_barrier && !exit_barrier) SLOG_VERBOSE("Why are we using a BarrierTimer without any barriers???\n");
  future<> fut;
  DBG("Entering BarrierTimer ", name, "\n");
  if (_entrance_barrier) {
    fut = when_all(Timings::get_pending(), make_future(now_str())).then([&name = this->name](string now) {
      // SLOG_VERBOSE(KLCYAN, "Timing ", name, ":  (entering barrier) ", KNORM);
    });
    Timings::set_pending(fut);
    auto timings = barrier_timings(_team);
    Timings::wait_pending();  // should be noop
    SLOG_VERBOSE(KLCYAN, "Timing (entrance barrier) ", name, ": ", timings.to_string(), KNORM, "\n");
  } else {
    fut = when_all(Timings::get_pending(), make_future(now_str())).then([&name = this->name](string now) {});
    Timings::set_pending(fut);
  }
  start();
  return fut;
}

BarrierTimer::~BarrierTimer() {
  if (!exited) initate_exit_barrier().wait();
}
future<> BarrierTimer::initate_exit_barrier() {
  stop();
  future<> fut;
  DBG("Exiting BarrierTimer ", name, "\n");
  if (exit_barrier) {
    fut = when_all(Timings::get_pending(), make_future(now_str())).then([name = this->name](string now) {});
    Timings::set_pending(fut);
    auto timings = barrier_timings(_team);
    Timings::wait_pending();
    SLOG_VERBOSE(KLCYAN, "Timing ", name, ": ", timings.to_string(), KNORM, "\n");
  } else {
    future<ShTimings> fut_shptr_timings = reduce_timings(_team);
    fut = when_all(Timings::get_pending(), fut_shptr_timings).then([name = this->name](ShTimings shptr_timings) {
      SLOG_VERBOSE(KLCYAN, "Timing ", name, ": ", shptr_timings->to_string(), KNORM, "\n");
    });
    Timings::set_pending(fut);
  }
  decrement_instance();
  exited = true;
  return fut;
}

//
// AsyncTimer
//

_AsyncTimer::_AsyncTimer(const upcxx::team &tm, const string &name)
    : BaseTimer(name)
    , tm(tm)
    , construct_t(BaseTimer::now())
    , start_t{} {}
void _AsyncTimer::start() {
  start_t = now();
  ((BaseTimer *)this)->start();
}
void _AsyncTimer::stop() { ((BaseTimer *)this)->stop(); }
void _AsyncTimer::report(const string label, MinSumMax<double> msm) {
  SLOG_VERBOSE(KLCYAN, "Timing ", name, " ", label, ":", msm.to_string(), KNORM, "\n");
}

future<> _AsyncTimer::initiate_construct_reduction() {
  auto fut_msm = BaseTimer::reduce_timepoint(tm, construct_t);
  auto fut = when_all(Timings::get_pending(), fut_msm).then([this](MinSumMax<double> msm) { this->report("construct", msm); });
  Timings::set_pending(fut);
  return fut;
}
future<> _AsyncTimer::initiate_start_reduction() {
  auto fut_msm = BaseTimer::reduce_timepoint(tm, start_t);
  auto fut = when_all(Timings::get_pending(), fut_msm).then([this](MinSumMax<double> msm) { this->report("start", msm); });
  Timings::set_pending(fut);
  return fut;
}
future<> _AsyncTimer::initiate_stop_reduction() {
  auto fut_msm = Timings::reduce(tm, 1, get_elapsed(), 1);
  auto fut = when_all(Timings::get_pending(), fut_msm).then([this](ShTimings sh_timings) {
    this->report("stop", sh_timings->before_elapsed);
  });
  Timings::set_pending(fut);
  return fut;
}

AsyncTimer::AsyncTimer(const upcxx::team &tm, const string &name)
    : timer(make_shared<_AsyncTimer>(tm, name)) {}
AsyncTimer::AsyncTimer(const string &name)
    : timer(make_shared<_AsyncTimer>(upcxx::world(), name)) {}
void AsyncTimer::start() const { timer->start(); }
void AsyncTimer::stop() const {
  timer->stop();
  LOG(timer->get_name(), " completed in ", timer->get_elapsed(), " s\n");
}
double AsyncTimer::get_elapsed() const { return timer->get_elapsed(); }
future<> AsyncTimer::initiate_construct_reduction() {
  return timer->initiate_construct_reduction().then([timer = this->timer]() {
    // keep timer alive
  });
}
future<> AsyncTimer::initiate_start_reduction() {
  return timer->initiate_start_reduction().then([timer = this->timer]() {
    // keep timer alive
  });
}
future<> AsyncTimer::initiate_stop_reduction() {
  return timer->initiate_stop_reduction().then([timer = this->timer]() {
    // keep timer alive
  });
}

//
// ActiveCountTimer
//

ActiveCountTimer::ActiveCountTimer(const string _name)
    : total_elapsed(0.0)
    , total_count(0)
    , active_count(0)
    , max_active(0)
    , name(_name)
    , my_fut(make_future()) {}

ActiveCountTimer::~ActiveCountTimer() {
  if (upcxx::initialized()) my_fut.wait();  // keep alive until all futures have finished
}

void ActiveCountTimer::clear() {
  total_elapsed = 0.0;
  total_count = 0;
  active_count = 0;
  max_active = 0;
}

timepoint_t ActiveCountTimer::begin() {
  active_count++;
  if (max_active < active_count) max_active = active_count;
  return BaseTimer::now();
}

void ActiveCountTimer::end(timepoint_t t) {
  duration_seconds interval = BaseTimer::now() - t;
  active_count--;
  total_count++;
  total_elapsed += interval.count();
}

void ActiveCountTimer::print_barrier_timings(const upcxx::team &team, string label) {
  Timings timings = BaseTimer::barrier_timings(team, total_count, total_elapsed, max_active);
  clear();
  Timings::wait_pending();
  print_timings(timings, label);
}

void ActiveCountTimer::print_reduce_timings(const upcxx::team &team, string label) {
  label = name + label;
  auto fut_timings = BaseTimer::reduce_timings(team, total_count, total_elapsed, max_active);
  auto _this = this;
  auto fut_clear = fut_timings.then([_this](ShTimings ignored) { _this->clear(); });
  auto fut = when_all(Timings::get_pending(), fut_timings, fut_clear).then([_this, label](ShTimings shptr_timings) {
    _this->print_timings(*shptr_timings, label);
  });
  my_fut = when_all(fut_clear, my_fut, fut);  // keep this in scope until clear has been called...
  Timings::set_pending(my_fut);
}

void ActiveCountTimer::print_timings(Timings &timings, string label) {
  label = name + label;
  DBG_VERBOSE(__func__, " label=", label, "\n");
  if (active_count > 0)
    SWARN("print_timings on ActiveCountTimer '", label, "' called while ", active_count, " (max ", max_active,
          ") are still active\n");
  if (timings.count_msm.max > 0.0) {
    SLOG_VERBOSE(KLCYAN, "Timing instances of ", label, ": ",
                 (timings.count_msm.max > 0.0 ? timings.to_string(true) : string("(none)")), KNORM, "\n");
  }
}

ActiveCountTimer _GenericActiveCountTimer("_upcxx_dummy");
GenericInstantiationTimer _GenericInstantiationTimer(_GenericActiveCountTimer);
template class ActiveInstantiationTimer<_upcxx_utils_dummy>;

SingletonInstantiationTimer _SingletonInstantiationTimer();
template class InstantiationTimer<_upcxx_utils_dummy>;

};  // namespace upcxx_utils
