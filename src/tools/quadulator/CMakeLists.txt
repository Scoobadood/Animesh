add_executable(
		quadulator
		main.cpp
)

target_include_directories(
		quadulator
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
		quadulator
		Quad
		spdlog::spdlog
)
