#pragma once

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#include <OpenGL/gl3.h>
#else /// your stuff for linux
// Include GLEW. Always include it before gl.h and glfw3.h, since it's a bit magic.
#include <GL/glew.h>

// Include other non Apple libs here.
#endif

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
private:
    GLuint createShader( const char * shaderSrc, GLenum shaderType ) {
        GLuint shader;
        shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &shaderSrc, NULL);
        glCompileShader(shader);

        int  success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::";
            std::cout << ((shaderType == GL_FRAGMENT_SHADER) 
                ? "FRAGMENT" 
                : (shaderType == GL_GEOMETRY_SHADER)
                    ? "GEOMETRY"
                    : (shaderType == GL_VERTEX_SHADER)
                        ? "VERTEX"
                        : "COMPUTE")
                << infoLog << std::endl;
        }
        return shader;
    }

    GLuint makeShaderProgram(GLuint vertexShader, GLuint geometryShader, GLuint fragmentShader ) {
        GLuint shaderProgram;
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        if( geometryShader != 0 ) {
            glAttachShader(shaderProgram, geometryShader);
        }
        glLinkProgram(shaderProgram);

        int  success;
        char infoLog[512];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::PROGRAM::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        return shaderProgram;
    }

public:
    // the program ID
    GLuint ID;

    /**
     * Construct with vertex and fragment shaders
     */
    Shader(const char* vertexPath, const char* fragmentPath) {
        makeShader(vertexPath, nullptr, fragmentPath);
    }
    /**
     * Construct with vertex geometry and fragment shaders
     */
    Shader(const char* vertexPath, const char* geometryPath, const char* fragmentPath) {
        makeShader(vertexPath, geometryPath, fragmentPath);
    }

    std::string loadCode( std::string shaderPath ) {
        std::string code;
        std::ifstream file;
        file.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try {
            file.open(shaderPath);
            std::stringstream stream;
            stream << file.rdbuf();
            file.close();
            code   = stream.str();
        }
        catch (std::ifstream::failure e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
        return code;
    }

    void makeShader(const char* vertexPath, const char* geometryPath, const char* fragmentPath) {
        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode = loadCode(vertexPath);
        const char* vShaderCode = vertexCode.c_str();
        GLuint vShader = createShader(vShaderCode, GL_VERTEX_SHADER);

        std::string fragmentCode = loadCode(fragmentPath);
        const char* fShaderCode = fragmentCode.c_str();
        GLuint fShader = createShader(fShaderCode, GL_FRAGMENT_SHADER);

        GLuint gShader = 0;
        if( geometryPath != nullptr) {
            std::string geometryCode = loadCode(geometryPath);
            const char* gShaderCode = geometryCode.c_str();
            gShader = createShader(gShaderCode, GL_GEOMETRY_SHADER);
        }
        ID = makeShaderProgram( vShader, gShader, fShader );
        if( gShader != 0 ) {
            glDeleteShader(gShader);
        }

        glDeleteShader(vShader);
        glDeleteShader(fShader);
    }

    void use() { 
        glUseProgram(ID);
    }  

    void setBool(const std::string &name, bool value) const {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }

    void setInt(const std::string &name, int value) const { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    void setFloat(const std::string &name, float value) const { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }
};