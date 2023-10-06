#include <upcxx/upcxx.hpp>

#include "upcxx_utils/log.hpp"

int test_log(int argc, char **argv) {
  upcxx_utils::open_dbg("test_log");

  OUT("Success: ", upcxx::rank_me(), " of ", upcxx::rank_n(), "\n");
  upcxx::barrier();

  SOUT("Done\n");

  upcxx_utils::close_dbg();

  return 0;
}
