// Include standard headers
#include <iostream>
#include <cmath>
#include "Shader.hpp"


// Include GLFW
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "model.hpp"

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}  

void handleInput(GLFWwindow *window) {
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}


int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    int windowWidth = 800;
    int windowHeight = 600;

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "DMR", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	// Set a callback to adjust viewport when we resize window
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); 
	glViewport(0, 0, windowWidth, windowHeight);
	glfwMakeContextCurrent(window);

	glEnable(GL_DEPTH_TEST);

	Model model{"/Users/dave/Library/Mobile Documents/com~apple~CloudDocs/PhD/Code/Animesh/data/Cube/cube.obj"};
	// Model model{"/Users/dave/Library/Mobile Documents/com~apple~CloudDocs/PhD/Code/Animesh/data/mini-horse/horse-04.obj"};
	Shader depthShader{"vertex_shader.glsl", "depth_frag_shader.glsl"};

	// Set up transform
	glm::mat4 worldTransform = glm::mat4(1.0f);
	glm::mat4 viewTransform = glm::mat4(1.0f);
	viewTransform[3][2] = -4.3;
	glm::mat4 projectionTransform = glm::perspective(55.0f, 1.f, 0.1f, 10.f);

	// Extract uniform addresses.
	GLuint modelMatrixLoc = glGetUniformLocation(depthShader.ID, "modelMatrix");
	GLuint viewMatrixLoc = glGetUniformLocation(depthShader.ID, "viewMatrix");
	GLuint projectionMatrixLoc = glGetUniformLocation(depthShader.ID, "projectionMatrix");

	while(!glfwWindowShouldClose(window)) {
		// Input
		handleInput(window);

		// Clear BG
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Update the scene
		float timeValue = glfwGetTime();
		worldTransform = glm::rotate(worldTransform, glm::radians(.1f), glm::vec3(0.0, 1.0, 0.0));

		// Push to shader program		
		depthShader.use();
		glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, glm::value_ptr(worldTransform));
		glUniformMatrix4fv(viewMatrixLoc, 1, GL_FALSE, glm::value_ptr(viewTransform));
		glUniformMatrix4fv(projectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(projectionTransform));
		model.draw(depthShader);

		// Display
	    glfwSwapBuffers(window);
	    glfwPollEvents();    
	}

	glfwTerminate();
    return 0;
}