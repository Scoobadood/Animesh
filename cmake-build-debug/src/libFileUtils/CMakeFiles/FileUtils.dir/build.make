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
include src/libFileUtils/CMakeFiles/FileUtils.dir/depend.make

# Include the progress variables for this target.
include src/libFileUtils/CMakeFiles/FileUtils.dir/progress.make

# Include the compile flags for this target's objects.
include src/libFileUtils/CMakeFiles/FileUtils.dir/flags.make

src/libFileUtils/CMakeFiles/FileUtils.dir/src/FileUtils.cpp.o: src/libFileUtils/CMakeFiles/FileUtils.dir/flags.make
src/libFileUtils/CMakeFiles/FileUtils.dir/src/FileUtils.cpp.o: ../src/libFileUtils/src/FileUtils.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/libFileUtils/CMakeFiles/FileUtils.dir/src/FileUtils.cpp.o"
	cd /Users/dave/Animesh/cmake-build-debug/src/libFileUtils && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/FileUtils.dir/src/FileUtils.cpp.o -c /Users/dave/Animesh/src/libFileUtils/src/FileUtils.cpp

src/libFileUtils/CMakeFiles/FileUtils.dir/src/FileUtils.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/FileUtils.dir/src/FileUtils.cpp.i"
	cd /Users/dave/Animesh/cmake-build-debug/src/libFileUtils && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dave/Animesh/src/libFileUtils/src/FileUtils.cpp > CMakeFiles/FileUtils.dir/src/FileUtils.cpp.i

src/libFileUtils/CMakeFiles/FileUtils.dir/src/FileUtils.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/FileUtils.dir/src/FileUtils.cpp.s"
	cd /Users/dave/Animesh/cmake-build-debug/src/libFileUtils && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dave/Animesh/src/libFileUtils/src/FileUtils.cpp -o CMakeFiles/FileUtils.dir/src/FileUtils.cpp.s

src/libFileUtils/CMakeFiles/FileUtils.dir/src/ObjFileParser.cpp.o: src/libFileUtils/CMakeFiles/FileUtils.dir/flags.make
src/libFileUtils/CMakeFiles/FileUtils.dir/src/ObjFileParser.cpp.o: ../src/libFileUtils/src/ObjFileParser.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/libFileUtils/CMakeFiles/FileUtils.dir/src/ObjFileParser.cpp.o"
	cd /Users/dave/Animesh/cmake-build-debug/src/libFileUtils && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/FileUtils.dir/src/ObjFileParser.cpp.o -c /Users/dave/Animesh/src/libFileUtils/src/ObjFileParser.cpp

src/libFileUtils/CMakeFiles/FileUtils.dir/src/ObjFileParser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/FileUtils.dir/src/ObjFileParser.cpp.i"
	cd /Users/dave/Animesh/cmake-build-debug/src/libFileUtils && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dave/Animesh/src/libFileUtils/src/ObjFileParser.cpp > CMakeFiles/FileUtils.dir/src/ObjFileParser.cpp.i

src/libFileUtils/CMakeFiles/FileUtils.dir/src/ObjFileParser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/FileUtils.dir/src/ObjFileParser.cpp.s"
	cd /Users/dave/Animesh/cmake-build-debug/src/libFileUtils && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dave/Animesh/src/libFileUtils/src/ObjFileParser.cpp -o CMakeFiles/FileUtils.dir/src/ObjFileParser.cpp.s

src/libFileUtils/CMakeFiles/FileUtils.dir/src/PgmFileParser.cpp.o: src/libFileUtils/CMakeFiles/FileUtils.dir/flags.make
src/libFileUtils/CMakeFiles/FileUtils.dir/src/PgmFileParser.cpp.o: ../src/libFileUtils/src/PgmFileParser.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object src/libFileUtils/CMakeFiles/FileUtils.dir/src/PgmFileParser.cpp.o"
	cd /Users/dave/Animesh/cmake-build-debug/src/libFileUtils && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/FileUtils.dir/src/PgmFileParser.cpp.o -c /Users/dave/Animesh/src/libFileUtils/src/PgmFileParser.cpp

src/libFileUtils/CMakeFiles/FileUtils.dir/src/PgmFileParser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/FileUtils.dir/src/PgmFileParser.cpp.i"
	cd /Users/dave/Animesh/cmake-build-debug/src/libFileUtils && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /Users/dave/Animesh/src/libFileUtils/src/PgmFileParser.cpp > CMakeFiles/FileUtils.dir/src/PgmFileParser.cpp.i

src/libFileUtils/CMakeFiles/FileUtils.dir/src/PgmFileParser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/FileUtils.dir/src/PgmFileParser.cpp.s"
	cd /Users/dave/Animesh/cmake-build-debug/src/libFileUtils && /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /Users/dave/Animesh/src/libFileUtils/src/PgmFileParser.cpp -o CMakeFiles/FileUtils.dir/src/PgmFileParser.cpp.s

# Object files for target FileUtils
FileUtils_OBJECTS = \
"CMakeFiles/FileUtils.dir/src/FileUtils.cpp.o" \
"CMakeFiles/FileUtils.dir/src/ObjFileParser.cpp.o" \
"CMakeFiles/FileUtils.dir/src/PgmFileParser.cpp.o"

# External object files for target FileUtils
FileUtils_EXTERNAL_OBJECTS =

../lib/libFileUtils.a: src/libFileUtils/CMakeFiles/FileUtils.dir/src/FileUtils.cpp.o
../lib/libFileUtils.a: src/libFileUtils/CMakeFiles/FileUtils.dir/src/ObjFileParser.cpp.o
../lib/libFileUtils.a: src/libFileUtils/CMakeFiles/FileUtils.dir/src/PgmFileParser.cpp.o
../lib/libFileUtils.a: src/libFileUtils/CMakeFiles/FileUtils.dir/build.make
../lib/libFileUtils.a: src/libFileUtils/CMakeFiles/FileUtils.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/dave/Animesh/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Linking CXX static library ../../../lib/libFileUtils.a"
	cd /Users/dave/Animesh/cmake-build-debug/src/libFileUtils && $(CMAKE_COMMAND) -P CMakeFiles/FileUtils.dir/cmake_clean_target.cmake
	cd /Users/dave/Animesh/cmake-build-debug/src/libFileUtils && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/FileUtils.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/libFileUtils/CMakeFiles/FileUtils.dir/build: ../lib/libFileUtils.a

.PHONY : src/libFileUtils/CMakeFiles/FileUtils.dir/build

src/libFileUtils/CMakeFiles/FileUtils.dir/clean:
	cd /Users/dave/Animesh/cmake-build-debug/src/libFileUtils && $(CMAKE_COMMAND) -P CMakeFiles/FileUtils.dir/cmake_clean.cmake
.PHONY : src/libFileUtils/CMakeFiles/FileUtils.dir/clean

src/libFileUtils/CMakeFiles/FileUtils.dir/depend:
	cd /Users/dave/Animesh/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/dave/Animesh /Users/dave/Animesh/src/libFileUtils /Users/dave/Animesh/cmake-build-debug /Users/dave/Animesh/cmake-build-debug/src/libFileUtils /Users/dave/Animesh/cmake-build-debug/src/libFileUtils/CMakeFiles/FileUtils.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/libFileUtils/CMakeFiles/FileUtils.dir/depend

