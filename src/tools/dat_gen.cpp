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


GLuint generateTexture( int tex_w, int tex_h) {
	// dimensions of the image
	GLuint tex_output;
	glGenTextures(1, &tex_output);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_output);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT,
	 NULL);
	glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	return tex_output;
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

	GLuint tex_output = generateTexture(512, 512);

	int work_grp_cnt[3];

	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

	std::cout << "max global (total) work group size x:" << work_grp_cnt[0] 
			<< " y:" << work_grp_cnt[1]
			<< " z:" << work_grp_cnt[2]
			<< std::endl;

	// ---------------------------------------------------------
	while(!glfwWindowShouldClose(window)) {
		// Input
		handleInput(window);

		// Clear BG
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render textur to full screen quad

		// Display
	    glfwSwapBuffers(window);
	    glfwPollEvents();    
	}

	glfwTerminate();
    return 0;
}