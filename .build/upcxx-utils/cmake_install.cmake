# Install script for directory: /home/gavinconant/software/mhm2-v2.1.0/upcxx-utils

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/gavinconant/software/mhm2-v2.1.0/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/UPCXX_UTILS.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/UPCXX_UTILS.cmake"
         "/home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/CMakeFiles/Export/cmake/UPCXX_UTILS.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/UPCXX_UTILS-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/cmake/UPCXX_UTILS.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake" TYPE FILE FILES "/home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/CMakeFiles/Export/cmake/UPCXX_UTILS.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake" TYPE FILE FILES "/home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/CMakeFiles/Export/cmake/UPCXX_UTILS-release.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake" TYPE FILE FILES "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/cmake/UPCXX_UTILSConfig.cmake")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake" TYPE FILE FILES
    "/home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/makeVersionFile/UPCXX_UTILSConfigVersion.cmake"
    "/home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/makeVersionFile/UPCXX_UTILS_VERSION"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/upcxx_utils" TYPE FILE FILES
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/Allocators.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/bin_hash.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/binary_search.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/colors.h"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/fixed_size_cache.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/flat_aggr_store.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/gasnetvars.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/gather.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/heavy_hitter_streaming_store.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/limit_outstanding.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/log.h"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/log.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/mem_profile.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/ofstream.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/progress_bar.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/promise_collectives.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/reduce_prefix.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/shared_array.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/shared_global_ptr.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/split_rank.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/thread_pool.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/three_tier_aggr_store.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/timers.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/two_tier_aggr_store.hpp"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/version.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/upcxx_utils/memory-allocators" TYPE FILE FILES
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/memory-allocators/Allocator.h"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/memory-allocators/PoolAllocator.h"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/memory-allocators/StackLinkedList.h"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/memory-allocators/StackLinkedListImpl.h"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/memory-allocators/UPCXXAllocator.h"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/memory-allocators/Utils.h"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/include/upcxx_utils/memory-allocators/upcxx_defs.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/." TYPE FILE FILES
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/LICENSE.txt"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/LEGAL.txt"
    "/home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/README.md"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/src/cmake_install.cmake")

endif()

