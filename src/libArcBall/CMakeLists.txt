list(APPEND CMAKE_PREFIX_PATH "/Users/dave/Qt/5.15.2/clang_64/lib/cmake/Qt5")
list(APPEND CMAKE_PREFIX_PATH "/Users/dave/Qt/5.15.2/clang_64/lib/cmake/Qt5Widgets")
find_package( Qt5Widgets REQUIRED )

add_library(
		ArcBall SHARED
		src/ArcBall.cpp
		include/ArcBall/ArcBall.h
)

target_include_directories(
		ArcBall PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
		PRIVATE
		include/ArcBall
)

target_link_libraries(
		ArcBall
		PRIVATE
		PRIVATE
		Geom
		Qt${QT_VERSION_MAJOR}::Widgets
		Qt5::Widgets
)