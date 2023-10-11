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
include src/ssw/CMakeFiles/SSW_LIBRARY.dir/depend.make

# Include the progress variables for this target.
include src/ssw/CMakeFiles/SSW_LIBRARY.dir/progress.make

# Include the compile flags for this target's objects.
include src/ssw/CMakeFiles/SSW_LIBRARY.dir/flags.make

src/ssw/CMakeFiles/SSW_LIBRARY.dir/ssw.cpp.o: src/ssw/CMakeFiles/SSW_LIBRARY.dir/flags.make
src/ssw/CMakeFiles/SSW_LIBRARY.dir/ssw.cpp.o: ../src/ssw/ssw.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gavinconant/software/mhm2-v2.1.0/.build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/ssw/CMakeFiles/SSW_LIBRARY.dir/ssw.cpp.o"
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/src/ssw && /usr/bin/mpicxx  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/SSW_LIBRARY.dir/ssw.cpp.o -c /home/gavinconant/software/mhm2-v2.1.0/src/ssw/ssw.cpp

src/ssw/CMakeFiles/SSW_LIBRARY.dir/ssw.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SSW_LIBRARY.dir/ssw.cpp.i"
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/src/ssw && /usr/bin/mpicxx $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gavinconant/software/mhm2-v2.1.0/src/ssw/ssw.cpp > CMakeFiles/SSW_LIBRARY.dir/ssw.cpp.i

src/ssw/CMakeFiles/SSW_LIBRARY.dir/ssw.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SSW_LIBRARY.dir/ssw.cpp.s"
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/src/ssw && /usr/bin/mpicxx $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gavinconant/software/mhm2-v2.1.0/src/ssw/ssw.cpp -o CMakeFiles/SSW_LIBRARY.dir/ssw.cpp.s

src/ssw/CMakeFiles/SSW_LIBRARY.dir/ssw_core.cpp.o: src/ssw/CMakeFiles/SSW_LIBRARY.dir/flags.make
src/ssw/CMakeFiles/SSW_LIBRARY.dir/ssw_core.cpp.o: ../src/ssw/ssw_core.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/gavinconant/software/mhm2-v2.1.0/.build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/ssw/CMakeFiles/SSW_LIBRARY.dir/ssw_core.cpp.o"
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/src/ssw && /usr/bin/mpicxx  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/SSW_LIBRARY.dir/ssw_core.cpp.o -c /home/gavinconant/software/mhm2-v2.1.0/src/ssw/ssw_core.cpp

src/ssw/CMakeFiles/SSW_LIBRARY.dir/ssw_core.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SSW_LIBRARY.dir/ssw_core.cpp.i"
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/src/ssw && /usr/bin/mpicxx $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/gavinconant/software/mhm2-v2.1.0/src/ssw/ssw_core.cpp > CMakeFiles/SSW_LIBRARY.dir/ssw_core.cpp.i

src/ssw/CMakeFiles/SSW_LIBRARY.dir/ssw_core.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SSW_LIBRARY.dir/ssw_core.cpp.s"
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/src/ssw && /usr/bin/mpicxx $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/gavinconant/software/mhm2-v2.1.0/src/ssw/ssw_core.cpp -o CMakeFiles/SSW_LIBRARY.dir/ssw_core.cpp.s

# Object files for target SSW_LIBRARY
SSW_LIBRARY_OBJECTS = \
"CMakeFiles/SSW_LIBRARY.dir/ssw.cpp.o" \
"CMakeFiles/SSW_LIBRARY.dir/ssw_core.cpp.o"

# External object files for target SSW_LIBRARY
SSW_LIBRARY_EXTERNAL_OBJECTS =

src/ssw/libSSW_LIBRARY.a: src/ssw/CMakeFiles/SSW_LIBRARY.dir/ssw.cpp.o
src/ssw/libSSW_LIBRARY.a: src/ssw/CMakeFiles/SSW_LIBRARY.dir/ssw_core.cpp.o
src/ssw/libSSW_LIBRARY.a: src/ssw/CMakeFiles/SSW_LIBRARY.dir/build.make
src/ssw/libSSW_LIBRARY.a: src/ssw/CMakeFiles/SSW_LIBRARY.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/gavinconant/software/mhm2-v2.1.0/.build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libSSW_LIBRARY.a"
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/src/ssw && $(CMAKE_COMMAND) -P CMakeFiles/SSW_LIBRARY.dir/cmake_clean_target.cmake
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/src/ssw && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/SSW_LIBRARY.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/ssw/CMakeFiles/SSW_LIBRARY.dir/build: src/ssw/libSSW_LIBRARY.a

.PHONY : src/ssw/CMakeFiles/SSW_LIBRARY.dir/build

src/ssw/CMakeFiles/SSW_LIBRARY.dir/clean:
	cd /home/gavinconant/software/mhm2-v2.1.0/.build/src/ssw && $(CMAKE_COMMAND) -P CMakeFiles/SSW_LIBRARY.dir/cmake_clean.cmake
.PHONY : src/ssw/CMakeFiles/SSW_LIBRARY.dir/clean

src/ssw/CMakeFiles/SSW_LIBRARY.dir/depend:
	cd /home/gavinconant/software/mhm2-v2.1.0/.build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gavinconant/software/mhm2-v2.1.0 /home/gavinconant/software/mhm2-v2.1.0/src/ssw /home/gavinconant/software/mhm2-v2.1.0/.build /home/gavinconant/software/mhm2-v2.1.0/.build/src/ssw /home/gavinconant/software/mhm2-v2.1.0/.build/src/ssw/CMakeFiles/SSW_LIBRARY.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/ssw/CMakeFiles/SSW_LIBRARY.dir/depend
