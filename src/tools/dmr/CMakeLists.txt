find_package(glm)

# Depth map renderer
add_executable(
		dmr
		dmr.cpp
		glutils.cpp glutils.hpp
		mesh.cpp mesh.hpp
		model.cpp model.hpp
)
target_include_directories(
		dmr PUBLIC
		${GLM_INCLUDE_DIRS}
		${ASSIMP_INCLUDE_DIRS}
)

target_link_libraries(
		dmr
		glm::glm
		${ASSIMP_LIBRARY}
)

target_link_libraries(dmr ${EXTRA_LIBS} glfw ${ASSIMP_LIBRARY})
