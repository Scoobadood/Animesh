package org.ddurbin.animesh.viewer;

import com.jogamp.newt.event.WindowAdapter;
import com.jogamp.newt.event.WindowEvent;
import com.jogamp.newt.opengl.GLWindow;
import com.jogamp.opengl.GL;
import com.jogamp.opengl.GL2;
import com.jogamp.opengl.GLAutoDrawable;
import com.jogamp.opengl.GLCapabilities;
import com.jogamp.opengl.GLEventListener;
import com.jogamp.opengl.GLProfile;
import com.jogamp.opengl.util.FPSAnimator;

public class JoglMain implements GLEventListener {
  private static String TITLE = "JOGL 2 with NEWT";  // window's title
  private static final int WINDOW_WIDTH = 640;  // width of the drawable
  private static final int WINDOW_HEIGHT = 480; // height of the drawable
  private static final int FPS = 60; // animator's target frames pe r second

  private double theta = 0.0f;  // rotational angle

  /**
   * The entry main() method
   */
  public static void main(String[] args) {
    // Get the default OpenGL profile, reflecting the best for your running platform
    GLProfile glp = GLProfile.getDefault();
    // Specifies a set of OpenGL capabilities, based on your profile.
    GLCapabilities caps = new GLCapabilities(glp);
    // Create the OpenGL rendering canvas
    GLWindow window = GLWindow.create(caps);

    // Create a animator that drives canvas' display() at the specified FPS.
    final FPSAnimator animator = new FPSAnimator(window, FPS, true);

    window.addWindowListener(new WindowAdapter() {
      @Override
      public void windowDestroyNotify(WindowEvent arg0) {
        // Use a dedicate thread to run the stop() to ensure that the
        // animator stops before program exits.
        new Thread(() -> {
          animator.stop(); // stop the animator loop
          System.exit(0);
        }).start();
      }

      ;
    });

    window.addGLEventListener(new JoglMain());
    window.setSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    window.setTitle(TITLE);
    window.setVisible(true);
    animator.start();
  }

  /**
   * Called back by the drawable to render OpenGL graphics
   */
  @Override
  public void display(GLAutoDrawable drawable) {
    render(drawable);
    update();
  }

  /**
   * Render the shape (triangle)
   */
  private void render(GLAutoDrawable drawable) {
    GL2 gl = drawable.getGL().getGL2();

    gl.glClear(GL.GL_COLOR_BUFFER_BIT);

    // Draw a triangle
    float sine = (float) Math.sin(theta);
    float cosine = (float) Math.cos(theta);
    gl.glBegin(GL.GL_TRIANGLES);
    gl.glColor3f(1, 0, 0);
    gl.glVertex2d(-cosine, -cosine);
    gl.glColor3f(0, 1, 0);
    gl.glVertex2d(0, cosine);
    gl.glColor3f(0, 0, 1);
    gl.glVertex2d(sine, -sine);
    gl.glEnd();
  }

  /**
   * Update the rotation angle after each frame refresh
   */
  private void update() {
    theta += 0.01;
  }

  /**
   * Called back immediately after the OpenGL context is initialized
   */
  @Override
  public void init(GLAutoDrawable drawable) {
  }

  /**
   * Called back before the OpenGL context is destroyed.
   */
  @Override
  public void dispose(GLAutoDrawable drawable) {
  }

  /**
   * Called back by the drawable when it is first set to visible,
   * and during the first repaint after the it has been resized.
   */
  @Override
  public void reshape(GLAutoDrawable drawable, int x, int y, int weight, int height) {
  }
}
