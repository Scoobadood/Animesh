set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND CMAKE_PREFIX_PATH "/Users/dave/Qt/5.15.2/clang_64/lib/cmake/Qt5")
list(APPEND CMAKE_PREFIX_PATH "/Users/dave/Qt/5.15.2/clang_64/lib/cmake/Qt5Widgets")
list(APPEND CMAKE_PREFIX_PATH "/Users/dave/Qt/5.15.2/clang_64/lib/cmake/Qt5OpenGL")
find_package( Qt5Widgets REQUIRED )
find_package( Qt5OpenGL REQUIRED )
find_package( OpenGL REQUIRED )

set(PROJECT_SOURCES
        main.cpp
        rosy_visualiser_window.cpp
        rosy_visualiser_window.h
        rosy_gl_widget.cpp
        rosy_gl_widget.h
        surfel_graph_geometry_extractor.h
        surfel_graph_geometry_extractor.cpp
        rosy_visualiser_window.ui
        rosy_visualiser_status_bar.cpp
        rosy_visualiser_status_bar.h
        rosy_visualiser_status_bar.ui
		arc_ball.h arc_ball.cpp
)

add_executable(rosy_visualiser
    ${PROJECT_SOURCES}
)

target_include_directories(
                rosy_visualiser
                PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(rosy_visualiser
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    ${OPENGL_LIBRARIES}
    Qt5::Widgets
    Qt5::OpenGL
    Surfel)
