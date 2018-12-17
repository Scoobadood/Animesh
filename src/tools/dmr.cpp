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


	Model model{"/Users/dave/Library/Mobile Documents/com~apple~CloudDocs/PhD/Code/Animesh/data/mini-horse/horse-04.obj"};
	Shader s2{"vertex_shader.glsl", "u_col_frag_shader.glsl"};

	// Set up transform
	// glm::mat4 trans = glm::mat4(1.0f);
	// trans = glm::rotate(trans, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
	// trans = glm::identity();
	// GLuint transformLoc = glGetUniformLocation(s2.ID, "transform");
	// glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
	float identity[] = {
		1.0f, 0.0f, 0.0f, 0.0f, 
		0.0f, 1.0f, 0.0f, 0.0f, 
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f 
	};
	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::rotate(trans, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
	
	while(!glfwWindowShouldClose(window)) {
		// Input
		handleInput(window);

		// Render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 3. Update colour of uniform
		float timeValue = glfwGetTime();
		float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
		s2.use();
		s2.setFloat( "greenShade", greenValue);
		GLuint transformLoc = glGetUniformLocation(s2.ID, "transform");
		trans = glm::rotate(trans, glm::radians(.01f), glm::vec3(0.0, 1.0, 0.0));
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
		model.draw(s2);

		// Display
	    glfwSwapBuffers(window);
	    glfwPollEvents();    
	}

	glfwTerminate();
    return 0;
}