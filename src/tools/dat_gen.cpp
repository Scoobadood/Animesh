/*
 * Generate cheat data.
 * Given an input OBJ file, moves the camera around the object and
 * for each frame generates an output 
 * - PNG file showing depth map
 * - PNG with nearest vertex index (1 - based) or 0 for none
 */

#include "glutils.hpp"
#include "Shader.hpp"
#include "model.hpp"

// Matrix libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// IO
#include <iostream>
#include <fstream>

void makeTransformMatrices( glm::mat4& world, glm::mat4& view, glm::mat4& project ) {
	// Set up transform
	world = glm::mat4(1.0f);
	view = glm::mat4(1.0f);
	view[3][2] = -5;

	project = glm::perspective(55.0f, 1.f, 0.1f, 10.f);
}

int main() {
    int windowWidth = 800;
    int windowHeight = 600;
	GLFWwindow* window = initGL(windowWidth, windowHeight);

	Model model{"/Users/dave/Library/Mobile Documents/com~apple~CloudDocs/PhD/Code/Animesh/data/Cube/cube.obj"};
	// Model model{"/Users/dave/Library/Mobile Documents/com~apple~CloudDocs/PhD/Code/Animesh/data/mini-horse/horse-04.obj"};
	Shader depthShader{"vertex_shader.glsl", "depth_frag_shader.glsl"};

		// Set up transform
	glm::mat4 worldTransform, viewTransform, projectionTransform;
	makeTransformMatrices(worldTransform, viewTransform, projectionTransform);


	// ---------------------------------------------------------
	while(!glfwWindowShouldClose(window)) {
		// Input
		handleInput(window);

		// Clear BG
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Display
	    glfwSwapBuffers(window);
	    glfwPollEvents();    
	}

	glfwTerminate();
    return 0;
}