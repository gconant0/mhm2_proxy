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
include upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/depend.make

# Include the progress variables for this target.
include upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/progress.make

# Include the compile flags for this target's objects.
include upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/flags.make

upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/makeVersionFile/version.cpp.o: upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/flags.make
upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/makeVersionFile/version.cpp.o: upcxx-utils/makeVersionFile/version.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gavinconant/software/mhm2-v2.1.0/.build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/makeVersionFile/version.cpp.o"
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils && /usr/bin/mpicxx  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/UPCXX_UTILS_VERSION.dir/makeVersionFile/version.cpp.o -c /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/makeVersionFile/version.cpp

upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/makeVersionFile/version.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/UPCXX_UTILS_VERSION.dir/makeVersionFile/version.cpp.i"
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils && /usr/bin/mpicxx $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/makeVersionFile/version.cpp > CMakeFiles/UPCXX_UTILS_VERSION.dir/makeVersionFile/version.cpp.i

upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/makeVersionFile/version.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/UPCXX_UTILS_VERSION.dir/makeVersionFile/version.cpp.s"
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils && /usr/bin/mpicxx $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/makeVersionFile/version.cpp -o CMakeFiles/UPCXX_UTILS_VERSION.dir/makeVersionFile/version.cpp.s

UPCXX_UTILS_VERSION: upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/makeVersionFile/version.cpp.o
UPCXX_UTILS_VERSION: upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/build.make

.PHONY : UPCXX_UTILS_VERSION

# Rule to build all files generated by this target.
upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/build: UPCXX_UTILS_VERSION

.PHONY : upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/build

upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/clean:
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils && $(CMAKE_COMMAND) -P CMakeFiles/UPCXX_UTILS_VERSION.dir/cmake_clean.cmake
.PHONY : upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/clean

upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/depend:
	cd /home/gavinconant/software/mhm2-v2.1.0/.build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gavinconant/software/mhm2-v2.1.0 /home/gavinconant/software/mhm2-v2.1.0/upcxx-utils /home/gavinconant/software/mhm2-v2.1.0/.build /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils /home/gavinconant/software/mhm2-v2.1.0/.build/upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : upcxx-utils/CMakeFiles/UPCXX_UTILS_VERSION.dir/depend
