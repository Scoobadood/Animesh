package org.ddurbin.animesh.viewer;

import static org.ddurbin.animesh.viewer.MatrixHelper.rotate;
import static org.ddurbin.animesh.viewer.MatrixHelper.translate;


import com.google.common.base.Strings;
import com.jogamp.common.nio.Buffers;
import com.jogamp.newt.opengl.GLWindow;
import com.jogamp.opengl.GL;
import com.jogamp.opengl.GL4;
import com.jogamp.opengl.GLAutoDrawable;
import com.jogamp.opengl.GLCapabilities;
import com.jogamp.opengl.GLEventListener;
import com.jogamp.opengl.GLProfile;
import com.jogamp.opengl.util.Animator;
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;
import java.nio.FloatBuffer;
import org.ddurbin.animesh.bin.State;
import org.ddurbin.animesh.bin.StateUtilities;


/**
 * Adapted from example.
 *
 * @author Xerxes Rånby (xranby)
 * {@see http://jogamp.org/deployment/jogamp-current/archive/jogamp-all-platforms.7z}
 */
public class JoglMain implements GLEventListener {

  private static final String VERSION_STRING = "#version 330\n";
  /*
   * Scene Data
   */
  private static final int COLOR_IDX = 0;
  private static final int VERTICES_IDX = 1;
  /*
   * Window related
   */
  private static int width = 1920;
  private static int height = 1080;

  // World
  private static float[] colours;
  private static float[] vertices;

  /*
   *  Variables for managing the view
   */
  private double theta = 0.0; // Y axis rotation
  private double phi   = 0.0; // X axis rotation
  private double psi   = 0.0; // Z axis rotation
  private static final double DELTA_ROT = Math.PI / 100.0;

  /*
   * Shader related variables/handles
   */
  private MyShaderProgram shadProg;
  // Where we put our handles for vertex buffers
  private int[] vboHandles;


  private static void createWorld(State state) {
    vertices = StateToGlData.convertStateToGlData(state, 3);
    int numItems = (vertices.length / (4 * 3));
    colours = new float[numItems * (4 * 4)];

    // We have 8 points making 4 lines
    for (int i = 0; i < (numItems); i++) {
      // normal : Red
      colours[i * 16 + 0] = 1.0f;
      colours[i * 16 + 1] = 0.0f;
      colours[i * 16 + 2] = 0.0f;
      colours[i * 16 + 3] = 1.0f;
      colours[i * 16 + 4] = 1.0f;
      colours[i * 16 + 5] = 0.0f;
      colours[i * 16 + 6] = 0.0f;
      colours[i * 16 + 7] = 1.0f;

      // Primary tangent - aqua
      colours[i * 16 + 8] = 0.0f;
      colours[i * 16 + 9] = 1.0f;
      colours[i * 16 + 10] = 1.0f;
      colours[i * 16 + 11] = 1.0f;
      colours[i * 16 + 12] = 0.0f;
      colours[i * 16 + 13] = 1.0f;
      colours[i * 16 + 14] = 1.0f;
      colours[i * 16 + 15] = 1.0f;

      // Opposite to primary : blue
      colours[i * 16 + 8] = 0.0f;
      colours[i * 16 + 9] = 0.0f;
      colours[i * 16 + 10] = 1.0f;
      colours[i * 16 + 11] = 1.0f;
      colours[i * 16 + 12] = 0.0f;
      colours[i * 16 + 13] = 0.0f;
      colours[i * 16 + 14] = 1.0f;
      colours[i * 16 + 15] = 1.0f;
      // 90 degrees : blue
      colours[i * 16 + 8] = 0.0f;
      colours[i * 16 + 9] = 0.0f;
      colours[i * 16 + 10] = 1.0f;
      colours[i * 16 + 11] = 1.0f;
      colours[i * 16 + 12] = 0.0f;
      colours[i * 16 + 13] = 0.0f;
      colours[i * 16 + 14] = 1.0f;
      colours[i * 16 + 15] = 1.0f;

    }
  }

  /**
   * Run it.
   */
  public static void main(String[] args) throws Exception {
    State state = StateUtilities.loadState(args[0], args[1]);
    createWorld(state);


    GLCapabilities caps = new GLCapabilities(GLProfile.get(GLProfile.GL4ES3));

    // We may at this point tweak the caps and request a translucent drawable
    caps.setBackgroundOpaque(false);
    GLWindow glWindow = GLWindow.create(caps);

    // In this demo we prefer to setup and view the GLWindow directly
    // this allows the demo to run on -Djava.awt.headless=true systems
    glWindow.setTitle("Raw GL4 Demo");
    glWindow.setSize(width, height);
    glWindow.setUndecorated(false);
    glWindow.setPointerVisible(true);
    glWindow.setVisible(true);

    // Finally we connect the GLEventListener application code to the NEWT GLWindow.
    // GLWindow will call the GLEventListener init, reshape, display and dispose
    // functions when needed.
    glWindow.addGLEventListener(new JoglMain() /* GLEventListener */);
    Animator animator = new Animator();
    animator.add(glWindow);
    animator.start();
  }

  private String loadResource(String fileName, GL gl) throws Exception {
    URL u = this.getClass().getClassLoader().getResource(fileName);
    if (u == null) {
      throw new Exception("Resource not found");
    }
    InputStream str = u.openStream();
    BufferedReader reader = new BufferedReader(new InputStreamReader(str));
    StringBuilder sb = new StringBuilder();

    if (gl.isGL3core()) {
      System.out.println("GL3 core detected: explicit add #version to shaders");
      sb.append(VERSION_STRING);
    }

    String line;
    while ((line = reader.readLine()) != null) {
      sb.append(line);
      sb.append("\n");
    }
    return sb.toString();
  }

  private void checkCompileSucceeded(GL4 gl, int shader, String shaderName) throws Exception {
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

  private String nameForType(int shaderType) throws Exception {
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

  private int makeShaderOrFail(GL4 gl, String shaderFileName, int shaderType) throws Exception {
    String shaderString = loadResource(shaderFileName, gl);
    if (Strings.isNullOrEmpty(shaderString)) {
      throw new Exception("Missing or empty " + nameForType(shaderType));
    }
    int shader = gl.glCreateShader(shaderType);
    if (shader == 0) {
      throw new Exception("Error creating " + nameForType(shaderType));
    }

    //Compile the vertexShader String into a program.
    String[] vlines = new String[] {
        shaderString
    };
    int[] vlengths = new int[] {vlines[0].length()};
    gl.glShaderSource(shader, vlines.length, vlines, vlengths, 0);
    gl.glCompileShader(shader);

    checkCompileSucceeded(gl, shader, nameForType(shaderType));
    return shader;
  }

  private MyShaderProgram setupShaderProgram(GL4 gl) throws Exception {
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

  private void reportCapabilities(GLAutoDrawable drawable, GL gl) {
    System.err.println("Chosen GLCapabilities: " + drawable.getChosenGLCapabilities());
    System.err.println("INIT GL IS: " + gl.getClass().getName());
    System.err.println("GL_VENDOR: " + gl.glGetString(GL.GL_VENDOR));
    System.err.println("GL_RENDERER: " + gl.glGetString(GL.GL_RENDERER));
    System.err.println("GL_VERSION: " + gl.glGetString(GL.GL_VERSION));
  }

  /**
   * GLEventListener::init.
   */
  public void init(GLAutoDrawable drawable) {
    GL4 gl = drawable.getGL().getGL4();
    try {
      reportCapabilities(drawable, gl);
      shadProg = setupShaderProgram(gl);
      vboHandles = new int[2];
      gl.glGenBuffers(2, vboHandles, 0);
    } catch (Exception e) {
      e.printStackTrace();
      System.exit(1);
    }
  }

  /**
   * GLEventListener::reshape.
   */
  public void reshape(GLAutoDrawable drawable, int x, int y, int z, int h) {
    System.out.println("Window resized to width=" + z + " height=" + h);
    width = z;
    height = h;

    // Get gl
    GL4 gl = drawable.getGL().getGL4();

    // Optional: Set viewport
    // Render to a square at the center of the window.
    gl.glViewport((width - height) / 2, 0, height, height);
  }

  private void updateAnimation() {
  }

  private void setTransform(GL4 gl) {
    float[] identity = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    float[] mvp = translate(identity, 0.0f, 0.0f, -0.8f);
    mvp = rotate(mvp, (float) theta, 0.0f, 1.0f, 0.0f);
    mvp = rotate(mvp, (float) phi,   1.0f, 0.0f, 0.0f);
    mvp = rotate(mvp, (float) psi, 0.0f, 0.0f, 1.0f);

    // Send the final projection matrix to the vertex shader by
    // using the uniform location id obtained during the init part.
    gl.glUniformMatrix4fv(shadProg.uniformMvpMatrix, 1, false, mvp, 0);
  }

  private void setVbo(GL4 gl, float[] data, int dataSize, int vboIndex, int vaaIndex) {
    FloatBuffer fbData = Buffers.newDirectFloatBuffer(data);
    gl.glBindBuffer(GL4.GL_ARRAY_BUFFER, vboHandles[vboIndex]);
    int numBytes = data.length * 4;
    gl.glBufferData(GL.GL_ARRAY_BUFFER, numBytes, fbData, GL.GL_STATIC_DRAW);
    fbData = null; // It is OK to release CPU vertices memory after transfer to GPU

    // Associate Vertex attribute 0 with the last bound VBO
    gl.glVertexAttribPointer(vaaIndex /* the vertex attribute */,
        dataSize,
        GL4.GL_FLOAT,
        false /* normalized? */,
        0 /* stride */,
        0 /* The bound VBO data offset */);
    gl.glEnableVertexAttribArray(vaaIndex);
  }

  /**
   * GLEventListener::display.
   */
  public void display(GLAutoDrawable drawable) {
    updateAnimation();

    // Get gl
    GL4 gl = drawable.getGL().getGL4();

    // Clear screen
    gl.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    gl.glClear(GL4.GL_STENCIL_BUFFER_BIT | GL4.GL_COLOR_BUFFER_BIT | GL4.GL_DEPTH_BUFFER_BIT);

    // Use the shaderProgram that got linked during the init part.
    gl.glUseProgram(shadProg.shaderProgramId);

    // Set the MVP matrix
    setTransform(gl);

    for (int i = 0; i < vertices.length; i++) {
      vertices[i] = vertices[i];
    }
    setVbo(gl, vertices, 3, VERTICES_IDX, 0);
    setVbo(gl, colours, 4, COLOR_IDX, 1);

    gl.glDrawArrays(GL4.GL_LINES, 0, vertices.length); //Draw the vertices as triangle
    gl.glDisableVertexAttribArray(0); // Allow release of vertex position memory
    gl.glDisableVertexAttribArray(1); // Allow release of vertex color memory
  }

  /**
   * GLEventListener::dispose.
   */
  public void dispose(GLAutoDrawable drawable) {
    System.out.println("cleanup, remember to release shaders");
    GL4 gl = drawable.getGL().getGL4();
    gl.glUseProgram(0);
    gl.glDeleteBuffers(2, vboHandles, 0); // Release VBO, color and vertices, buffer GPU memory.
    vboHandles = null;
    gl.glDetachShader(shadProg.shaderProgramId, shadProg.vertexShaderId);
    gl.glDeleteShader(shadProg.vertexShaderId);
    gl.glDetachShader(shadProg.shaderProgramId, shadProg.fragmentShaderId);
    gl.glDeleteShader(shadProg.fragmentShaderId);
    gl.glDeleteProgram(shadProg.shaderProgramId);
    System.exit(0);
  }

  private static class MyShaderProgram {
    final int shaderProgramId;
    final int vertexShaderId;
    final int fragmentShaderId;
    final int uniformMvpMatrix;

    MyShaderProgram(int shaderProgramId, //
                    int vertexShaderId, //
                    int fragmentShaderId, //
                    int uniformMvpMatrix) {
      this.shaderProgramId = shaderProgramId;
      this.fragmentShaderId = fragmentShaderId;
      this.vertexShaderId = vertexShaderId;
      this.uniformMvpMatrix = uniformMvpMatrix;
    }
  }
}