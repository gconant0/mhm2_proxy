# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/gavinconant/software/mhm2-v2.1.0

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/gavinconant/software/mhm2-v2.1.0/.build

# Include any dependencies generated for this target.
include upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/depend.make

# Include the progress variables for this target.
include upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/progress.make

# Include the compile flags for this target's objects.
include upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/flags.make

upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/reduce_prefix-extern-template-float-min_not_max-false.cpp.o: upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/flags.make
upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/reduce_prefix-extern-template-float-min_not_max-false.cpp.o: upcxx-utils/src/reduce_prefix-extern-template-float-min_not_max-false.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gavinconant/software/mhm2-v2.1.0/.build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/reduce_prefix-extern-template-float-min_not_max-false.cpp.o"
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/src && /usr/bin/mpicxx  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/reduce_prefix-extern-template-float-min_not_max-false.cpp.o -c /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/src/reduce_prefix-extern-template-float-min_not_max-false.cpp

upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/reduce_prefix-extern-template-float-min_not_max-false.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/reduce_prefix-extern-template-float-min_not_max-false.cpp.i"
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/src && /usr/bin/mpicxx $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/src/reduce_prefix-extern-template-float-min_not_max-false.cpp > CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/reduce_prefix-extern-template-float-min_not_max-false.cpp.i

upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/reduce_prefix-extern-template-float-min_not_max-false.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/reduce_prefix-extern-template-float-min_not_max-false.cpp.s"
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/src && /usr/bin/mpicxx $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/src/reduce_prefix-extern-template-float-min_not_max-false.cpp -o CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/reduce_prefix-extern-template-float-min_not_max-false.cpp.s

upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false: upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/reduce_prefix-extern-template-float-min_not_max-false.cpp.o
upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false: upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/build.make

.PHONY : upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false

# Rule to build all files generated by this target.
upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/build: upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false

.PHONY : upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/build

upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/clean:
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/src && $(CMAKE_COMMAND) -P CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/cmake_clean.cmake
.PHONY : upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/clean

upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/depend:
	cd /home/gavinconant/software/mhm2-v2.1.0/.build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gavinconant/software/mhm2-v2.1.0 /home/gavinconant/software/mhm2-v2.1.0/upcxx-utils/src /home/gavinconant/software/mhm2-v2.1.0/.build /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/src /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : upcxx-utils/src/CMakeFiles/upcxx_utils_reduce_prefix-extern-template-float-min_not_max-false.dir/depend
