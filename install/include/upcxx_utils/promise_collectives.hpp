#pragma once
#include <vector>

#include "upcxx/upcxx.hpp"

using std::vector;

using upcxx::dist_object;
using upcxx::intrank_t;

namespace upcxx_utils {

int roundup_log2(uint64_t n);

class PromiseBarrier {
  // a two step barrier with delayed execution
  //
  // construct in the master thread (with strict ordering of collectives on team)
  // fulfill() and get_future() methods are safe to call within the restricted context
  // of a progress callback
  //
  // implements the Dissemination Algorithm
  //   detailed in "Two algorithms for Barrier Synchronization"
  //   Manber et al, 1998
  // O(logN) latency
  // O(NlogN) total small messages
  //
  // Usage:
  // PromiseBarrier prom_barrier(team);
  // ...
  // prom_barrier.fulfill();  // may call within progess() callback
  // ...
  // prom_barrier.get_future().then(...); // may call within progress() callback
  //
  struct DisseminationWorkflow;
  using DistDisseminationWorkflow = dist_object<DisseminationWorkflow>;
  struct DisseminationWorkflow {
    static void init_workflow(DistDisseminationWorkflow &dist_dissem);

    DisseminationWorkflow();
    upcxx::future<> get_future() const;

    vector<upcxx::promise<> > level_proms;  // one for each level instance
    upcxx::promise<> initiated_prom;        // to signal this rank start
    upcxx::future<> done_future;
  };

  const upcxx::team &tm;
  DistDisseminationWorkflow dist_workflow;
  bool moved = false;

 public:
  PromiseBarrier(const upcxx::team &tm = upcxx::world());
  PromiseBarrier(PromiseBarrier &&move);
  PromiseBarrier(const PromiseBarrier &copy) = delete;
  PromiseBarrier &operator=(PromiseBarrier &&move);
  PromiseBarrier &operator=(const PromiseBarrier &copy) = delete;
  ~PromiseBarrier();
  void fulfill() const;
  upcxx::future<> get_future() const;
};
};  // namespace upcxx_utils
