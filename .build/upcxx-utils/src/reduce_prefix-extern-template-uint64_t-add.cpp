#include "upcxx_utils/reduce_prefix.hpp"

namespace upcxx_utils {

MACRO_REDUCE_PREFIX(uint64_t, upcxx::detail::op_wrap<upcxx::detail::opfn_add COMMA true>,  template);

};
