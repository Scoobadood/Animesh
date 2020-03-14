file(GLOB SOURCES "src/*.cpp")

add_library(DepthMap STATIC
        src/Normals.cpp  include/DepthMap/Normals.h
        src/CrossProductNormals.cpp  include/DepthMap/CrossProductNormals.h
        src/PclNormals.cpp  include/DepthMap/PclNormals.h
        src/PlaneFittingNormals.cpp  include/DepthMap/PlaneFittingNormals.h
        src/DepthMap.cpp  include/DepthMap/DepthMap.h
)

# Need PCL
find_package(PCL 1.8 REQUIRED)
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})


# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(DepthMap
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${EIGEN3_INCLUDE_DIR}
        ${PCL_INCLUDE_DIRS}
        PRIVATE
        src)

target_link_libraries(DepthMap
        Camera
        FileUtils
        Geom
        ${PCL_LIBRARIES}
        )

# This makes the project importable from the build directory
export(TARGETS DepthMap
        FILE DepthMapLibraryConfig.cmake)


# GTest requires pthreads
# find_package( Threads REQUIRED )
# find_package( GTest REQUIRED )

# Include specific tests
# include_directories("${GTEST_INCLUDE_DIRS}")

add_executable(testDepthMap
        tests/main.cpp
        tests/TestDepthMap.cpp
        tests/TestDepthMap.h)

target_link_libraries(testDepthMap
        DepthMap
        Geom
        ${PCL_LIBRARIES}
        gtest
        gmock)

add_custom_command(
        TARGET testDepthMap
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_data ${CMAKE_BINARY_DIR}/depthmap_test_data
        COMMENT "Copying unit test data."
        DEPENDS testDepthMap
)

add_executable(testPlanarNormals
        tests/main.cpp
        tests/TestPlanarNormals.cpp
        tests/TestPlanarNormals.h)

target_link_libraries(testPlanarNormals
        DepthMap
        ${PCL_LIBRARIES}
        gtest
        gmock)

add_test(NAME FileMissingShouldThrow COMMAND testDepthMap --gtest_filter=FileMissingShouldThrow)

# Stash it
install(TARGETS testDepthMap DESTINATION bin)
install(TARGETS testPlanarNormals DESTINATION bin)