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
include src/libCamera/CMakeFiles/testCamera.dir/depend.make

# Include the progress variables for this target.
include src/libCamera/CMakeFiles/testCamera.dir/progress.make

# Include the compile flags for this target's objects.
include src/libCamera/CMakeFiles/testCamera.dir/flags.make

src/libCamera/CMakeFiles/testCamera.dir/tests/TestCamera.cpp.o: src/libCamera/CMakeFiles/testCamera.dir/flags.make
src/libCamera/CMakeFiles/testCamera.dir/tests/TestCamera.cpp.o: ../src/libCamera/tests/TestCamera.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/libCamera/CMakeFiles/testCamera.dir/tests/TestCamera.cpp.o"
	cd /Users/dave/Animesh/cmake-build-debug/src/libCamera && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/testCamera.dir/tests/TestCamera.cpp.o -c /Users/dave/Animesh/src/libCamera/tests/TestCamera.cpp

src/libCamera/CMakeFiles/testCamera.dir/tests/TestCamera.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testCamera.dir/tests/TestCamera.cpp.i"
	cd /Users/dave/Animesh/cmake-build-debug/src/libCamera && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dave/Animesh/src/libCamera/tests/TestCamera.cpp > CMakeFiles/testCamera.dir/tests/TestCamera.cpp.i

src/libCamera/CMakeFiles/testCamera.dir/tests/TestCamera.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testCamera.dir/tests/TestCamera.cpp.s"
	cd /Users/dave/Animesh/cmake-build-debug/src/libCamera && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dave/Animesh/src/libCamera/tests/TestCamera.cpp -o CMakeFiles/testCamera.dir/tests/TestCamera.cpp.s

src/libCamera/CMakeFiles/testCamera.dir/tests/main.cpp.o: src/libCamera/CMakeFiles/testCamera.dir/flags.make
src/libCamera/CMakeFiles/testCamera.dir/tests/main.cpp.o: ../src/libCamera/tests/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/libCamera/CMakeFiles/testCamera.dir/tests/main.cpp.o"
	cd /Users/dave/Animesh/cmake-build-debug/src/libCamera && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/testCamera.dir/tests/main.cpp.o -c /Users/dave/Animesh/src/libCamera/tests/main.cpp

src/libCamera/CMakeFiles/testCamera.dir/tests/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testCamera.dir/tests/main.cpp.i"
	cd /Users/dave/Animesh/cmake-build-debug/src/libCamera && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dave/Animesh/src/libCamera/tests/main.cpp > CMakeFiles/testCamera.dir/tests/main.cpp.i

src/libCamera/CMakeFiles/testCamera.dir/tests/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testCamera.dir/tests/main.cpp.s"
	cd /Users/dave/Animesh/cmake-build-debug/src/libCamera && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dave/Animesh/src/libCamera/tests/main.cpp -o CMakeFiles/testCamera.dir/tests/main.cpp.s

# Object files for target testCamera
testCamera_OBJECTS = \
"CMakeFiles/testCamera.dir/tests/TestCamera.cpp.o" \
"CMakeFiles/testCamera.dir/tests/main.cpp.o"

# External object files for target testCamera
testCamera_EXTERNAL_OBJECTS =

../bin/testCamera: src/libCamera/CMakeFiles/testCamera.dir/tests/TestCamera.cpp.o
../bin/testCamera: src/libCamera/CMakeFiles/testCamera.dir/tests/main.cpp.o
../bin/testCamera: src/libCamera/CMakeFiles/testCamera.dir/build.make
../bin/testCamera: ../lib/libCamera.a
../bin/testCamera: src/libCamera/CMakeFiles/testCamera.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../../../bin/testCamera"
	cd /Users/dave/Animesh/cmake-build-debug/src/libCamera && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testCamera.dir/link.txt --verbose=$(VERBOSE)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold "Copying unit test data."
	cd /Users/dave/Animesh/cmake-build-debug/src/libCamera && /usr/local/Cellar/cmake/3.16.5/bin/cmake -E copy_directory /Users/dave/Animesh/src/libCamera/tests/camera_test_data /Users/dave/Animesh/cmake-build-debug/camera_test_data

# Rule to build all files generated by this target.
src/libCamera/CMakeFiles/testCamera.dir/build: ../bin/testCamera

.PHONY : src/libCamera/CMakeFiles/testCamera.dir/build

src/libCamera/CMakeFiles/testCamera.dir/clean:
	cd /Users/dave/Animesh/cmake-build-debug/src/libCamera && $(CMAKE_COMMAND) -P CMakeFiles/testCamera.dir/cmake_clean.cmake
.PHONY : src/libCamera/CMakeFiles/testCamera.dir/clean

src/libCamera/CMakeFiles/testCamera.dir/depend:
	cd /Users/dave/Animesh/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/dave/Animesh /Users/dave/Animesh/src/libCamera /Users/dave/Animesh/cmake-build-debug /Users/dave/Animesh/cmake-build-debug/src/libCamera /Users/dave/Animesh/cmake-build-debug/src/libCamera/CMakeFiles/testCamera.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/libCamera/CMakeFiles/testCamera.dir/depend
