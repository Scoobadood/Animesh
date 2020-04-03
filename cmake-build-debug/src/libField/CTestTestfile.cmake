# CMake generated Testfile for 
# Source directory: /Users/dave/Animesh/src/libField
# Build directory: /Users/dave/Animesh/cmake-build-debug/src/libField
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(TestFieldDataTangentVectorIsPerpendicularToNormal "/Users/dave/Animesh/bin/testField" "--gtest_filter=TestFieldData.TangentVectorIsPerpendicularToNormal")
set_tests_properties(TestFieldDataTangentVectorIsPerpendicularToNormal PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libField/CMakeLists.txt;40;add_test;/Users/dave/Animesh/src/libField/CMakeLists.txt;0;")
add_test(TestFieldDataTangentVectorIsUnit "/Users/dave/Animesh/bin/testField" "--gtest_filter=TestFieldData.TangentVectorIsUnit")
set_tests_properties(TestFieldDataTangentVectorIsUnit PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libField/CMakeLists.txt;41;add_test;/Users/dave/Animesh/src/libField/CMakeLists.txt;0;")
add_test(TestFieldDataKIsZero "/Users/dave/Animesh/bin/testField" "--gtest_filter=TestFieldData.KIsZero")
set_tests_properties(TestFieldDataKIsZero PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libField/CMakeLists.txt;42;add_test;/Users/dave/Animesh/src/libField/CMakeLists.txt;0;")
add_test(TestFieldElement_Pass "/Users/dave/Animesh/bin/testField" "--gtest_filter=TestFieldElement.passingTest")
set_tests_properties(TestFieldElement_Pass PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libField/CMakeLists.txt;45;add_test;/Users/dave/Animesh/src/libField/CMakeLists.txt;0;")
add_test(TestFieldElement_twoArgConstructWithNonUnitNormalShouldThrow "/Users/dave/Animesh/bin/testField" "--gtest_filter=TestFieldElement.twoArgConstructWithNonUnitNormalShouldThrow")
set_tests_properties(TestFieldElement_twoArgConstructWithNonUnitNormalShouldThrow PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libField/CMakeLists.txt;46;add_test;/Users/dave/Animesh/src/libField/CMakeLists.txt;0;")
add_test(TestFieldElement.twoArgConstructWithUnitNormalShouldGenerateUnitTangent "/Users/dave/Animesh/bin/testField" "--gtest_filter=TestFieldElement.twoArgConstructWithUnitNormalShouldGenerateUnitTangent")
set_tests_properties(TestFieldElement.twoArgConstructWithUnitNormalShouldGenerateUnitTangent PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libField/CMakeLists.txt;47;add_test;/Users/dave/Animesh/src/libField/CMakeLists.txt;0;")
add_test(TestFieldElement.twoArgConstructWithUnitNormalShouldGeneratePerpTangent "/Users/dave/Animesh/bin/testField" "--gtest_filter=TestFieldElement.twoArgConstructWithUnitNormalShouldGeneratePerpTangent")
set_tests_properties(TestFieldElement.twoArgConstructWithUnitNormalShouldGeneratePerpTangent PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libField/CMakeLists.txt;48;add_test;/Users/dave/Animesh/src/libField/CMakeLists.txt;0;")
add_test(TestFieldElement.threeArgConstructWithNonUnitNormalShouldThrow "/Users/dave/Animesh/bin/testField" "--gtest_filter=TestFieldElement.threeArgConstructWithNonUnitNormalShouldThrow")
set_tests_properties(TestFieldElement.threeArgConstructWithNonUnitNormalShouldThrow PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libField/CMakeLists.txt;49;add_test;/Users/dave/Animesh/src/libField/CMakeLists.txt;0;")
add_test(TestFieldElement.threeArgConstructWithNonUnitTangentShouldThrow "/Users/dave/Animesh/bin/testField" "--gtest_filter=TestFieldElement.threeArgConstructWithNonUnitTangentShouldThrow")
set_tests_properties(TestFieldElement.threeArgConstructWithNonUnitTangentShouldThrow PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libField/CMakeLists.txt;50;add_test;/Users/dave/Animesh/src/libField/CMakeLists.txt;0;")
add_test(TestFieldElement.threeArgConstructWithNonPerpTangentShouldThrow "/Users/dave/Animesh/bin/testField" "--gtest_filter=TestFieldElement.threeArgConstructWithNonPerpTangentShouldThrow")
set_tests_properties(TestFieldElement.threeArgConstructWithNonPerpTangentShouldThrow PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libField/CMakeLists.txt;51;add_test;/Users/dave/Animesh/src/libField/CMakeLists.txt;0;")
add_test(TestFieldElement.mergeIdenticalVectorsShouldGiveSameFE "/Users/dave/Animesh/bin/testField" "--gtest_filter=TestFieldElement.mergeIdenticalVectorsShouldGiveSameFE")
set_tests_properties(TestFieldElement.mergeIdenticalVectorsShouldGiveSameFE PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libField/CMakeLists.txt;52;add_test;/Users/dave/Animesh/src/libField/CMakeLists.txt;0;")
add_test(TestFieldElement.mergeVectorsWithCommonNormalShouldHaveSameNormal "/Users/dave/Animesh/bin/testField" "--gtest_filter=TestFieldElement.mergeVectorsWithCommonNormalShouldHaveSameNormal")
set_tests_properties(TestFieldElement.mergeVectorsWithCommonNormalShouldHaveSameNormal PROPERTIES  _BACKTRACE_TRIPLES "/Users/dave/Animesh/src/libField/CMakeLists.txt;53;add_test;/Users/dave/Animesh/src/libField/CMakeLists.txt;0;")
