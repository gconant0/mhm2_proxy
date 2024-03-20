// two_step_barrier.cpp
#include "upcxx_utils/promise_collectives.hpp"

#include <cassert>

#include "upcxx_utils/log.hpp"

using upcxx::future;
using upcxx::make_future;
using upcxx::when_all;

int upcxx_utils::roundup_log2(uint64_t n) {
  //DBG("n=", n, "\n");
  if (n == 0) return -1;
#define S(k)                     \
  if (n >= (UINT64_C(1) << k)) { \
    i += k;                      \
    n >>= k;                     \
  }

  n--;
  //DBG(" n=", n, "\n");
  n |= n >> 1;
  //DBG(" n=", n, "\n");
  n |= n >> 2;
  //DBG(" n=", n, "\n");
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n |= n >> 32;
  n++;
  //DBG(" n=", n, "\n");

  int i = 0;
  S(32);
  S(16);
  S(8);
  S(4);
  S(2);
  S(1);

  //DBG(" n=", n, " i=", i, "\n");
  return i;
#undef S
}

upcxx_utils::PromiseBarrier::DisseminationWorkflow::DisseminationWorkflow()
    : level_proms()  // empty
    , initiated_prom()
    , done_future()  // never ready
{}

void upcxx_utils::PromiseBarrier::DisseminationWorkflow::init_workflow(DistDisseminationWorkflow &dist_dissem) {
  DBG_VERBOSE("\n");
  assert(upcxx::master_persona().active_with_caller());

  DisseminationWorkflow &dissem = *dist_dissem;
  assert(dissem.level_proms.size() == 0);

  const upcxx::team &tm = dist_dissem.team();
  intrank_t me = tm.rank_me(), n = tm.rank_n();
  int levels = roundup_log2(n);
  dissem.level_proms.resize(levels);

  intrank_t mask = 0x01;
  future<> fut_chain = dissem.initiated_prom.get_future();
  for (int level = 0; level < levels; level++) {
    intrank_t send_to = (me + mask) % n;

    fut_chain = fut_chain.then([&dist_dissem, &tm, send_to, level]() {
      rpc_ff(
          tm, send_to,
          [](DistDisseminationWorkflow &dist_dissem, int level) {
            auto &prom = dist_dissem->level_proms[level];
            prom.fulfill_anonymous(1);
          },
          dist_dissem, level);
    });

    auto &prom = dissem.level_proms[level];
    fut_chain = when_all(fut_chain, prom.get_future());
    mask <<= 1;
  }
  dissem.done_future = fut_chain.then([&dissem]() {
    vector<upcxx::promise<> > empty{};
    dissem.level_proms.swap(empty);  // cleanup memory
  });
}

upcxx::future<> upcxx_utils::PromiseBarrier::DisseminationWorkflow::get_future() const { return done_future; }

upcxx_utils::PromiseBarrier::PromiseBarrier(const upcxx::team &tm)
    : tm(tm)
    , dist_workflow(tm) {
  DBG_VERBOSE("tm.n=", tm.rank_n(), " this=", this, "\n");
  assert(upcxx::master_persona().active_with_caller());
  DisseminationWorkflow::init_workflow(dist_workflow);
}

upcxx_utils::PromiseBarrier::PromiseBarrier(PromiseBarrier &&mv)
    : tm(mv.tm)
    , dist_workflow(std::move(mv.dist_workflow)) {
  DBG_VERBOSE("moved mv=", &mv, " to this=", this, "\n");
  mv.moved = true;
}

upcxx_utils::PromiseBarrier &upcxx_utils::PromiseBarrier::operator=(PromiseBarrier &&mv) {
  PromiseBarrier newme(std::move(mv));
  std::swap(*this, newme);
  DBG_VERBOSE("Swapped newme=", &newme, " to this=", this, "\n");
  return *this;
}

upcxx_utils::PromiseBarrier::~PromiseBarrier() {
  DBG_VERBOSE("Destroy this=", this, " move=", moved, "\n");
  if (moved) return;  // invalidated
  assert(upcxx::master_persona().active_with_caller());
  assert(dist_workflow->initiated_prom.get_future().ready());
  get_future().wait();
}

void upcxx_utils::PromiseBarrier::fulfill() const {
  DBG_VERBOSE("fulfill this=", this, "\n");
  assert(upcxx::master_persona().active_with_caller());
  assert(!dist_workflow->initiated_prom.get_future().ready());
  dist_workflow->initiated_prom.fulfill_anonymous(1);
}

upcxx::future<> upcxx_utils::PromiseBarrier::get_future() const {
  assert(upcxx::master_persona().active_with_caller());
  return dist_workflow->get_future();
}
