file(REMOVE_RECURSE
  "libUPCXX_UTILS_LIBRARY.a"
  "libUPCXX_UTILS_LIBRARY.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/UPCXX_UTILS_LIBRARY.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
