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
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.16.5/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.16.5/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/dave/Animesh

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/dave/Animesh/cmake-build-debug

# Include any dependencies generated for this target.
include src/libGraph/CMakeFiles/Graph.dir/depend.make

# Include the progress variables for this target.
include src/libGraph/CMakeFiles/Graph.dir/progress.make

# Include the compile flags for this target's objects.
include src/libGraph/CMakeFiles/Graph.dir/flags.make

src/libGraph/CMakeFiles/Graph.dir/src/Path.cpp.o: src/libGraph/CMakeFiles/Graph.dir/flags.make
src/libGraph/CMakeFiles/Graph.dir/src/Path.cpp.o: ../src/libGraph/src/Path.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/libGraph/CMakeFiles/Graph.dir/src/Path.cpp.o"
	cd /Users/dave/Animesh/cmake-build-debug/src/libGraph && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Graph.dir/src/Path.cpp.o -c /Users/dave/Animesh/src/libGraph/src/Path.cpp

src/libGraph/CMakeFiles/Graph.dir/src/Path.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Graph.dir/src/Path.cpp.i"
	cd /Users/dave/Animesh/cmake-build-debug/src/libGraph && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dave/Animesh/src/libGraph/src/Path.cpp > CMakeFiles/Graph.dir/src/Path.cpp.i

src/libGraph/CMakeFiles/Graph.dir/src/Path.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Graph.dir/src/Path.cpp.s"
	cd /Users/dave/Animesh/cmake-build-debug/src/libGraph && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dave/Animesh/src/libGraph/src/Path.cpp -o CMakeFiles/Graph.dir/src/Path.cpp.s

# Object files for target Graph
Graph_OBJECTS = \
"CMakeFiles/Graph.dir/src/Path.cpp.o"

# External object files for target Graph
Graph_EXTERNAL_OBJECTS =

../lib/libGraph.a: src/libGraph/CMakeFiles/Graph.dir/src/Path.cpp.o
../lib/libGraph.a: src/libGraph/CMakeFiles/Graph.dir/build.make
../lib/libGraph.a: src/libGraph/CMakeFiles/Graph.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library ../../../lib/libGraph.a"
	cd /Users/dave/Animesh/cmake-build-debug/src/libGraph && $(CMAKE_COMMAND) -P CMakeFiles/Graph.dir/cmake_clean_target.cmake
	cd /Users/dave/Animesh/cmake-build-debug/src/libGraph && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Graph.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/libGraph/CMakeFiles/Graph.dir/build: ../lib/libGraph.a

.PHONY : src/libGraph/CMakeFiles/Graph.dir/build

src/libGraph/CMakeFiles/Graph.dir/clean:
	cd /Users/dave/Animesh/cmake-build-debug/src/libGraph && $(CMAKE_COMMAND) -P CMakeFiles/Graph.dir/cmake_clean.cmake
.PHONY : src/libGraph/CMakeFiles/Graph.dir/clean

src/libGraph/CMakeFiles/Graph.dir/depend:
	cd /Users/dave/Animesh/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/dave/Animesh /Users/dave/Animesh/src/libGraph /Users/dave/Animesh/cmake-build-debug /Users/dave/Animesh/cmake-build-debug/src/libGraph /Users/dave/Animesh/cmake-build-debug/src/libGraph/CMakeFiles/Graph.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/libGraph/CMakeFiles/Graph.dir/depend
