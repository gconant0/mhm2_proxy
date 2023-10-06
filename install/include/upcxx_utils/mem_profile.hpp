#pragma once

#include <fstream>

#include "log.hpp"
#include "version.h"

#ifndef UPCXX_UTILS_NO_THREADS
#include <thread>
#endif

#include <upcxx/upcxx.hpp>

namespace upcxx_utils {

extern ofstream _memlog;
void open_memlog(string name);
void close_memlog();

double get_free_mem(void);
string get_self_stat(void);

#define _TRACK_MEMORY(free_mem, self_stat) " Free memory: ", ::upcxx_utils::get_size_str(free_mem), ", stat: ", self_stat

#define LOG_MEM_OS(os, label)                                             \
  ::upcxx_utils::logger(os, false, false, true, LOG_LINE_TS_LABEL, label, \
                        _TRACK_MEMORY(::upcxx_utils::get_free_mem(), ::upcxx_utils::get_self_stat()))

#define LOG_MEM(label)                                \
  do {                                                \
    if (::upcxx_utils::_memlog.is_open()) {           \
      LOG_MEM_OS(::upcxx_utils::_memlog, label);      \
      ::upcxx_utils::_memlog.flush();                 \
    } else if (::upcxx_utils::_logstream.is_open()) { \
      LOG_MEM_OS(::upcxx_utils::_logstream, label);   \
    }                                                 \
  } while (0)

#ifndef UPCXX_UTILS_NO_THREADS
class MemoryTrackerThread {
  std::thread *t = nullptr;
  double start_free_mem, min_free_mem;
  int ticks = 0, sample_ms = 500;
  bool fin = false;
  bool opened = false;
  std::string tracker_filename;

 public:
  MemoryTrackerThread(string _tracker_filename = "")
      : tracker_filename(_tracker_filename) {}
  void start();
  void stop();
};
#endif

};  // namespace upcxx_utils
