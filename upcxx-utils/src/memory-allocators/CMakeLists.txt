message(STATUS "Building the Memory Allocators")

add_library(UPCXX_UTILS_MEMORY_ALLOCATORS OBJECT Allocator.cpp PoolAllocator.cpp UPCXXAllocator.cpp Utils.cpp)
if (${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.13 AND DEFINED UPCXX_LIBRARIES)
    cmake_policy(SET CMP0079 NEW)
    target_link_libraries(UPCXX_UTILS_MEMORY_ALLOCATORS PUBLIC ${UPCXX_LIBRARIES} Threads::Threads)
endif()

