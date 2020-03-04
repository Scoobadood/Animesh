package org.ddurbin.animesh.viewer;

import static org.ddurbin.animesh.viewer.MatrixHelper.identity;
import static org.ddurbin.animesh.viewer.MatrixHelper.invert;
import static org.ddurbin.animesh.viewer.MatrixHelper.multiply;
import static org.ddurbin.animesh.viewer.MatrixHelper.rotate;
import static org.ddurbin.animesh.viewer.MatrixHelper.translate;

import com.jogamp.common.nio.Buffers;
import com.jogamp.newt.awt.NewtCanvasAWT;
import com.jogamp.newt.opengl.GLWindow;
import com.jogamp.opengl.GL;
import com.jogamp.opengl.GL4;
import com.jogamp.opengl.GLAutoDrawable;
import com.jogamp.opengl.GLCapabilities;
import com.jogamp.opengl.GLEventListener;
import com.jogamp.opengl.GLProfile;
import com.jogamp.opengl.util.Animator;

import java.awt.*;
import java.nio.FloatBuffer;
import java.util.Arrays;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import javax.swing.*;

import org.ddurbin.animesh.tools.State;
import org.ddurbin.animesh.tools.StateUtilities;
import org.ddurbin.common.Pair;
import org.ddurbin.common.Vector3f;


/**
 * Adapted from example.
 *
 * @author Xerxes Rånby (xranby)
 * {@see http://jogamp.org/deployment/jogamp-current/archive/jogamp-all-platforms.7z}
 */
public class JoglMain implements GLEventListener {
    /*
     * Rendering flags
     */
    private boolean normalsEnabled = true;
    private boolean tangentsEnabled = true;
    private boolean principalTangentEnabled = true;

    /*
     * Colouring in
     */
    SurfelColourer surfelColour;

    /*
     * Scene Data
     */
    private static final int COLOR_IDX = 0;
    private static final int VERTICES_IDX = 1;
    private static final double DELTA_ROT = Math.PI / 100.0;
    /*
     * Window related
     */
    private static int width = 1920;
    private static int height = 1080;
    // World
    private float[] vertices;
    /*
     * Rotation matrix
     */
    private float[] rotationMatrix = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };

    /**
     * Projection Matrix
     */
    private float[] projectionMatrix = new float[16];

    private float model_tx = 0.0f;
    private float model_ty = 0.0f;
    private float model_tz = 5.0f;

    /*
     * Shader related variables/handles
     */
    private MyShaderProgram shadProg;

    // Where we put our handles for vertex buffers
    private int[] vboHandles;

    // Ctor
    private JoglMain(float[] vertices, float[] projectionMatrix) {
        this.vertices = vertices;
        this.projectionMatrix = projectionMatrix;
        this.surfelColour = new DefaultSurfelColourer(); //SurfelOrientationColourer(vertices);
    }

    /**
     * Load normals and tangents from state file and colour them appropriately.
     */
    private static float[] createWorld(State state, int frame) {
        return StateToGlData.convertStateToGlData(state, frame);
    }


    int[] computeSurfelsToRender( ) {
        // Construct VM matrix
        float[] vm = new float[16];
        identity(vm);
        vm = translate(vm, -model_tx, -model_ty, -model_tz);
        vm = multiply(invert(rotationMatrix), vm);
        Vector3f camOrigin = new Vector3f(
                vm[12],
                vm[13],
                vm[14]
        );

        int numSourceSurfels = vertices.length / Constants.FLOATS_FOR_SURFEL;
        int[] renderSurfelIndices = new int[numSourceSurfels];
        int numRenderSurfels = 0;

        // We only write items to output that have normals visible to camera
        for (int i = 0; i < numSourceSurfels; i++) {
            int sourceVertexIndex = i * Constants.FLOATS_FOR_SURFEL;

            Vector3f normStart = new Vector3f(vertices[sourceVertexIndex], vertices[sourceVertexIndex + 1], vertices[sourceVertexIndex + 2]);
            Vector3f normEnd = new Vector3f(vertices[sourceVertexIndex + 3], vertices[sourceVertexIndex + 4], vertices[sourceVertexIndex + 5]);
            Vector3f norm = normEnd.minus(normStart);
            if (norm.dot(camOrigin) > 0) {
                renderSurfelIndices[numRenderSurfels++] = i;
            }
        }
        return Arrays.copyOfRange(renderSurfelIndices, 0, numRenderSurfels);
    }

    /**
     * Return an array of vertices to be rendered and an array of Surfel indices to render
     */
    private Pair<float[], int[]> removeHidden() {
        if (!normalsEnabled && !tangentsEnabled && !principalTangentEnabled) {
            return new Pair<>(new float[0],new int[0]);
        }

        int[] surfelsToRender = computeSurfelsToRender();

        // Storage per surfel
        int floatsPerSurfel = (normalsEnabled ? Constants.FLOATS_FOR_NORMAL : 0) + (tangentsEnabled ? Constants.FLOATS_FOR_TANGENTS : 0);
        float[] renderVertices = new float[floatsPerSurfel * surfelsToRender.length];

        int targetVertexIndex = 0;

        for (int value : surfelsToRender) {
            int sourceSurfelVertexIndex = value * Constants.FLOATS_FOR_SURFEL;
            if (normalsEnabled) {
                System.arraycopy(vertices, sourceSurfelVertexIndex, renderVertices, targetVertexIndex, Constants.FLOATS_FOR_NORMAL);
                targetVertexIndex += Constants.FLOATS_FOR_NORMAL;
            }
            sourceSurfelVertexIndex += Constants.FLOATS_FOR_NORMAL;
            if (tangentsEnabled) {
                System.arraycopy(vertices, sourceSurfelVertexIndex, renderVertices, targetVertexIndex, Constants.FLOATS_FOR_TANGENTS);
                targetVertexIndex += Constants.FLOATS_FOR_TANGENTS;
            }
        }

        return new Pair<>(renderVertices, surfelsToRender);
    }

    private static void createProjectionMatrix(float fovy, float near, float far, float aspect,
                                               float[] pm) {
        double d = 1.0f / Math.tan(fovy / 2.0);
        for (int i = 0; i < 16; i++) {
            pm[i] = 0;
        }
        pm[0] = (float) (d / aspect);
        pm[5] = (float) d;
        float diffZ = near - far;
        pm[10] = -(near + far) / diffZ;
        pm[11] = 1.0f;
        pm[14] = (2 * far * near) / diffZ;
    }

    private void reportCapabilities(GLAutoDrawable drawable, GL gl) {
        System.out.println("Chosen GLCapabilities: " + drawable.getChosenGLCapabilities());
        System.out.println("INIT GL IS: " + gl.getClass().getName());
        System.out.println("GL_VENDOR: " + gl.glGetString(GL.GL_VENDOR));
        System.out.println("GL_RENDERER: " + gl.glGetString(GL.GL_RENDERER));
        System.out.println("GL_VERSION: " + gl.glGetString(GL.GL_VERSION));
    }

    /**
     * GLEventListener::init.
     */
    public void init(GLAutoDrawable drawable) {
        GL4 gl = drawable.getGL().getGL4();
        try {
            reportCapabilities(drawable, gl);
            shadProg = MyShaderProgram.setupShaderProgram(gl);
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
    public void reshape(GLAutoDrawable drawable, int x, int y, int w, int h) {
        System.out.println("Window resized to width=" + w + " height=" + h);
        width = w;
        height = h;

        // Get gl
        GL4 gl = drawable.getGL().getGL4();

        // Optional: Set viewport
        // Render to a square at the center of the window.
        gl.glViewport((width - height) / 2, 0, width, height);
    }


    // Make a transformation matrix based on position of cam.
    private void setTransform(GL4 gl, float[] pvm) {
        identity(pvm);
        float[] newPvm = multiply(rotationMatrix, pvm);
        newPvm  = translate(newPvm, model_tx, model_ty, model_tz);
        newPvm  = multiply(projectionMatrix, newPvm);

        // Send the final projection matrix to the vertex shader by
        // using the uniform location id obtained during the init part.
        gl.glUniformMatrix4fv(shadProg.uniformMvpMatrix, 1, false, newPvm , 0);
        System.arraycopy(newPvm, 0, pvm, 0, pvm.length);
    }

    private void setVbo(GL4 gl, float[] data, int numItems, int dataSize, int vboIndex, int vaaIndex) {
        gl.glBindBuffer(GL4.GL_ARRAY_BUFFER, vboHandles[vboIndex]);

        FloatBuffer fbData = Buffers.newDirectFloatBuffer(data, 0, numItems * dataSize);
        int numBytes = numItems * dataSize * Constants.BYTES_PER_FLOAT;
        gl.glBufferData(GL.GL_ARRAY_BUFFER, numBytes, fbData, GL.GL_STATIC_DRAW);

        // Associate Vertex attribute 0 with the last bound VBO
        gl.glVertexAttribPointer( //
                vaaIndex,                   // the vertex attribute
                dataSize,                   // Components per generic vertex attribute. 1, 2, 3 or 4
                GL4.GL_FLOAT,               // Type of each attribute
                false,              // normalized? Applies to fixed point data
                0,                      // Byte offset between items in the array
                0);             // Pointer to the first attribute in the array
        gl.glEnableVertexAttribArray(vaaIndex);
        int err = gl.glGetError();
        assert( err == GL.GL_NO_ERROR);
    }

    /**
     * GLEventListener::display.
     */
    public void display(GLAutoDrawable drawable) {
        // Get gl
        GL4 gl = drawable.getGL().getGL4();

        // Clear screen
        gl.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        gl.glClear(GL4.GL_STENCIL_BUFFER_BIT | GL4.GL_COLOR_BUFFER_BIT | GL4.GL_DEPTH_BUFFER_BIT);

        // Use the shaderProgram that got linked during the init part.
        gl.glUseProgram(shadProg.shaderProgramId);

        // Set the MVP matrix
        float[] pvm = new float[16];
        setTransform(gl, pvm);

        // Populate renderVertices with the vertices to be rendered.
        // Return the number of Surfels (some are hidden) and number of vertices per surfel (some are hidden)
        Pair<float[], int[]> render = removeHidden();
        float[] renderVertices = render.first;
        int[] renderSurfelIndices = render.second;

        float[] renderColour = surfelColour.generateColoursForSurfels(renderSurfelIndices, normalsEnabled, tangentsEnabled, principalTangentEnabled);
        int numSurfels = renderSurfelIndices.length;

        int expectedVertsPerSurfel = (normalsEnabled ? 2 : 0 ) + ( tangentsEnabled ? 6 : 0);
        assert (renderColour.length == numSurfels * expectedVertsPerSurfel * Constants.NUM_COLOUR_PLANES);
        assert (renderVertices.length == numSurfels * expectedVertsPerSurfel * Constants.FLOATS_FOR_VERTEX);

        setVbo(gl, renderVertices, numSurfels, 3, VERTICES_IDX, 0);
        setVbo(gl, renderColour, numSurfels,4, COLOR_IDX, 1);

        gl.glDrawArrays(GL4.GL_LINES, 0, renderVertices.length/2); //Draw the vertices as lines
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




    /*
     * Rotate around X and Y axes
     */
    public void rotateXY(int dX, int dY) {
        float deltaX = dX / (float) width;
        float deltaY = dY / (float) height;
        float[] m = new float[16];
        identity(m);
        m = rotate(m, (float) (-deltaY * Math.PI * 2), 1.0f, 0.0f, 0.0f);
        m = rotate(m, (float) (-deltaX * Math.PI * 2), 0.0f, 1.0f, 0.0f);
        rotationMatrix = multiply(m, rotationMatrix);
    }

    /*
     * Rotate around Z axis
     */
    public void rotateZ(int dZ) {
        float deltaZ = dZ / (float) width;
        float[] m = new float[16];
        identity(m);
        m = rotate(m, (float) (deltaZ * Math.PI * 2), 0.0f, 0.0f, -1.0f);
        rotationMatrix = multiply(m, rotationMatrix);
    }

    /*
     * Rotate around Z axis.
     */
    public void pan(int dX, int dY) {
        float deltaX = dX / (float) width;
        float deltaY = dY / (float) height;
        model_tx += (2 * deltaX);
        model_ty -= (2 * deltaY);
    }

    /*
     * Zoom in and out
     */
    public void zoom(int dZ) {
        float deltaZ = dZ / (float)height;
        model_tz -= 2 * deltaZ;
    }

    void enableNormals(boolean normalsEnabled) {
        this.normalsEnabled = normalsEnabled;
    }

    boolean isNormalsEnabled() {
        return normalsEnabled;
    }

    void enableTangents(boolean tangentsEnabled) {
        this.tangentsEnabled = tangentsEnabled;
    }

    boolean isTangentsEnabled() {
        return tangentsEnabled;
    }


    void enablePrincipalTangent(boolean principalTangentEnabled) {
        this.principalTangentEnabled = principalTangentEnabled;
    }

    boolean isPrincipalTangentEnabled() {
        return principalTangentEnabled;
    }


    /**
     * Run it.
     */
    public static void main(String[] args) throws Exception {
        State state = StateUtilities.loadState(args[0], args[1]);
        int frameId = 6;
        if( args.length > 2 ) {
            frameId = Integer.valueOf(args[2]);
        }
        float[] vertices = createWorld(state, frameId);


        GLCapabilities caps = new GLCapabilities(GLProfile.get(GLProfile.GL4));

        // We may at this point tweak the caps and request a translucent drawable
        caps.setBackgroundOpaque(false);
        GLWindow glWindow = GLWindow.create(caps);

        // In this demo we prefer to setup and view the GLWindow directly
        // this allows the demo to run on -Djava.awt.headless=true systems
        glWindow.setTitle("Animesh");
        glWindow.setSize(width, height);
        glWindow.setUndecorated(false);
        glWindow.setPointerVisible(true);
        glWindow.setVisible(true);

        // Finally we connect the GLEventListener application code to the NEWT GLWindow.
        // GLWindow will call the GLEventListener init, reshape, display and dispose
        // functions when needed.
        float[] projectionMatrix = new float[16];
        createProjectionMatrix((float) Math.toRadians(35), 0.005f, 19.0f, (width / (float)height), projectionMatrix);

        JoglMain jm = new JoglMain(vertices, projectionMatrix);

        glWindow.addGLEventListener(jm);
        glWindow.addMouseListener(new AnimeshMouseListener(jm));

        JFrame frame = new JFrame();
        frame.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        frame.setTitle("Daves Frame");
        frame.setBounds(10, 10, 1000, 800);

        JPanel panel = new JPanel();
        panel.setBackground(Color.blue);
        panel.setLayout(new BorderLayout());
        NewtCanvasAWT canvas = new NewtCanvasAWT(glWindow);
        canvas.setBackground(Color.red);
        panel.add(canvas, BorderLayout.CENTER);
        JPanel controls = new ControlPanel(jm);
        controls.setSize(width / 4, height);
        panel.add(controls, BorderLayout.WEST);

        frame.getContentPane().add(panel);
        Animator animator = new Animator();
        animator.add(glWindow);

        // make the window visible using the EDT
        final Semaphore windowOpenSemaphore = new Semaphore(0);
        SwingUtilities.invokeLater(() -> {
            frame.pack();
            frame.setVisible(true);
            windowOpenSemaphore.release();
        });

        // wait for the window to be visible and start the animation
        try {
            final boolean windowOpened = windowOpenSemaphore.tryAcquire(5000, TimeUnit.MILLISECONDS);
            assert (windowOpened);
        } catch (final InterruptedException e) {
            System.err.println("Closing wait interrupted: " + e.getMessage());
        }
        animator.start();
    }
}