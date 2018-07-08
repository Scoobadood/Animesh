cmake_minimum_required (VERSION 3.8 FATAL_ERROR)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

project( Animesh
         VERSION 0.1 
         LANGUAGES CXX )


# include_directories(${CMAKE_CURRENT_SOURCE_DIR})


# Write targets to bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../lib)

set(CMAKE_CXX_STANDARD 11)

# Enable Test 
enable_testing( )

# Add libraries
add_subdirectory( QtMain )
add_subdirectory( main )
add_subdirectory( libRoSy )
add_subdirectory( libGraph )
add_subdirectory( libElement )
add_subdirectory( libField )
add_subdirectory( libArgs )
add_subdirectory( libFileUtils )


# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src"
                 "${CMAKE_BINARY_DIR}/googletest-build")

# The gtest/gmock targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if(CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
                        "${gmock_SOURCE_DIR}/include")
endif()
