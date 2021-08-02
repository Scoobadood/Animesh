set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND CMAKE_PREFIX_PATH "/Users/dave/Qt/5.15.2/clang_64/lib/cmake/Qt5")
list(APPEND CMAKE_PREFIX_PATH "/Users/dave/Qt/5.15.2/clang_64/lib/cmake/Qt5Widgets")
list(APPEND CMAKE_PREFIX_PATH "/Users/dave/Qt/5.15.2/clang_64/lib/cmake/Qt5OpenGL")
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(OpenGL REQUIRED)

set(PROJECT_SOURCES
		main.cpp
		field_gl_widget.h
		field_gl_widget.cpp
		field_visualiser_window.cpp
		field_visualiser_window.h
		field_visualiser_window.ui

		posy_gl_widget.cpp
		posy_gl_widget.h
		posy_surfel_graph_geometry_extractor.cpp
		posy_surfel_graph_geometry_extractor.h

		rosy_gl_widget.cpp
		rosy_gl_widget.h
		rosy_surfel_graph_geometry_extractor.h
		rosy_surfel_graph_geometry_extractor.cpp

		quad_gl_widget.cpp
		quad_gl_widget.h
		quad_geometry_extractor.cpp
		quad_geometry_extractor.h

		gl_tools.cpp gl_tools.h
		)

add_executable(field_visualiser
		${PROJECT_SOURCES}
		)

target_include_directories(
		field_visualiser
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
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
)
