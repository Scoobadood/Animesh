package org.ddurbin.animesh.viewer;

import com.google.common.base.Strings;
import com.jogamp.opengl.GL;
import com.jogamp.opengl.GL4;

import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;

//
// Shader support
//
class MyShaderProgram {
    final int shaderProgramId;
    final int vertexShaderId;
    final int fragmentShaderId;
    final int uniformMvpMatrix;

    private MyShaderProgram(int shaderProgramId, //
                    int vertexShaderId, //
                    int fragmentShaderId, //
                    int uniformMvpMatrix) {
        this.shaderProgramId = shaderProgramId;
        this.fragmentShaderId = fragmentShaderId;
        this.vertexShaderId = vertexShaderId;
        this.uniformMvpMatrix = uniformMvpMatrix;
    }

    static MyShaderProgram setupShaderProgram(GL4 gl) throws Exception {
        int vertexShader = makeShaderOrFail(gl, "triangle.vert", GL4.GL_VERTEX_SHADER);
        int fragmentShader = makeShaderOrFail(gl, "triangle.frag", GL4.GL_FRAGMENT_SHADER);

        //Each shaderProgram must have
        //one vertex shader and one fragment shader.
        int shaderProgram = gl.glCreateProgram();
        gl.glAttachShader(shaderProgram, vertexShader);
        gl.glAttachShader(shaderProgram, fragmentShader);

        //Associate attribute ids with the attribute names inside
        //the vertex shader.
        gl.glBindAttribLocation(shaderProgram, 0, "attribute_Position");
        gl.glBindAttribLocation(shaderProgram, 1, "attribute_Color");

        gl.glLinkProgram(shaderProgram);

        //Get a id number to the uniform_Projection matrix
        //so that we can update it.
        int uniProjection = gl.glGetUniformLocation(shaderProgram, "uniform_Projection");
        return new MyShaderProgram(shaderProgram, vertexShader, fragmentShader, uniProjection);
    }

    private static int makeShaderOrFail(GL4 gl, String shaderFileName, int shaderType) throws Exception {
        String shaderString = loadResource(shaderFileName, gl);
        if (Strings.isNullOrEmpty(shaderString)) {
            throw new Exception("Missing or empty " + nameForType(shaderType));
        }
        int shader = gl.glCreateShader(shaderType);
        if (shader == 0) {
            throw new Exception("Error creating " + nameForType(shaderType));
        }

        //Compile the vertexShader String into a program.
        String[] vlines = new String[]{
                shaderString
        };
        int[] vlengths = new int[]{vlines[0].length()};
        gl.glShaderSource(shader, vlines.length, vlines, vlengths, 0);
        gl.glCompileShader(shader);

        checkCompileSucceeded(gl, shader, nameForType(shaderType));
        return shader;
    }

    private static String loadResource(String fileName, GL gl) throws Exception {
        URL u = MyShaderProgram.class.getClassLoader().getResource(fileName);
        if (u == null) {
            throw new Exception("Resource not found");
        }
        InputStream str = u.openStream();
        BufferedReader reader = new BufferedReader(new InputStreamReader(str));
        StringBuilder sb = new StringBuilder();

        String line;
        while ((line = reader.readLine()) != null) {
            sb.append(line);
            sb.append("\n");
        }
        return sb.toString();
    }

    private static void checkCompileSucceeded(GL4 gl, int shader, String shaderName) throws Exception {
        //Check compile status.
        int[] compiled = new int[1];
        gl.glGetShaderiv(shader, GL4.GL_COMPILE_STATUS, compiled, 0);
        if (compiled[0] != 0) {
            System.out.println("Hooray! " + shaderName + " compiled");
        } else {
            int[] logLength = new int[1];
            gl.glGetShaderiv(shader, GL4.GL_INFO_LOG_LENGTH, logLength, 0);

            byte[] log = new byte[logLength[0]];
            gl.glGetShaderInfoLog(shader, logLength[0], null, 0, log, 0);

            System.err.println("Error compiling the " + shaderName + ": " + new String(log));
            throw new Exception("Error compiling the shader : " + shaderName);
        }
    }

    private static String nameForType(int shaderType) throws Exception {
        switch (shaderType) {
            case GL4.GL_VERTEX_SHADER:
                return "vertex shader";

            case GL4.GL_FRAGMENT_SHADER:
                return "fragment shader";

            case GL4.GL_GEOMETRY_SHADER:
                return "geometry shader";

            default:
                throw new Exception(String.format("Unknown shader type %d", shaderType));
        }
    }


}
