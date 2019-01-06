// Include standard headers
#include <iostream>
#include <fstream>
#include <cmath>
#include "Shader.hpp"

#include "glutils.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "model.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


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
		-1.0,  1.0
	};
	unsigned int indices[] = {0, 2, 1, 0, 3, 2};

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	// Bind to a EBO
	GLuint EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0); 

	return VAO;
}

/**
 * Extract data from FBO and save as PNG
 */
void saveImage(GLuint FBO, GLuint texture, unsigned int windowWidth, unsigned int windowHeight ) {
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "saveImage entry error: " << std::hex << "0x" << err << std::endl;
    }
	unsigned int *imageData = new unsigned int[windowWidth * windowHeight * 4];
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);  

	// Extract texture and save as PNG
	glBindTexture(GL_TEXTURE_2D, texture);
	glReadBuffer( GL_COLOR_ATTACHMENT0);

	glReadPixels(0, 0, windowWidth, windowHeight, GL_RED_INTEGER, GL_UNSIGNED_INT, imageData);

    std::ofstream rawData{"/Users/dave/Desktop/stbi.dat", std::ios::out | std::ios::binary};
    rawData.write( (const char *)imageData, windowWidth * windowHeight * sizeof(unsigned int));

	stbi_write_png("/Users/dave/Desktop/stbi.png", 
		windowWidth, windowHeight, 4, 
		imageData, windowWidth * sizeof(unsigned int));
	glBindFramebuffer(GL_FRAMEBUFFER, 0);  
	delete[] imageData;

	while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "saveImage exit error: " << std::hex << "0x" << err << std::endl;
    }
}

int main() {
    int windowWidth = 800;
    int windowHeight = 600;
	GLFWwindow* window = initGL(windowWidth, windowHeight);

	Model model{"/Users/dave/Library/Mobile Documents/com~apple~CloudDocs/PhD/Code/Animesh/data/Cube/cube.obj"};
	// Model model{"/Users/dave/Library/Mobile Documents/com~apple~CloudDocs/PhD/Code/Animesh/data/mini-horse/horse-04.obj"};
	Shader depthShader{"vertex_shader.glsl", "depth_frag_shader.glsl"};

	// Set up transform
	glm::mat4 worldTransform = glm::mat4(1.0f);
	glm::mat4 viewTransform = glm::mat4(1.0f);
	viewTransform[3][2] = -5;
	glm::mat4 projectionTransform = glm::perspective(55.0f, 1.f, 0.1f, 10.f);

	// Extract uniform addresses.
	GLuint modelMatrixLoc = glGetUniformLocation(depthShader.ID, "modelMatrix");
	GLuint viewMatrixLoc = glGetUniformLocation(depthShader.ID, "viewMatrix");
	GLuint projectionMatrixLoc = glGetUniformLocation(depthShader.ID, "projectionMatrix");

	// ---------------------------------------------------------
	// Off screen Rendering Setup
	// Store scene data to texture
	GLuint sceneTexture = model.writeToTexture();
	assert( sceneTexture != 0 );

	// Set up VAO for screen data
	GLuint rtVao = loadScreenVec2(windowWidth, windowHeight);
	Shader rtVertexShader{"rt_vertex_shader.glsl", "rt_fragment_shader.glsl"};

	// Make FBO for rendering indexes to
	GLuint FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);  
	// Make an empty texture
	GLuint outTexture;
	glGenTextures(1, &outTexture);
	glBindTexture(GL_TEXTURE_2D, outTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 800, 600, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  
	// Attach the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outTexture, 0);  
	// Set the list of draw buffers.
	GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, drawBuffers); // "1" is the size of DrawBuffers
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);  
	// ---------------------------------------------------------

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


		// ------------------------------------
		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR) {
	        std::cerr << "Before FBO code error: " << std::hex << "0x" << err << std::endl;
	    }
		//	Switch to offscreen rendering to my FBO
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glViewport(0, 0, windowWidth, windowHeight);

		rtVertexShader.use();
		rtVertexShader.setFloat( "focalLength", 1.);
		rtVertexShader.setFloat( "aspect", windowHeight / windowWidth);
		GLuint projectionMatrixRTLoc = glGetUniformLocation(rtVertexShader.ID, "projectionMatrix");
		glUniformMatrix4fv(projectionMatrixRTLoc, 1, GL_FALSE, glm::value_ptr(projectionTransform));

		// Bind texture
		glBindVertexArray(rtVao);
		glBindTexture(GL_TEXTURE_2D, sceneTexture);

		glDrawElements(GL_TRIANGLES, 2, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0); 
		while ((err = glGetError()) != GL_NO_ERROR) {
	        std::cerr << "After FBO code error: " << std::hex << "0x" << err << std::endl;
	    }

		// ---------------------------------------

		// Display
	    glfwSwapBuffers(window);
	    glfwPollEvents();    
	}
	saveImage(FBO, outTexture, windowWidth, windowHeight);

	glDeleteFramebuffers(1, &FBO);  
	glfwTerminate();
    return 0;
}