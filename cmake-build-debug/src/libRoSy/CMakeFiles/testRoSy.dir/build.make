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
include src/libRoSy/CMakeFiles/testRoSy.dir/depend.make

# Include the progress variables for this target.
include src/libRoSy/CMakeFiles/testRoSy.dir/progress.make

# Include the compile flags for this target's objects.
include src/libRoSy/CMakeFiles/testRoSy.dir/flags.make

src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestAngleBetweenVectors.cpp.o: src/libRoSy/CMakeFiles/testRoSy.dir/flags.make
src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestAngleBetweenVectors.cpp.o: ../src/libRoSy/tests/TestAngleBetweenVectors.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestAngleBetweenVectors.cpp.o"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/testRoSy.dir/tests/TestAngleBetweenVectors.cpp.o -c /Users/dave/Animesh/src/libRoSy/tests/TestAngleBetweenVectors.cpp

src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestAngleBetweenVectors.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testRoSy.dir/tests/TestAngleBetweenVectors.cpp.i"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dave/Animesh/src/libRoSy/tests/TestAngleBetweenVectors.cpp > CMakeFiles/testRoSy.dir/tests/TestAngleBetweenVectors.cpp.i

src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestAngleBetweenVectors.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testRoSy.dir/tests/TestAngleBetweenVectors.cpp.s"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dave/Animesh/src/libRoSy/tests/TestAngleBetweenVectors.cpp -o CMakeFiles/testRoSy.dir/tests/TestAngleBetweenVectors.cpp.s

src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestMinimiseKL.cpp.o: src/libRoSy/CMakeFiles/testRoSy.dir/flags.make
src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestMinimiseKL.cpp.o: ../src/libRoSy/tests/TestMinimiseKL.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestMinimiseKL.cpp.o"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/testRoSy.dir/tests/TestMinimiseKL.cpp.o -c /Users/dave/Animesh/src/libRoSy/tests/TestMinimiseKL.cpp

src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestMinimiseKL.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testRoSy.dir/tests/TestMinimiseKL.cpp.i"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dave/Animesh/src/libRoSy/tests/TestMinimiseKL.cpp > CMakeFiles/testRoSy.dir/tests/TestMinimiseKL.cpp.i

src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestMinimiseKL.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testRoSy.dir/tests/TestMinimiseKL.cpp.s"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dave/Animesh/src/libRoSy/tests/TestMinimiseKL.cpp -o CMakeFiles/testRoSy.dir/tests/TestMinimiseKL.cpp.s

src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestRoSy.cpp.o: src/libRoSy/CMakeFiles/testRoSy.dir/flags.make
src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestRoSy.cpp.o: ../src/libRoSy/tests/TestRoSy.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestRoSy.cpp.o"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/testRoSy.dir/tests/TestRoSy.cpp.o -c /Users/dave/Animesh/src/libRoSy/tests/TestRoSy.cpp

src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestRoSy.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testRoSy.dir/tests/TestRoSy.cpp.i"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dave/Animesh/src/libRoSy/tests/TestRoSy.cpp > CMakeFiles/testRoSy.dir/tests/TestRoSy.cpp.i

src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestRoSy.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testRoSy.dir/tests/TestRoSy.cpp.s"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dave/Animesh/src/libRoSy/tests/TestRoSy.cpp -o CMakeFiles/testRoSy.dir/tests/TestRoSy.cpp.s

src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestVectorRotation.cpp.o: src/libRoSy/CMakeFiles/testRoSy.dir/flags.make
src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestVectorRotation.cpp.o: ../src/libRoSy/tests/TestVectorRotation.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestVectorRotation.cpp.o"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/testRoSy.dir/tests/TestVectorRotation.cpp.o -c /Users/dave/Animesh/src/libRoSy/tests/TestVectorRotation.cpp

src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestVectorRotation.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testRoSy.dir/tests/TestVectorRotation.cpp.i"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dave/Animesh/src/libRoSy/tests/TestVectorRotation.cpp > CMakeFiles/testRoSy.dir/tests/TestVectorRotation.cpp.i

src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestVectorRotation.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testRoSy.dir/tests/TestVectorRotation.cpp.s"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dave/Animesh/src/libRoSy/tests/TestVectorRotation.cpp -o CMakeFiles/testRoSy.dir/tests/TestVectorRotation.cpp.s

src/libRoSy/CMakeFiles/testRoSy.dir/tests/main.cpp.o: src/libRoSy/CMakeFiles/testRoSy.dir/flags.make
src/libRoSy/CMakeFiles/testRoSy.dir/tests/main.cpp.o: ../src/libRoSy/tests/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object src/libRoSy/CMakeFiles/testRoSy.dir/tests/main.cpp.o"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/testRoSy.dir/tests/main.cpp.o -c /Users/dave/Animesh/src/libRoSy/tests/main.cpp

src/libRoSy/CMakeFiles/testRoSy.dir/tests/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/testRoSy.dir/tests/main.cpp.i"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dave/Animesh/src/libRoSy/tests/main.cpp > CMakeFiles/testRoSy.dir/tests/main.cpp.i

src/libRoSy/CMakeFiles/testRoSy.dir/tests/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/testRoSy.dir/tests/main.cpp.s"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dave/Animesh/src/libRoSy/tests/main.cpp -o CMakeFiles/testRoSy.dir/tests/main.cpp.s

# Object files for target testRoSy
testRoSy_OBJECTS = \
"CMakeFiles/testRoSy.dir/tests/TestAngleBetweenVectors.cpp.o" \
"CMakeFiles/testRoSy.dir/tests/TestMinimiseKL.cpp.o" \
"CMakeFiles/testRoSy.dir/tests/TestRoSy.cpp.o" \
"CMakeFiles/testRoSy.dir/tests/TestVectorRotation.cpp.o" \
"CMakeFiles/testRoSy.dir/tests/main.cpp.o"

# External object files for target testRoSy
testRoSy_EXTERNAL_OBJECTS =

../bin/testRoSy: src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestAngleBetweenVectors.cpp.o
../bin/testRoSy: src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestMinimiseKL.cpp.o
../bin/testRoSy: src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestRoSy.cpp.o
../bin/testRoSy: src/libRoSy/CMakeFiles/testRoSy.dir/tests/TestVectorRotation.cpp.o
../bin/testRoSy: src/libRoSy/CMakeFiles/testRoSy.dir/tests/main.cpp.o
../bin/testRoSy: src/libRoSy/CMakeFiles/testRoSy.dir/build.make
../bin/testRoSy: ../lib/libRoSy.a
../bin/testRoSy: ../lib/libField.a
../bin/testRoSy: ../lib/libRoSy.a
../bin/testRoSy: ../lib/libField.a
../bin/testRoSy: ../lib/libFileUtils.a
../bin/testRoSy: ../lib/libGeom.a
../bin/testRoSy: ../lib/libGraph.a
../bin/testRoSy: src/libRoSy/CMakeFiles/testRoSy.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable ../../../bin/testRoSy"
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/testRoSy.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/libRoSy/CMakeFiles/testRoSy.dir/build: ../bin/testRoSy

.PHONY : src/libRoSy/CMakeFiles/testRoSy.dir/build

src/libRoSy/CMakeFiles/testRoSy.dir/clean:
	cd /Users/dave/Animesh/cmake-build-debug/src/libRoSy && $(CMAKE_COMMAND) -P CMakeFiles/testRoSy.dir/cmake_clean.cmake
.PHONY : src/libRoSy/CMakeFiles/testRoSy.dir/clean

src/libRoSy/CMakeFiles/testRoSy.dir/depend:
	cd /Users/dave/Animesh/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/dave/Animesh /Users/dave/Animesh/src/libRoSy /Users/dave/Animesh/cmake-build-debug /Users/dave/Animesh/cmake-build-debug/src/libRoSy /Users/dave/Animesh/cmake-build-debug/src/libRoSy/CMakeFiles/testRoSy.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/libRoSy/CMakeFiles/testRoSy.dir/depend

