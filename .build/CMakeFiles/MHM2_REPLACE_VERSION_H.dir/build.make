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

# Utility rule file for MHM2_REPLACE_VERSION_H.

# Include the progress variables for this target.
include CMakeFiles/MHM2_REPLACE_VERSION_H.dir/progress.make

CMakeFiles/MHM2_REPLACE_VERSION_H:
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/gavinconant/software/mhm2-v2.1.0/.build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building /home/gavinconant/software/mhm2-v2.1.0/.build/makeVersionFile/MHM2_VERSION and /home/gavinconant/software/mhm2-v2.1.0/.build/makeVersionFile/___MHM2_AUTOGEN_version.c"
	cd /home/gavinconant/software/mhm2-v2.1.0 && /usr/bin/cmake -DMHM2_VERSION= -DMHM2_VERSION_C_FILE=/home/gavinconant/software/mhm2-v2.1.0/.build/makeVersionFile/___MHM2_AUTOGEN_version.c -DMHM2_VERSION_FILE=/home/gavinconant/software/mhm2-v2.1.0/.build/makeVersionFile/MHM2_VERSION -DMHM2_VERSION_C_FILE_TEMPLATE=/home/gavinconant/software/mhm2-v2.1.0/.build/makeVersionFile/version.cpp.in -DBUILD_DATE=20230111_131515 -DMHM2_BUILD_DATE=20230111_131515 -DMHM2_BRANCH= -P /home/gavinconant/software/mhm2-v2.1.0/.build/makeVersionFile/MHM2_makeVersionFile.cmake

MHM2_REPLACE_VERSION_H: CMakeFiles/MHM2_REPLACE_VERSION_H
MHM2_REPLACE_VERSION_H: CMakeFiles/MHM2_REPLACE_VERSION_H.dir/build.make

.PHONY : MHM2_REPLACE_VERSION_H

# Rule to build all files generated by this target.
CMakeFiles/MHM2_REPLACE_VERSION_H.dir/build: MHM2_REPLACE_VERSION_H

.PHONY : CMakeFiles/MHM2_REPLACE_VERSION_H.dir/build

CMakeFiles/MHM2_REPLACE_VERSION_H.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/MHM2_REPLACE_VERSION_H.dir/cmake_clean.cmake
.PHONY : CMakeFiles/MHM2_REPLACE_VERSION_H.dir/clean

CMakeFiles/MHM2_REPLACE_VERSION_H.dir/depend:
	cd /home/gavinconant/software/mhm2-v2.1.0/.build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/gavinconant/software/mhm2-v2.1.0 /home/gavinconant/software/mhm2-v2.1.0 /home/gavinconant/software/mhm2-v2.1.0/.build /home/gavinconant/software/mhm2-v2.1.0/.build /home/gavinconant/software/mhm2-v2.1.0/.build/CMakeFiles/MHM2_REPLACE_VERSION_H.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/MHM2_REPLACE_VERSION_H.dir/depend
