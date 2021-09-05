# OBJ File Converter
add_executable(
		surfel_info
		surfel_info.h surfel_info.cpp
)

target_link_libraries(
		surfel_info
		Surfel
)