// Include standard headers
#include <iostream>
#include <fstream>
#include <cmath>
#include "Shader.hpp"

// Include GLFW
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "model.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void handleInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

GLFWwindow*  initGL(int windowWidth, int windowHeight) {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "DMR", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	// Set a callback to adjust viewport when we resize window
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glViewport(0, 0, windowWidth, windowHeight);
	glfwMakeContextCurrent(window);
	return window;
}

/*
 * Set up the 2D vector input to the rt renderer.
 * Each corner of the screen is supplied
 */
GLuint loadScreenVec2(unsigned int width, unsigned int height) {
	using namespace std;

	// Corners of screen
	float vertices[] = {
		-1.0, -1.0,
		 1.0, -1.0,
		 1.0,  1.0,
		-1.0, -1.0,
		 1.0,  1.0,
		-1.0,  1.0
	};

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	return VAO;
}

int main() {
	int windowWidth = 800;
	int windowHeight = 600;
	GLFWwindow * window = initGL(windowWidth, windowHeight);

	GLenum err;

	Model model{"/Users/dave/Library/Mobile Documents/com~apple~CloudDocs/PhD/Code/Animesh/data/Cube/cube.obj"};

	// Write scene to texture
	GLuint sceneTexture = model.writeToTexture();

	//
	// --------------------------------------------------------------------------------
	// Render the raytrace to FBO
	// --------------------------------------------------------------------------------

	// Make FBO for rendering to
	std::cout << "Making FBO" << std::endl;
	GLuint FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// Make an empty texture to render to
	GLuint outTexture;
	glGenTextures(1, &outTexture);
	glBindTexture(GL_TEXTURE_2D, outTexture);

	// Set structure of texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, windowWidth, windowHeight, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Attach the texture to the FBO for drawing
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outTexture, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	glViewport(0, 0, windowWidth, windowHeight);

	// Set up VAO for screen data
	std::cout << "Setting up VAO" << std::endl;
	GLuint rtVao = loadScreenVec2(windowWidth, windowHeight);

	// Set up shader program
	std::cout << "Compiling shader" << std::endl;
	Shader rtVertexShader{"rt_vertex_shader.glsl", "rt_fragment_shader.glsl"};
	rtVertexShader.use();
	rtVertexShader.setFloat( "focalLength", 1.);
	rtVertexShader.setFloat( "aspect", 1.0f);
	GLuint projectionMatrixRTLoc = glGetUniformLocation(rtVertexShader.ID, "projectionMatrix");
	glm::mat4 projectionTransform = glm::perspective(55.0f, 1.f, 0.1f, 5.f);
	glUniformMatrix4fv(projectionMatrixRTLoc, 1, GL_FALSE, glm::value_ptr(projectionTransform));


	std::cout << "Rendering to FBO" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glViewport(0, 0, windowWidth, windowHeight);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(rtVao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "After FBO code error: " << std::hex << "0x" << err << std::endl;
	}

	//
	// --------------------------------------------------------------------------------
	// Render the scene Texture back to image data and save 
	// --------------------------------------------------------------------------------
	float *imageData = new float[windowWidth * windowHeight * 4];
	glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	// glBindTexture(GL_TEXTURE_2D, outTexture);
	// glGetTexImage(GL_TEXTURE_2D, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, imageData);
	glReadPixels(0, 0, windowWidth, windowHeight, GL_RGBA, GL_FLOAT, imageData);
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "readPixels exit error: " << std::hex << "0x" << err << std::endl;
	}

	std::ofstream rawData{"/Users/dave/Desktop/stbi.dat", std::ios::out | std::ios::binary};
	rawData.write( (const char *)imageData, windowWidth * windowHeight * 4 * sizeof(float));

	// stbi_write_png("/Users/dave/Desktop/stbi.png",
	//                windowWidth, windowHeight, 4,
	//                imageData, windowWidth * sizeof(unsigned int));
	delete[] imageData;

	// glDeleteFramebuffers(1, &FBO);
	return 0;
}