set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND CMAKE_PREFIX_PATH "/Users/dave/Qt/5.15.2/clang_64/lib/cmake/Qt5")
list(APPEND CMAKE_PREFIX_PATH "/Users/dave/Qt/5.15.2/clang_64/lib/cmake/Qt5Widgets")
list(APPEND CMAKE_PREFIX_PATH "/Users/dave/Qt/5.15.2/clang_64/lib/cmake/Qt5OpenGL")
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(field_visualiser
		main.cpp
		field_gl_widget.h
		field_gl_widget.cpp
		field_visualiser_window.cpp
		field_visualiser_window.h
		field_visualiser_window.ui
		geometry_extractor.h

		posy/posy_gl_widget.cpp
		posy/posy_gl_widget.h
		posy/posy_surfel_graph_geometry_extractor.cpp
		posy/posy_surfel_graph_geometry_extractor.h

		rosy/rosy_gl_widget.cpp
		rosy/rosy_gl_widget.h
		rosy/rosy_surfel_graph_geometry_extractor.h
		rosy/rosy_surfel_graph_geometry_extractor.cpp

		quad/quad_gl_widget.cpp
		quad/quad_gl_widget.h
		quad/quad_geometry_extractor.cpp
		quad/quad_geometry_extractor.h)

target_include_directories(
		field_visualiser
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		PRIVATE quad rosy posy
)

target_link_libraries(
		field_visualiser
		PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
		${OPENGL_LIBRARIES}
		Qt5::Widgets
		Qt5::OpenGL
		ArcBall
		Properties
		Quad
		Surfel
		spdlog::spdlog
)
