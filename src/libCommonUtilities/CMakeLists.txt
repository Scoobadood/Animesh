add_library(
        CommonUtilities
        SHARED
        src/split.cpp include/CommonUtilities/split.h
        )

target_include_directories(
        CommonUtilities
        PUBLIC
        ${EIGEN3_INCLUDE_DIR}
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        include/CommonUtilities
)

target_link_libraries(
        CommonUtilities
)
