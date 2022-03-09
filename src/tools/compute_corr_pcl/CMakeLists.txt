# OBJ File Converter

add_executable(
        compute_correspondences_pcl
        main.cpp
)

target_include_directories(
        compute_correspondences_pcl
        PUBLIC
        ${PCL_INCLUDE_DIRS}
)

target_link_libraries(
        compute_correspondences_pcl
        PUBLIC
        Correspondence
        Properties
        Surfel
        Tool
        Utilities
        ${PCL_LIBRARIES}
)

target_compile_features(
        compute_correspondences_pcl
        PUBLIC
        cxx_std_11
)