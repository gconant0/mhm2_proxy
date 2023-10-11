#
# This file is autogenerated by GetGitVersion.cmake
#
file(READ /home/gavinconant/software/mhm2-v2.1.0/.build/makeVersionFile/MHM2_VERSION     MHM2_VERSION_RAW LIMIT 1024)
string(STRIP "${MHM2_VERSION_RAW}" MHM2_VERSION_AND_BRANCH)
separate_arguments(MHM2_VERSION_AND_BRANCH)
list(GET MHM2_VERSION_AND_BRANCH 0 MHM2_VERSION)
list(LENGTH MHM2_VERSION_AND_BRANCH MHM2_VERSION_AND_BRANCH_LEN)
if (MHM2_VERSION_AND_BRANCH_LEN GREATER 1)
  list(GET MHM2_VERSION_AND_BRANCH 1 MHM2_BRANCH)
else()
  set(MHM2_BRANCH)
endif()

set(MHM2_BUILD_DATE "20230111_131515")

set(MHM2_VERSION_STRING  "${MHM2_VERSION}")
message(STATUS "Building MHM2 version ${MHM2_VERSION_STRING} on branch ${MHM2_BRANCH}")
configure_file(/home/gavinconant/software/mhm2-v2.1.0/.build/makeVersionFile/version.cpp.in /home/gavinconant/software/mhm2-v2.1.0/.build/makeVersionFile/___MHM2_AUTOGEN_version.c)
