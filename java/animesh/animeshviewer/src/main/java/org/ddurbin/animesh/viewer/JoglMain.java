package org.ddurbin.animesh.viewer;

import com.jogamp.opengl.awt.GLCanvas;
import com.jogamp.opengl.util.FPSAnimator;
import java.awt.Dimension;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import javax.swing.JFrame;
import javax.swing.SwingUtilities;

public class JoglMain {
  private static String TITLE = "JOGL 2.0 Setup (GLCanvas)";
  private static final int CANVAS_WIDTH = 640;
  private static final int CANVAS_HEIGHT = 480;
  private static final int FPS = 30;

  /**
   * The entry main() method to setup the top-level container and animator
   */
  public static void main(String[] args) {
    // Run the GUI codes in the event-dispatching thread for thread safety
    SwingUtilities.invokeLater(JoglMain::new);
  }

  /**
   * Constructor to setup the GUI for this Component
   */
  public JoglMain() {
    GLCanvas canvas = new JoglTriangle();
    canvas.setPreferredSize(new Dimension(CANVAS_WIDTH, CANVAS_HEIGHT));

    // Create a animator that drives canvas' display() at the specified FPS.
    final FPSAnimator animator = new FPSAnimator(canvas, FPS, true);

    // Create the top-level container
    final JFrame frame = new JFrame();
    frame.getContentPane().add(canvas);

    frame.addWindowListener(new WindowAdapter() {
      @Override
      public void windowClosing(WindowEvent e) {
        // Use a dedicate thread to run the stop() to ensure that the animator stops before program exits.
        new Thread(() -> {
          if (animator.isStarted()) {
            animator.stop();
          }
          System.exit(0);
        }).start();
      }
    });

    frame.setTitle(TITLE);
    frame.pack();
    frame.setVisible(true);
    animator.start(); // start the animation loop
  }


}
