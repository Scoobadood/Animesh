package org.ddurbin.animesh.viewer;

import static org.ddurbin.animesh.viewer.MatrixHelper.identity;
import static org.ddurbin.animesh.viewer.MatrixHelper.invert;
import static org.ddurbin.animesh.viewer.MatrixHelper.multiply;
import static org.ddurbin.animesh.viewer.MatrixHelper.rotate;
import static org.ddurbin.animesh.viewer.MatrixHelper.translate;

import com.jogamp.common.nio.Buffers;
import com.jogamp.newt.awt.NewtCanvasAWT;
import com.jogamp.newt.event.MouseEvent;
import com.jogamp.newt.event.MouseListener;
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
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import javax.swing.*;

import org.ddurbin.animesh.tools.State;
import org.ddurbin.animesh.tools.StateUtilities;
import org.ddurbin.common.Vector3f;


/**
 * Adapted from example.
 *
 * @author Xerxes Rånby (xranby)
 * {@see http://jogamp.org/deployment/jogamp-current/archive/jogamp-all-platforms.7z}
 */
public class JoglMain implements GLEventListener, MouseListener {
    private static final int VERTICES_FOR_NORMAL = 2;
    private static final int FLOATS_FOR_NORMAL = VERTICES_FOR_NORMAL * 3;
    private static final int VERTICES_FOR_TANGENTS = 6;
    private static final int FLOATS_FOR_TANGENTS = VERTICES_FOR_TANGENTS * 3;
    private static final int NUM_COLOUR_PLANES = 4;
    private static final int BYTES_PER_FLOAT = 4;

    /*
     * Rendering flags
     */
    private boolean normalsEnabled = true;
    private boolean tangentsEnabled = true;
    private boolean principalTangentEnabled = true;

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
    private static float[] vertices;
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

    static final Colour NORMAL_COLOUR = new Colour( 1.0f, 0.0f, 0.0f );
    static final Colour PRIMARY_TANGENT_COLOUR = new Colour( 0.0f, 1.0f, 0.0f );
    static final Colour ORTH_TANGENT_COLOUR = new Colour( 1.0f, 1.0f, 2.0f );

    /*
     * Shader related variables/handles
     */
    private MyShaderProgram shadProg;

    // Where we put our handles for vertex buffers
    private int[] vboHandles;
    //
    // Mouse Management
    //
    private int mouseDownX;
    private int mouseDownY;

    // Ctor
    private JoglMain(float[] projectionMatrix) {
        this.projectionMatrix = projectionMatrix;
    }


    /**
     * Load normals and tangents from state file and colour them appropriately.
     */
    private static void createWorld(State state, int frame) {
        vertices = StateToGlData.convertStateToGlData(state, frame);
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
        createWorld(state, frameId);


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
        float[] projectionMatrix = new float[16];
        createProjectionMatrix((float) Math.toRadians(35), 0.005f, 19.0f, (width / height), projectionMatrix);

        JoglMain jm = new JoglMain(projectionMatrix);

        glWindow.addGLEventListener(jm);
        glWindow.addMouseListener(jm);

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

    /**
     * Generate an array of coordinates to be rendered. Return this as well as the size of the resulting items.
     *
     */
    private int removeHidden(float[] outputVertexData) {
        if (!normalsEnabled && !tangentsEnabled && !principalTangentEnabled) {
            return 0;
        }

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

        int vertexDestIndex = 0;
        int vertexSourceIndex = 0;

        // Each 'object' occupies 2 verts for normal + 6 verts for tangents
        // each vert occupies 3 floats.
        int numItems = vertices.length / ((VERTICES_FOR_NORMAL + VERTICES_FOR_TANGENTS) * 3);
        for (int i = 0; i < numItems; i++) {
            // We only write items to output that have normals visible to camera
            Vector3f normStart = new Vector3f(vertices[vertexSourceIndex], vertices[vertexSourceIndex + 1], vertices[vertexSourceIndex + 2]);
            Vector3f normEnd = new Vector3f(vertices[vertexSourceIndex + 3], vertices[vertexSourceIndex + 4], vertices[vertexSourceIndex + 5]);
            Vector3f norm = normEnd.minus(normStart);
            if (norm.dot(camOrigin) > 0) {
                if (normalsEnabled) {
                    System.arraycopy(vertices, vertexSourceIndex, outputVertexData, vertexDestIndex, FLOATS_FOR_NORMAL);
                    vertexDestIndex += FLOATS_FOR_NORMAL;
                }
                vertexSourceIndex += FLOATS_FOR_NORMAL;

                if (tangentsEnabled) {
                    System.arraycopy(vertices, vertexSourceIndex, outputVertexData, vertexDestIndex, FLOATS_FOR_TANGENTS);
                    vertexDestIndex += FLOATS_FOR_TANGENTS;
                }
                vertexSourceIndex += FLOATS_FOR_TANGENTS;
            } else {
                vertexSourceIndex += (FLOATS_FOR_NORMAL + FLOATS_FOR_TANGENTS);
            }
        }

        return vertexDestIndex / 3;
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
        FloatBuffer fbData = Buffers.newDirectFloatBuffer(data, 0, numItems * dataSize);
        gl.glBindBuffer(GL4.GL_ARRAY_BUFFER, vboHandles[vboIndex]);
        int numBytes = numItems * dataSize * BYTES_PER_FLOAT;
        gl.glBufferData(GL.GL_ARRAY_BUFFER, numBytes, fbData, GL.GL_STATIC_DRAW);

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
     * Generate vertex colours.
     * We are given N, the number of items in a float array and S the size of each item
     * S can be 0 (we're not showing anything)
     *          6 (we're showing just normal or primary tangent)
     * There are V = N/3 vertices.
     * There are L = V/2 = N/6 lines.
     * There are I = L/4 = N/24 distinct items.
     * Each item has a normal, primary tangent, opposing tangent and orthogonal tangents
     * We must colour them.
     */
    float[] generateVertexColours(int numVertices) {
        // Compute size of each item (surfel) in vertices
        int verticesPerSurfel = (normalsEnabled ? VERTICES_FOR_NORMAL : 0) + (tangentsEnabled ? VERTICES_FOR_TANGENTS : 0);
        if( verticesPerSurfel == 0 ) return new float[0];
        // Number of items is the number of vertices
        int numSurfels = numVertices / verticesPerSurfel;
        assert( numVertices % verticesPerSurfel == 0 );

        // Number of colours depends on a what's on display
        float[] colours = new float[ numVertices * NUM_COLOUR_PLANES];
        int destColourIndex = 0;
        for( int i=0; i<numSurfels; i++ ) {
            if( normalsEnabled) {
                for( int j=0; j<VERTICES_FOR_NORMAL; j++ ) {
                    // Copy normal colour for two vertices
                    colours[destColourIndex++] = NORMAL_COLOUR.red;
                    colours[destColourIndex++] = NORMAL_COLOUR.green;
                    colours[destColourIndex++] = NORMAL_COLOUR.blue;
                    colours[destColourIndex++] = NORMAL_COLOUR.alpha;
                }
            }
            if( tangentsEnabled) {
                for( int j=0; j<VERTICES_FOR_TANGENTS; j++ ) {
                    // Copy normal colour for two vertices
                    colours[destColourIndex++] = PRIMARY_TANGENT_COLOUR.red;
                    colours[destColourIndex++] = PRIMARY_TANGENT_COLOUR.green;
                    colours[destColourIndex++] = PRIMARY_TANGENT_COLOUR.blue;
                    colours[destColourIndex++] = PRIMARY_TANGENT_COLOUR.alpha;
                }
            }
        }
        return colours;
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

        float[] renderVertices = new float[vertices.length];
        int numRenderVertices = removeHidden(renderVertices);
        float[] renderColour = generateVertexColours(numRenderVertices);

        setVbo(gl, renderVertices, numRenderVertices, 3, VERTICES_IDX, 0);
        setVbo(gl, renderColour, numRenderVertices,4, COLOR_IDX, 1);

        gl.glDrawArrays(GL4.GL_LINES, 0, numRenderVertices); //Draw the vertices as lines
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

    @Override
    public void mouseClicked(MouseEvent e) {
    }

    @Override
    public void mouseEntered(MouseEvent e) {
    }

    @Override
    public void mouseExited(MouseEvent e) {
    }

    @Override
    public void mousePressed(MouseEvent e) {
        mouseDownX = e.getX();
        mouseDownY = e.getY();
    }

    @Override
    public void mouseReleased(MouseEvent e) {
    }

    @Override
    public void mouseMoved(MouseEvent e) {
    }

    private boolean isShiftModifier(int mod) {
        return ((mod & MouseEvent.SHIFT_MASK) != 0)
                && ((mod & MouseEvent.ALT_MASK) == 0)
                && ((mod & MouseEvent.META_MASK) == 0)
                && ((mod & MouseEvent.CTRL_MASK) == 0);
    }

    private boolean isNoModifier(int mod) {
        return ((mod & MouseEvent.SHIFT_MASK) == 0)
                && ((mod & MouseEvent.ALT_MASK) == 0)
                && ((mod & MouseEvent.META_MASK) == 0)
                && ((mod & MouseEvent.CTRL_MASK) == 0);
    }

    private boolean isCmdModifier(int mod) {
        return ((mod & MouseEvent.SHIFT_MASK) == 0)
                && ((mod & MouseEvent.ALT_MASK) == 0)
                && ((mod & MouseEvent.META_MASK) != 0)
                && ((mod & MouseEvent.CTRL_MASK) == 0);
    }

    private boolean isCtrlModifier(int mod) {
        return ((mod & MouseEvent.SHIFT_MASK) == 0)
                && ((mod & MouseEvent.ALT_MASK) == 0)
                && ((mod & MouseEvent.META_MASK) == 0)
                && ((mod & MouseEvent.CTRL_MASK) != 0);
    }

    /*
     * Rotate around X and Y axes
     */
    private void rotateXY(float deltaX, float deltaY) {
        float[] m = new float[16];
        identity(m);
        m = rotate(m, (float) (-deltaY * Math.PI * 2), 1.0f, 0.0f, 0.0f);
        m = rotate(m, (float) (-deltaX * Math.PI * 2), 0.0f, 1.0f, 0.0f);
        rotationMatrix = multiply(m, rotationMatrix);
    }

    /*
     * Rotate around Z axis
     */
    private void rotateZ(float deltaZ) {
        float[] m = new float[16];
        identity(m);
        m = rotate(m, (float) (deltaZ * Math.PI * 2), 0.0f, 0.0f, -1.0f);
        rotationMatrix = multiply(m, rotationMatrix);
    }

    /*
     * Rotate around Z axis.
     */
    private void pan(float deltaX, float deltaY) {
        model_tx += 2 * deltaX;
        model_ty -= 2 * deltaY;
    }

    /*
     * Zoom in and out
     */
    private void zoom(float deltaZ) {
        model_tz -= 2 * deltaZ;
    }

    @Override
    public void mouseDragged(MouseEvent e) {
        float deltaX = (e.getX() - mouseDownX) / (float) width;
        float deltaY = (e.getY() - mouseDownY) / (float) height;
        mouseDownX = e.getX();
        mouseDownY = e.getY();

        int mod = e.getModifiers();

        if (isNoModifier(mod)) {
            rotateXY(deltaX, deltaY);
        } else if (isCmdModifier(mod)) {
            rotateZ(deltaX);
        } else if (isCtrlModifier(mod)) {
            zoom(deltaY*4.0f);
        } else if (isShiftModifier(mod)) {
            pan(deltaX, deltaY);
        }
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

    @Override
    public void mouseWheelMoved(MouseEvent e) {
    }

}