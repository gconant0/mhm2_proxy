file(REMOVE_RECURSE
  "libCGRAPH_LIBRARY.a"
  "libCGRAPH_LIBRARY.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/CGRAPH_LIBRARY.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
