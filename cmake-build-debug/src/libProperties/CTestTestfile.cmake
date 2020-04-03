# CMake generated Testfile for 
# Source directory: /Users/dave/Animesh/src/libProperties
# Build directory: /Users/dave/Animesh/cmake-build-debug/src/libProperties
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(TestProperties.EmptyFileShouldLoad "/Users/dave/Animesh/bin/testProperties" "--gtest_filter=TestProperties.EmptyFileShouldLoad")
set_tests_properties(TestProperties.EmptyFileShouldLoad PROPERTIES  WORKING_DIRECTORY "/Users/dave/Animesh/cmake-build-debug/properties_test_data" _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libProperties/CMakeLists.txt;40;add_test;/Users/dave/Animesh/src/libProperties/CMakeLists.txt;0;")
add_test(TestProperties.IntegerValueShouldReadWithSpaces "/Users/dave/Animesh/bin/testProperties" "--gtest_filter=TestProperties.IntegerValueShouldReadWithSpaces")
set_tests_properties(TestProperties.IntegerValueShouldReadWithSpaces PROPERTIES  WORKING_DIRECTORY "/Users/dave/Animesh/cmake-build-debug/properties_test_data" _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libProperties/CMakeLists.txt;44;add_test;/Users/dave/Animesh/src/libProperties/CMakeLists.txt;0;")
add_test(TestProperties.IntegerValueShouldReadWithNoSpaces "/Users/dave/Animesh/bin/testProperties" "--gtest_filter=TestProperties.IntegerValueShouldReadWithNoSpaces")
set_tests_properties(TestProperties.IntegerValueShouldReadWithNoSpaces PROPERTIES  WORKING_DIRECTORY "/Users/dave/Animesh/cmake-build-debug/properties_test_data" _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libProperties/CMakeLists.txt;48;add_test;/Users/dave/Animesh/src/libProperties/CMakeLists.txt;0;")
