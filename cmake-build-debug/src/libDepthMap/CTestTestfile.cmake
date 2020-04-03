# CMake generated Testfile for 
# Source directory: /Users/dave/Animesh/src/libDepthMap
# Build directory: /Users/dave/Animesh/cmake-build-debug/src/libDepthMap
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(FileMissingShouldThrow "/Users/dave/Animesh/bin/testDepthMap" "--gtest_filter=FileMissingShouldThrow")
set_tests_properties(FileMissingShouldThrow PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libDepthMap/CMakeLists.txt;80;add_test;/Users/dave/Animesh/src/libDepthMap/CMakeLists.txt;0;")
