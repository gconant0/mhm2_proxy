#include "upcxx_utils/reduce_prefix.hpp"

namespace upcxx_utils {

MACRO_REDUCE_PREFIX(int, upcxx::detail::op_wrap<upcxx::detail::opfn_add COMMA true>,  template);

};
