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
include CMakeFiles/MHM2_VERSION.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/MHM2_VERSION.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/MHM2_VERSION.dir/flags.make

CMakeFiles/MHM2_VERSION.dir/makeVersionFile/version.cpp.o: CMakeFiles/MHM2_VERSION.dir/flags.make
CMakeFiles/MHM2_VERSION.dir/makeVersionFile/version.cpp.o: makeVersionFile/version.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gavinconant/software/mhm2-v2.1.0/.build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/MHM2_VERSION.dir/makeVersionFile/version.cpp.o"
	/usr/bin/mpicxx  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/MHM2_VERSION.dir/makeVersionFile/version.cpp.o -c /home/gavinconant/software/mhm2-v2.1.0/.build/makeVersionFile/version.cpp

CMakeFiles/MHM2_VERSION.dir/makeVersionFile/version.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/MHM2_VERSION.dir/makeVersionFile/version.cpp.i"
	/usr/bin/mpicxx $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gavinconant/software/mhm2-v2.1.0/.build/makeVersionFile/version.cpp > CMakeFiles/MHM2_VERSION.dir/makeVersionFile/version.cpp.i

CMakeFiles/MHM2_VERSION.dir/makeVersionFile/version.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/MHM2_VERSION.dir/makeVersionFile/version.cpp.s"
	/usr/bin/mpicxx $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gavinconant/software/mhm2-v2.1.0/.build/makeVersionFile/version.cpp -o CMakeFiles/MHM2_VERSION.dir/makeVersionFile/version.cpp.s

MHM2_VERSION: CMakeFiles/MHM2_VERSION.dir/makeVersionFile/version.cpp.o
MHM2_VERSION: CMakeFiles/MHM2_VERSION.dir/build.make

.PHONY : MHM2_VERSION

# Rule to build all files generated by this target.
CMakeFiles/MHM2_VERSION.dir/build: MHM2_VERSION

.PHONY : CMakeFiles/MHM2_VERSION.dir/build

CMakeFiles/MHM2_VERSION.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/MHM2_VERSION.dir/cmake_clean.cmake
.PHONY : CMakeFiles/MHM2_VERSION.dir/clean

CMakeFiles/MHM2_VERSION.dir/depend:
	cd /home/gavinconant/software/mhm2-v2.1.0/.build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gavinconant/software/mhm2-v2.1.0 /home/gavinconant/software/mhm2-v2.1.0 /home/gavinconant/software/mhm2-v2.1.0/.build /home/gavinconant/software/mhm2-v2.1.0/.build /home/gavinconant/software/mhm2-v2.1.0/.build/CMakeFiles/MHM2_VERSION.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/MHM2_VERSION.dir/depend
