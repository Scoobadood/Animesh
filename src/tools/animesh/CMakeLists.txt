set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND CMAKE_PREFIX_PATH "/Users/dave/Qt/5.15.2/clang_64/lib/cmake/Qt5")
list(APPEND CMAKE_PREFIX_PATH "/Users/dave/Qt/5.15.2/clang_64/lib/cmake/Qt5Widgets")
list(APPEND CMAKE_PREFIX_PATH "/Users/dave/Qt/5.15.2/clang_64/lib/cmake/Qt5OpenGL")
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(animesh
		src/main.cpp
		src/AnimeshWindow.cpp include/AnimeshWindow.h
		src/AnimeshApp.cpp include/AnimeshApp.h
		src/AnimeshGLWidget.cpp include/AnimeshGLWidget.h
		src/AnimeshWindow.ui
)

target_include_directories(
		animesh
		PRIVATE
		include
		../build-animesh-Desktop_Qt_5_15_2_clang_64bit-Debug/animesh_autogen/include
)

target_link_libraries(
		animesh
		PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
		${OPENGL_LIBRARIES}
		Qt5::Widgets
		Qt5::OpenGL
		spdlog::spdlog
		ArcBall
		Properties
		Surfel
		Tool
)
