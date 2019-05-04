package org.ddurbin.animesh.viewer;

import static java.lang.System.in;


import com.google.common.io.Files;
import com.jogamp.opengl.GL;
import com.jogamp.opengl.GL4;
import com.jogamp.opengl.GLAutoDrawable;
import com.jogamp.opengl.GLEventListener;
import com.jogamp.opengl.GLProfile;
import com.jogamp.opengl.GLCapabilities;
import com.jogamp.newt.opengl.GLWindow;

import com.jogamp.opengl.util.*;
import com.jogamp.common.nio.Buffers;

import com.jogamp.opengl.util.glsl.ShaderCode;
import com.jogamp.opengl.util.glsl.ShaderProgram;
import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URI;
import java.net.URL;
import java.nio.FloatBuffer;
import java.nio.charset.Charset;
import java.util.List;
import java.util.ResourceBundle;

/**
 * Compile, run and enjoy:
 * dapted from example by
 *
 * @author Xerxes Rånby (xranby)
 * wget http://jogamp.org/deployment/jogamp-current/archive/jogamp-all-platforms.7z
 */

public class JoglMain implements GLEventListener {

  static final String VERSION_STRING = "#version 330\n";

  /* Introducing the GL4 demo
 *
 * How to render a triangle using ~500 lines of code using the RAW
 * OpenGL ES 2 API.
 * The Programmable pipeline in OpenGL ES 2 are both fast and flexible
 * yet it do take some extra lines of code to setup.
 *
 */
  private double t0 = System.currentTimeMillis();
  private double theta;
  private double s;

  private static int width = 1920;
  private static int height = 1080;

  private int shaderProgram;
  private int vertShader;
  private int fragShader;
  private int ModelViewProjectionMatrix_location;

  static final int COLOR_IDX = 0;
  static final int VERTICES_IDX = 1;
  int[] vboHandles;

  /* Introducing projection matrix helper functions
   *
   * OpenGL ES 2 vertex projection transformations gets applied inside the
   * vertex shader, all you have to do are to calculate and supply a projection matrix.
   *
   * Its recomended to use the com/jogamp/opengl/util/PMVMatrix.java
   * import com.jogamp.opengl.util.PMVMatrix;
   * To simplify all your projection model view matrix creation needs.
   *
   * These helpers here are based on PMVMatrix code and common linear
   * algebra for matrix multiplication, translate and rotations.
   */
  private void glMultMatrixf(FloatBuffer a, FloatBuffer b, FloatBuffer d) {
    final int aP = a.position();
    final int bP = b.position();
    final int dP = d.position();
    for (int i = 0; i < 4; i++) {
      final float ai0 = a.get(aP + i + 0 * 4), ai1 = a.get(aP + i + 1 * 4), ai2 = a.get(aP + i + 2 * 4), ai3 = a.get(aP + i + 3 * 4);
      d.put(dP + i + 0 * 4, ai0 * b.get(bP + 0 + 0 * 4) + ai1 * b.get(bP + 1 + 0 * 4) + ai2 * b.get(bP + 2 + 0 * 4) + ai3 * b.get(bP + 3 + 0 * 4));
      d.put(dP + i + 1 * 4, ai0 * b.get(bP + 0 + 1 * 4) + ai1 * b.get(bP + 1 + 1 * 4) + ai2 * b.get(bP + 2 + 1 * 4) + ai3 * b.get(bP + 3 + 1 * 4));
      d.put(dP + i + 2 * 4, ai0 * b.get(bP + 0 + 2 * 4) + ai1 * b.get(bP + 1 + 2 * 4) + ai2 * b.get(bP + 2 + 2 * 4) + ai3 * b.get(bP + 3 + 2 * 4));
      d.put(dP + i + 3 * 4, ai0 * b.get(bP + 0 + 3 * 4) + ai1 * b.get(bP + 1 + 3 * 4) + ai2 * b.get(bP + 2 + 3 * 4) + ai3 * b.get(bP + 3 + 3 * 4));
    }
  }

  private float[] multiply(float[] a, float[] b) {
    float[] tmp = new float[16];
    glMultMatrixf(FloatBuffer.wrap(a), FloatBuffer.wrap(b), FloatBuffer.wrap(tmp));
    return tmp;
  }

  private float[] translate(float[] m, float x, float y, float z) {
    float[] t = {1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        x, y, z, 1.0f};
    return multiply(m, t);
  }

  private float[] rotate(float[] m, float a, float x, float y, float z) {
    float s, c;
    s = (float) Math.sin(Math.toRadians(a));
    c = (float) Math.cos(Math.toRadians(a));
    float[] r = {
        x * x * (1.0f - c) + c, y * x * (1.0f - c) + z * s, x * z * (1.0f - c) - y * s, 0.0f,
        x * y * (1.0f - c) - z * s, y * y * (1.0f - c) + c, y * z * (1.0f - c) + x * s, 0.0f,
        x * z * (1.0f - c) + y * s, y * z * (1.0f - c) - x * s, z * z * (1.0f - c) + c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};
    return multiply(m, r);
  }


  public static void main(String[] s) {

        /* This demo are based on the GL4 GLProfile that uses common hardware acceleration
         * functionality of desktop OpenGL 3, 2 and mobile OpenGL ES 2 devices.
         * JogAmp JOGL will probe all the installed libGL.so, libEGL.so and libGLESv2.so librarys on
         * the system to find which one provide hardware acceleration for your GPU device.
         * Its common to find more than one version of these librarys installed on a system.
         * For example on a ARM Linux system JOGL may find
         * Hardware accelerated Nvidia tegra GPU drivers in: /usr/lib/nvidia-tegra/libEGL.so
         * Software rendered Mesa Gallium driver in: /usr/lib/arm-linux-gnueabi/mesa-egl/libEGL.so.1
         * Software rendered Mesa X11 in: /usr/lib/arm-linux-gnueabi/mesa/libGL.so
         * Good news!: JOGL does all this probing for you all you have to do are to ask for
         * the GLProfile you want to use.
         */

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

  String loadResource(String fileName) throws Exception {
    URL u = this.getClass().getClassLoader().getResource(fileName);
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

  /* GLEvenetListener::init
   */
  public void init(GLAutoDrawable drawable) {
    GL4 gl = drawable.getGL().getGL4();

    System.err.println("Chosen GLCapabilities: " + drawable.getChosenGLCapabilities());
    System.err.println("INIT GL IS: " + gl.getClass().getName());
    System.err.println("GL_VENDOR: " + gl.glGetString(GL.GL_VENDOR));
    System.err.println("GL_RENDERER: " + gl.glGetString(GL.GL_RENDERER));
    System.err.println("GL_VERSION: " + gl.glGetString(GL.GL_VERSION));

        /* The initialization below will use the OpenGL ES 2 API directly
         * to setup the two shader programs that will be run on the GPU.
         *
         * Its recommended to use the jogamp/opengl/util/glsl/ classes
         * import com.jogamp.opengl.util.glsl.ShaderCode;
         * import com.jogamp.opengl.util.glsl.ShaderProgram;
         * import com.jogamp.opengl.util.glsl.ShaderState;
         * to simplify shader customization, compile and loading.
         *
         * You may also want to look at the JOGL RedSquareES2 demo
         * http://jogamp.org/git/?p=jogl.git;a=blob;f=src/test/com/jogamp/opengl/test/junit/jogl/demos/es2/RedSquareES2.java;hb=HEAD#l78
         * to see how the shader customization, compile and loading is done
         * using the recommended JogAmp GLSL utility classes.
         */

    // Make the shader strings compatible with OpenGL 3 core if needed
    // GL4 also includes the intersection of GL3 core
    // The default implicit GLSL version 1.1 is now depricated in GL3 core
    // GLSL 1.3 is the minimum version that now has to be explicitly set.
    // This allows the shaders to compile using the latest
    // desktop OpenGL 3 and 4 drivers.
    String vertexShaderString = "";
    String fragmentShaderString = "";
    try {
      vertexShaderString = loadResource("triangle.vert");
      fragmentShaderString = loadResource("triangle.frag");
      if (gl.isGL3core()) {
        System.out.println("GL3 core detected: explicit add #version to shaders");
        vertexShaderString = VERSION_STRING  + vertexShaderString;
        fragmentShaderString = VERSION_STRING + fragmentShaderString;
      }
    } catch (Exception e) {
      System.out.println("EPIC FAIL!");
      System.exit(1);
    }

    // Create GPU shader handles
    // OpenGL ES retuns a index id to be stored for future reference.
    vertShader = gl.glCreateShader(GL4.GL_VERTEX_SHADER);
    fragShader = gl.glCreateShader(GL4.GL_FRAGMENT_SHADER);

    //Compile the vertexShader String into a program.
    String[] vlines = new String[] {vertexShaderString};
    int[] vlengths = new int[] {vlines[0].length()};
    gl.glShaderSource(vertShader, vlines.length, vlines, vlengths, 0);
    gl.glCompileShader(vertShader);

    //Check compile status.
    int[] compiled = new int[1];
    gl.glGetShaderiv(vertShader, GL4.GL_COMPILE_STATUS, compiled, 0);
    if (compiled[0] != 0) {
      System.out.println("Horray! vertex shader compiled");
    } else {
      int[] logLength = new int[1];
      gl.glGetShaderiv(vertShader, GL4.GL_INFO_LOG_LENGTH, logLength, 0);

      byte[] log = new byte[logLength[0]];
      gl.glGetShaderInfoLog(vertShader, logLength[0], (int[]) null, 0, log, 0);

      System.err.println("Error compiling the vertex shader: " + new String(log));
      System.exit(1);
    }

    //Compile the fragmentShader String into a program.
    String[] flines = new String[] {fragmentShaderString};
    int[] flengths = new int[] {flines[0].length()};
    gl.glShaderSource(fragShader, flines.length, flines, flengths, 0);
    gl.glCompileShader(fragShader);

    //Check compile status.
    gl.glGetShaderiv(fragShader, GL4.GL_COMPILE_STATUS, compiled, 0);
    if (compiled[0] != 0) {
      System.out.println("Horray! fragment shader compiled");
    } else {
      int[] logLength = new int[1];
      gl.glGetShaderiv(fragShader, GL4.GL_INFO_LOG_LENGTH, logLength, 0);

      byte[] log = new byte[logLength[0]];
      gl.glGetShaderInfoLog(fragShader, logLength[0], (int[]) null, 0, log, 0);

      System.err.println("Error compiling the fragment shader: " + new String(log));
      System.exit(1);
    }

    //Each shaderProgram must have
    //one vertex shader and one fragment shader.
    shaderProgram = gl.glCreateProgram();
    gl.glAttachShader(shaderProgram, vertShader);
    gl.glAttachShader(shaderProgram, fragShader);

    //Associate attribute ids with the attribute names inside
    //the vertex shader.
    gl.glBindAttribLocation(shaderProgram, 0, "attribute_Position");
    gl.glBindAttribLocation(shaderProgram, 1, "attribute_Color");

    gl.glLinkProgram(shaderProgram);

    //Get a id number to the uniform_Projection matrix
    //so that we can update it.
    ModelViewProjectionMatrix_location = gl.glGetUniformLocation(shaderProgram, "uniform_Projection");

        /* GL4 also includes the intersection of GL3 core
         * GL3 core and later mandates that a "Vector Buffer Object" must
         * be created and bound before calls such as gl.glDrawArrays is used.
         * The VBO lines in this demo makes the code forward compatible with
         * OpenGL 3 and ES 3 core and later where a default
         * vector buffer object is deprecated.
         *
         * Generate two VBO pointers / handles
         * VBO is data buffers stored inside the graphics card memory.
         */
    vboHandles = new int[2];
    gl.glGenBuffers(2, vboHandles, 0);
  }

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

  public void display(GLAutoDrawable drawable) {
    // Update variables used in animation
    double t1 = System.currentTimeMillis();
    theta += (t1 - t0) * 0.005f;
    t0 = t1;
    s = Math.sin(theta);

    // Get gl
    GL4 gl = drawable.getGL().getGL4();

    // Clear screen
    gl.glClearColor(1, 0, 1, 0.5f);  // Purple
    gl.glClear(GL4.GL_STENCIL_BUFFER_BIT |
        GL4.GL_COLOR_BUFFER_BIT |
        GL4.GL_DEPTH_BUFFER_BIT);

    // Use the shaderProgram that got linked during the init part.
    gl.glUseProgram(shaderProgram);

        /* Change a projection matrix
         * The matrix multiplications and OpenGL ES2 code below
         * basically match this OpenGL ES1 code.
         * note that the model_view_projection matrix gets sent to the vertexShader.
         *
         * gl.glLoadIdentity();
         * gl.glTranslatef(0.0f,0.0f,-0.1f);
         * gl.glRotatef((float)30f*(float)s,1.0f,0.0f,1.0f);
         *
         */

    float[] model_view_projection;
    float[] identity_matrix = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    model_view_projection = translate(identity_matrix, 0.0f, 0.0f, -0.1f);
    model_view_projection = rotate(model_view_projection, 30f * (float) s, 1.0f, 0.0f, 1.0f);

    // Send the final projection matrix to the vertex shader by
    // using the uniform location id obtained during the init part.
    gl.glUniformMatrix4fv(ModelViewProjectionMatrix_location, 1, false, model_view_projection, 0);

        /*
         *  Render a triangle:
         *  The OpenGL ES2 code below basically match this OpenGL code.
         *
         *    gl.glBegin(GL_TRIANGLES);                      // Drawing Using Triangles
         *    gl.glVertex3f( 0.0f, 1.0f, 0.0f);              // Top
         *    gl.glVertex3f(-1.0f,-1.0f, 0.0f);              // Bottom Left
         *    gl.glVertex3f( 1.0f,-1.0f, 0.0f);              // Bottom Right
         *    gl.glEnd();                            // Finished Drawing The Triangle
         */

    float[] vertices = {0.0f, 1.0f, 0.0f, //Top
        -1.0f, -1.0f, 0.0f, //Bottom Left
        1.0f, -1.0f, 0.0f  //Bottom Right
    };


    // Observe that the vertex data passed to glVertexAttribPointer must stay valid
    // through the OpenGL rendering lifecycle.
    // Therefore it is mandatory to allocate a NIO Direct buffer that stays pinned in memory
    // and thus can not get moved by the java garbage collector.
    // Also we need to keep a reference to the NIO Direct buffer around up untill
    // we call glDisableVertexAttribArray first then will it be safe to garbage collect the memory.
    // I will here use the com.jogamp.common.nio.Buffers to quicly wrap the array in a Direct NIO buffer.
    FloatBuffer fbVertices = Buffers.newDirectFloatBuffer(vertices);

    // Select the VBO, GPU memory data, to use for vertices
    gl.glBindBuffer(GL4.GL_ARRAY_BUFFER, vboHandles[VERTICES_IDX]);

    // transfer data to VBO, this perform the copy of data from CPU -> GPU memory
    int numBytes = vertices.length * 4;
    gl.glBufferData(GL.GL_ARRAY_BUFFER, numBytes, fbVertices, GL.GL_STATIC_DRAW);
    fbVertices = null; // It is OK to release CPU vertices memory after transfer to GPU

    // Associate Vertex attribute 0 with the last bound VBO
    gl.glVertexAttribPointer(0 /* the vertex attribute */, 3,
        GL4.GL_FLOAT, false /* normalized? */, 0 /* stride */,
        0 /* The bound VBO data offset */);

    // VBO
    // gl.glBindBuffer(GL4.GL_ARRAY_BUFFER, 0); // You can unbind the VBO after it have been associated using glVertexAttribPointer

    gl.glEnableVertexAttribArray(0);


    float[] colors = {1.0f, 0.0f, 0.0f, 1.0f, //Top color (red)
        0.0f, 0.0f, 0.0f, 1.0f, //Bottom Left color (black)
        1.0f, 1.0f, 0.0f, 0.9f  //Bottom Right color (yellow) with 10% transparence
    };

    FloatBuffer fbColors = Buffers.newDirectFloatBuffer(colors);

    // Select the VBO, GPU memory data, to use for colors
    gl.glBindBuffer(GL4.GL_ARRAY_BUFFER, vboHandles[COLOR_IDX]);

    numBytes = colors.length * 4;
    gl.glBufferData(GL.GL_ARRAY_BUFFER, numBytes, fbColors, GL.GL_STATIC_DRAW);
    fbColors = null; // It is OK to release CPU color memory after transfer to GPU

    // Associate Vertex attribute 1 with the last bound VBO
    gl.glVertexAttribPointer(1 /* the vertex attribute */, 4 /* four possitions used for each vertex */,
        GL4.GL_FLOAT, false /* normalized? */, 0 /* stride */,
        0 /* The bound VBO data offset */);

    gl.glEnableVertexAttribArray(1);

    gl.glDrawArrays(GL4.GL_TRIANGLES, 0, 3); //Draw the vertices as triangle

    gl.glDisableVertexAttribArray(0); // Allow release of vertex position memory
    gl.glDisableVertexAttribArray(1); // Allow release of vertex color memory
  }

  public void dispose(GLAutoDrawable drawable) {
    System.out.println("cleanup, remember to release shaders");
    GL4 gl = drawable.getGL().getGL4();
    gl.glUseProgram(0);
    gl.glDeleteBuffers(2, vboHandles, 0); // Release VBO, color and vertices, buffer GPU memory.
    vboHandles = null;
    gl.glDetachShader(shaderProgram, vertShader);
    gl.glDeleteShader(vertShader);
    gl.glDetachShader(shaderProgram, fragShader);
    gl.glDeleteShader(fragShader);
    gl.glDeleteProgram(shaderProgram);
    System.exit(0);
  }
}