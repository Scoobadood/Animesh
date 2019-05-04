package org.ddurbin.animesh.viewer;

import static com.jogamp.opengl.GL.GL_COLOR_BUFFER_BIT;
import static com.jogamp.opengl.GL.GL_DEPTH_BUFFER_BIT;
import static com.jogamp.opengl.GL.GL_TRIANGLES;


import com.jogamp.opengl.GL2;
import com.jogamp.opengl.GLAutoDrawable;
import com.jogamp.opengl.GLEventListener;
import com.jogamp.opengl.awt.GLCanvas;
import com.jogamp.opengl.glu.GLU;

public class JoglTriangle extends GLCanvas implements GLEventListener {
  private GLU glu;

  private float angle = 0.0f;

  public JoglTriangle() {
    this.addGLEventListener(this);
  }

  /**
   * Called back immediately after the OpenGL context is initialized. Can be used
   * to perform one-time initialization. Run only once.
   */
  @Override
  public void init(GLAutoDrawable drawable) {}

  /**
   * Call-back handler for window re-size event. Also called when the drawable is
   * first set to visible.
   */
  @Override
  public void reshape(GLAutoDrawable drawable, int x, int y, int width, int height) {}

  /**
   * Called back by the animator to perform rendering.
   */
  public void display(GLAutoDrawable drawable) {
    render(drawable);
    update();
  }

  /**
   * Called back before the OpenGL context is destroyed. Release resource such as buffers.
   */
  public void dispose(GLAutoDrawable drawable) {
  }

  private void render(GLAutoDrawable drawable) {
    // get the OpenGL 2 graphics context
    GL2 gl = drawable.getGL().getGL2();

    // clear color and depth buffers
    gl.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // reset the model-view matrix
    gl.glLoadIdentity();

    // Draw the tringle
    float sin = (float) Math.sin(angle);
    float cos = (float) Math.cos(angle);
    gl.glBegin(GL_TRIANGLES);
    gl.glColor3f(1.0f, 0.0f, 0.0f);   // Red
    gl.glVertex2d(-cos, -cos);
    gl.glColor3f(0.0f, 1.0f, 0.0f);   // Green
    gl.glVertex2d(0.0f, cos);
    gl.glColor3f(0.0f, 0.0f, 1.0f);   // Blue
    gl.glVertex2d(sin, -sin);
    gl.glEnd();
  }

  // Update the angle of the triangle after each frame
  private void update() {
    angle += 0.03f;
  }
}
