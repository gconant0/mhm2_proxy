file(REMOVE_RECURSE
  "libMHM2_VERSION_LIB.a"
  "libMHM2_VERSION_LIB.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/MHM2_VERSION_LIB.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
