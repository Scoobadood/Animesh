#define GL_SILENCE_DEPRECATION


// Include standard headers
#include <iostream>
#include <cmath>

// Apple OpenGL includes
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#include <OpenGL/gl3.h>
#else /// your stuff for linux
// Include GLEW. Always include it before gl.h and glfw3.h, since it's a bit magic.
#include <GL/glew.h>

// Include other non Apple libs here.
#endif

// Include GLFW
#include <GLFW/glfw3.h>

// Vertex shader
const char * vertexShaderSrc = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"out vec3 vertexColour;\n"
"void main() { \n"
"  	gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"   vertexColour = vec3( 0.5, 0.0, 0.0);\n"
"}";

// Fragment shader2
const char * redFragmentShaderSrc = "#version 330 core\n"
"in vec3 vertexColour;\n"
"out vec4 FragColor;\n"
"void main() {"
"  FragColor = vec4(vertexColour, 1.0f);\n"
"}";
const char * uniformFragmentShaderSrc = "#version 330 core\n"
"out vec4 FragColor;\n"
"uniform vec4 ourColour;\n"
"void main() {"
"  FragColor = ourColour;\n"
"}";

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}  

void handleInput(GLFWwindow *window) {
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

unsigned int createVertexShader( ) {
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSrc, NULL);
	glCompileShader(vertexShader);

	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if(!success) {
    	glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
	    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	return vertexShader;
}

GLuint createFragmentShader( const char * src ) {
	GLuint fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &src, NULL);
	glCompileShader(fragmentShader);

	int  success;
	char infoLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if(!success) {
    	glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
	    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	return fragmentShader;
}

GLuint makeShaderProgram(unsigned int vertexShader, unsigned int fragmentShader ) {
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	int  success;
	char infoLog[512];
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if(!success) {
	    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
	    std::cout << "ERROR::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	return shaderProgram;
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

	// OSX Hack - move window by one pixel to force renderer to work.


	// Make shaders
	GLuint vertexShader = createVertexShader();
	GLuint redFragmentShader = createFragmentShader(redFragmentShaderSrc);
	GLuint uniformFragmentShader = createFragmentShader(uniformFragmentShaderSrc);
	GLuint redShaderProgram = makeShaderProgram(vertexShader, redFragmentShader);
	GLuint uniformShaderProgram = makeShaderProgram(vertexShader, uniformFragmentShader);
	glDeleteShader(vertexShader);
	glDeleteShader(redFragmentShader);
	glDeleteShader(uniformFragmentShader);

	float vertices[] = {
	    -0.5f, -0.5f, 0.0f,
	     0.5f, -0.5f, 0.0f,
	     0.5f,  0.5f, 0.0f,
	    -0.5f, -0.5f, 0.0f,
	     0.5f,  0.5f, 0.0f,
	    -0.5f,  0.5f, 0.0f
	}; 

	// Make VBOs
	GLuint VBO[2];
	glGenBuffers(2, VBO);  

	GLuint VAO[2];
	glGenVertexArrays(2, VAO);

	// 1. bind Vertex Array Object
	glBindVertexArray(VAO[0]);
	// 2. copy our vertices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), vertices, GL_STATIC_DRAW);
	// 3. then set our vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);  

	glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), &(vertices[9]), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);  



	while(!glfwWindowShouldClose(window)) {
		// Input
		handleInput(window);

		// Render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// 2. use red shader program
		glUseProgram(redShaderProgram);
		glBindVertexArray(VAO[0]);
		glDrawArrays(GL_TRIANGLES, 0, 3);


		// 3. Update colour of uniform
		float timeValue = glfwGetTime();
		float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
		int vertexColourLocation = glGetUniformLocation(uniformShaderProgram, "ourColour");
		glUseProgram(uniformShaderProgram);
		glUniform4f(vertexColourLocation, 0.0f, greenValue, 0.0f, 1.0f);
		glBindVertexArray(VAO[1]);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// Display
	    glfwSwapBuffers(window);
	    glfwPollEvents();    
	}
    glDeleteVertexArrays(2, VAO);
    glDeleteBuffers(2, VBO);
    glDeleteProgram(redShaderProgram);
    glDeleteProgram(uniformShaderProgram);

	glfwTerminate();
    return 0;
}