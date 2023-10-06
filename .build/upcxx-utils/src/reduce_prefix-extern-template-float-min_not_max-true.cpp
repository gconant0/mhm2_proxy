#include "upcxx_utils/reduce_prefix.hpp"

namespace upcxx_utils {

MACRO_REDUCE_PREFIX(float, upcxx::detail::op_wrap<upcxx::detail::opfn_min_not_max<true> COMMA true>,  template);

};
