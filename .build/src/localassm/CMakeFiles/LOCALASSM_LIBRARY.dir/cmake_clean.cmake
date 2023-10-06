file(REMOVE_RECURSE
  "libLOCALASSM_LIBRARY.a"
  "libLOCALASSM_LIBRARY.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/LOCALASSM_LIBRARY.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
