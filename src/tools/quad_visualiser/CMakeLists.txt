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
		quad_gl_widget.h
		quad_gl_widget.cpp
		quad_visualiser_window.cpp
		quad_visualiser_window.h
		quad_visualiser_window.ui
		)

add_executable(quad_visualiser
		${PROJECT_SOURCES}
		)

target_include_directories(
		quad_visualiser
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${EIGEN3_INCLUDE_DIR}
		../../third_party/include
)

target_link_libraries(
		quad_visualiser
		PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
		${OPENGL_LIBRARIES}
		Qt5::Widgets
		Qt5::OpenGL
		ArcBall
		Graph
		PoSy
		Properties
		Quad
		RoSy
		Surfel
)
