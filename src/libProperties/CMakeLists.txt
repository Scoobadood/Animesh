file(GLOB SOURCES "src/*.cpp")

add_library(Properties STATIC ${SOURCES} src/Properties.cpp include/Properties/Properties.h)


# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(
        Properties PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${EIGEN3_INCLUDE_DIR}
        PRIVATE src)


# This makes the project importable from the build directory
export( TARGETS Properties
        FILE PropertiesLibraryConfig.cmake)


# Every library has unit tests, of course
file( GLOB TEST_SOURCES "tests/*.cpp" )
add_executable(
        testProperties
        ${TEST_SOURCES})

target_link_libraries(testProperties
        Properties gtest gmock)

# Copy test data
add_custom_command(
        TARGET testProperties
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_data ${CMAKE_BINARY_DIR}/properties_test_data
        COMMENT "Copying properties unit test data."
        DEPENDS testProperties
)

add_test( NAME TestProperties.EmptyFileShouldLoad
          COMMAND  testProperties --gtest_filter=TestProperties.EmptyFileShouldLoad
          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/properties_test_data)

add_test( NAME TestProperties.IntegerValueShouldReadWithSpaces
        COMMAND  testProperties --gtest_filter=TestProperties.IntegerValueShouldReadWithSpaces
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/properties_test_data)

add_test( NAME TestProperties.IntegerValueShouldReadWithNoSpaces
        COMMAND  testProperties --gtest_filter=TestProperties.IntegerValueShouldReadWithNoSpaces
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/properties_test_data)

# Stash it
install(TARGETS testProperties DESTINATION bin)

