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
include third_party/nanogui/CMakeFiles/example3.dir/depend.make

# Include the progress variables for this target.
include third_party/nanogui/CMakeFiles/example3.dir/progress.make

# Include the compile flags for this target's objects.
include third_party/nanogui/CMakeFiles/example3.dir/flags.make

third_party/nanogui/CMakeFiles/example3.dir/src/example3.cpp.o: third_party/nanogui/CMakeFiles/example3.dir/flags.make
third_party/nanogui/CMakeFiles/example3.dir/src/example3.cpp.o: ../third_party/nanogui/src/example3.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object third_party/nanogui/CMakeFiles/example3.dir/src/example3.cpp.o"
	cd /Users/dave/Animesh/cmake-build-debug/third_party/nanogui && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/example3.dir/src/example3.cpp.o -c /Users/dave/Animesh/third_party/nanogui/src/example3.cpp

third_party/nanogui/CMakeFiles/example3.dir/src/example3.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/example3.dir/src/example3.cpp.i"
	cd /Users/dave/Animesh/cmake-build-debug/third_party/nanogui && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dave/Animesh/third_party/nanogui/src/example3.cpp > CMakeFiles/example3.dir/src/example3.cpp.i

third_party/nanogui/CMakeFiles/example3.dir/src/example3.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/example3.dir/src/example3.cpp.s"
	cd /Users/dave/Animesh/cmake-build-debug/third_party/nanogui && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dave/Animesh/third_party/nanogui/src/example3.cpp -o CMakeFiles/example3.dir/src/example3.cpp.s

# Object files for target example3
example3_OBJECTS = \
"CMakeFiles/example3.dir/src/example3.cpp.o"

# External object files for target example3
example3_EXTERNAL_OBJECTS =

third_party/nanogui/example3: third_party/nanogui/CMakeFiles/example3.dir/src/example3.cpp.o
third_party/nanogui/example3: third_party/nanogui/CMakeFiles/example3.dir/build.make
third_party/nanogui/example3: third_party/nanogui/libnanogui.dylib
third_party/nanogui/example3: third_party/nanogui/CMakeFiles/example3.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable example3"
	cd /Users/dave/Animesh/cmake-build-debug/third_party/nanogui && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/example3.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
third_party/nanogui/CMakeFiles/example3.dir/build: third_party/nanogui/example3

.PHONY : third_party/nanogui/CMakeFiles/example3.dir/build

third_party/nanogui/CMakeFiles/example3.dir/clean:
	cd /Users/dave/Animesh/cmake-build-debug/third_party/nanogui && $(CMAKE_COMMAND) -P CMakeFiles/example3.dir/cmake_clean.cmake
.PHONY : third_party/nanogui/CMakeFiles/example3.dir/clean

third_party/nanogui/CMakeFiles/example3.dir/depend:
	cd /Users/dave/Animesh/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/dave/Animesh /Users/dave/Animesh/third_party/nanogui /Users/dave/Animesh/cmake-build-debug /Users/dave/Animesh/cmake-build-debug/third_party/nanogui /Users/dave/Animesh/cmake-build-debug/third_party/nanogui/CMakeFiles/example3.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : third_party/nanogui/CMakeFiles/example3.dir/depend

