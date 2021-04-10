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
        posy_visualiser_window.cpp
        posy_visualiser_window.h
        posy_gl_widget.cpp
        posy_gl_widget.h
        posy_visualiser_window.ui
)

add_executable(posy_visualiser
    ${PROJECT_SOURCES}
)

target_include_directories(
                posy_visualiser
                PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(posy_visualiser
    PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
    ${OPENGL_LIBRARIES}
    Qt5::Widgets
    Qt5::OpenGL
    Surfel)
