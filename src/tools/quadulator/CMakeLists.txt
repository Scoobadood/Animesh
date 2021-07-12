set(
		PROJECT_SOURCES
		main.cpp
)

add_executable(
		quadulator
		${PROJECT_SOURCES}
)

target_include_directories(
		quadulator
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		../../third_party/include
)

target_link_libraries(
		quadulator
		Surfel
)
