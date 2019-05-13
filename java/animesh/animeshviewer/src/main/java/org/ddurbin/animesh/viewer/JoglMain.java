package org.ddurbin.animesh.viewer;

import static org.ddurbin.animesh.viewer.MatrixHelper.identity;
import static org.ddurbin.animesh.viewer.MatrixHelper.invert;
import static org.ddurbin.animesh.viewer.MatrixHelper.multiply;
import static org.ddurbin.animesh.viewer.MatrixHelper.rotate;
import static org.ddurbin.animesh.viewer.MatrixHelper.translate;


import com.google.common.base.Strings;
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
import java.io.BufferedReader;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.URL;
import java.nio.FloatBuffer;
import java.util.Arrays;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import javax.swing.*;

import com.jogamp.opengl.util.FPSAnimator;
import org.ddurbin.animesh.bin.State;
import org.ddurbin.animesh.bin.StateUtilities;
import org.ddurbin.common.Pair;
import org.ddurbin.common.Vector3f;
import org.junit.Assert;


/**
 * Adapted from example.
 *
 * @author Xerxes Rånby (xranby)
 * {@see http://jogamp.org/deployment/jogamp-current/archive/jogamp-all-platforms.7z}
 */
public class JoglMain implements GLEventListener, MouseListener {

    private static final String VERSION_STRING = "#version 330\n";
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
    private static float[] colours;
    private static float[] vertices;
    float camZ = 0.0f;
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
    private float model_tz = 2.0f;

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

    private JoglMain(float[] projectionMatrix) {
        this.projectionMatrix = projectionMatrix;
    }


    /**
     * Store the given RGB colour at the location in the array provided. Alphas is assumed to be 1.0;
     */
    private static void setColourAtPosition(float[] arr, int position, float r, float g, float b) {
        arr[position] = r;
        arr[position + 1] = g;
        arr[position + 2] = b;
        arr[position + 3] = 1.0f;
    }

    /**
     * Store the given RGB colour twice, once for each end of a line
     */
    private static void setLineColourAtPosition(float[] arr, int position, float r, float g, float b) {
        setColourAtPosition(arr, position, r, g, b);
        setColourAtPosition(arr, position + 4, r, g, b);
    }

    /**
     * Colour the normal and tangents at the give position in the array.
     */
    private static void setColoursForObjectAtPosition(float[] arr, int position) {
        // Normal is red
        setLineColourAtPosition(arr, position, 1.0f, 0.0f, 0.0f);
        // Principal tangent is green
        setLineColourAtPosition(arr, position + 8, 0.0f, 1.0f, 0.0f);
        // Other tangents are blue
        setLineColourAtPosition(arr, position + 16, 0.0f, 0.0f, 1.0f);
        setLineColourAtPosition(arr, position + 24, 0.0f, 0.0f, 1.0f);
    }


    /**
     * Load normals and tangets from state file and colour them appropriately.
     */
    private static void createWorld(State state) {
        vertices = StateToGlData.convertStateToGlData(state, 3);
        int numItems = (vertices.length / 24);
        colours = new float[numItems * 32];

        // We have 8 points making 4 lines
        for (int i = 0; i < (numItems); i++) {
            setColoursForObjectAtPosition(colours, i * 32);
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
        float[] projectionMatrix = new float[16];
        createProjectionMatrix((float) Math.toRadians(30), 0.005f, 3.0f, (width / height), projectionMatrix);

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
        controls.setBackground(Color.ORANGE);
        controls.setSize(100, 100);
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
            Assert.assertEquals(true, windowOpened);
        } catch (final InterruptedException e) {
            System.err.println("Closing wait interrupted: " + e.getMessage());
        }
        animator.start();
    }

    /**
     *
     */
    private Pair<Integer, Integer> removeHidden(float[] renderVertices, float[] renderColours) {
        if (!normalsEnabled && !tangentsEnabled && !principalTangentEnabled) {
            return new Pair(0, 0);
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

        // Each 'object' occupies 2 verts for normal + 6 verts for tangents
        // each vert occupies 3 floats.
        int numItems = (vertices.length / 24);
        int vertexDestIndex = 0;
        int colourDestIndex = 0;
        int numItemsOut = 0;
        for (int i = 0; i < numItems; i++) {

            int vertexSourceIndex = i * 24;
            int colourSourceIndex = i * 32;

            // we only write items to output that have normals visible to camera
            Vector3f normStart = new Vector3f(vertices[vertexSourceIndex], vertices[vertexSourceIndex + 1], vertices[vertexSourceIndex + 2]);
            Vector3f normEnd = new Vector3f(vertices[vertexSourceIndex + 3], vertices[vertexSourceIndex + 4], vertices[vertexSourceIndex + 5]);
            Vector3f norm = normEnd.minus(normStart);

            if (norm.dot(camOrigin) > 0) {
                numItemsOut++;
                if (normalsEnabled) {
                    System.arraycopy(vertices, vertexSourceIndex, renderVertices, vertexDestIndex, 6);
                    System.arraycopy(colours, colourSourceIndex, renderColours, colourDestIndex, 8);
                    vertexDestIndex += 6;
                    colourDestIndex += 8;
                }
                vertexSourceIndex += 6;
                colourSourceIndex += 8;
                if (principalTangentEnabled) {
                    System.arraycopy(vertices, vertexSourceIndex, renderVertices, vertexDestIndex, 6);
                    System.arraycopy(colours, colourSourceIndex, renderColours, colourDestIndex, 8);
                    vertexDestIndex += 6;
                    colourDestIndex += 8;
                }
                vertexSourceIndex += 6;
                colourSourceIndex += 8;
                if (tangentsEnabled) {
                    System.arraycopy(vertices, vertexSourceIndex, renderVertices, vertexDestIndex, 12);
                    System.arraycopy(colours, colourSourceIndex, renderColours, colourDestIndex, 16);
                    vertexDestIndex += 12;
                    colourDestIndex += 16;
                }
            }
        }
        // Size of items
        int itemSize = (normalsEnabled ? 6 : 0) + (principalTangentEnabled ? 6 : 0) + (tangentsEnabled ? 12 : 0);
        return new Pair<>(itemSize, vertexDestIndex);
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
        String[] vlines = new String[]{
                shaderString
        };
        int[] vlengths = new int[]{vlines[0].length()};
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

    private void setTransform(GL4 gl) {
        float[] pvm = new float[16];
        identity(pvm);
        pvm = multiply(rotationMatrix, pvm);
        pvm = translate(pvm, model_tx, model_ty, model_tz);
        pvm = multiply(projectionMatrix, pvm);

        // Send the final projection matrix to the vertex shader by
        // using the uniform location id obtained during the init part.
        gl.glUniformMatrix4fv(shadProg.uniformMvpMatrix, 1, false, pvm, 0);
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
        // Get gl
        GL4 gl = drawable.getGL().getGL4();

        // Clear screen
        gl.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        gl.glClear(GL4.GL_STENCIL_BUFFER_BIT | GL4.GL_COLOR_BUFFER_BIT | GL4.GL_DEPTH_BUFFER_BIT);

        // Use the shaderProgram that got linked during the init part.
        gl.glUseProgram(shadProg.shaderProgramId);

        // Set the MVP matrix
        setTransform(gl);
        float[] renderVertices = new float[vertices.length];
        float[] renderColour = new float[colours.length];
        Pair<Integer, Integer> renderObjects = removeHidden(renderVertices, renderColour);
        int itemSize = renderObjects.first;
        int dataSize = renderObjects.second;
        int numItems = dataSize / itemSize;
        renderVertices = Arrays.copyOf(renderVertices, dataSize);
        renderColour = Arrays.copyOf(renderColour, numItems * (itemSize * 4 / 3));

        setVbo(gl, renderVertices, 3, VERTICES_IDX, 0);
        setVbo(gl, renderColour, 4, COLOR_IDX, 1);

        gl.glDrawArrays(GL4.GL_LINES, 0, dataSize); //Draw the vertices as lines
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
        m = rotate(m, (float) (deltaX * Math.PI * 2), 0.0f, 1.0f, 0.0f);
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
        float deltaZ = deltaY;
        mouseDownX = e.getX();
        mouseDownY = e.getY();

        int mod = e.getModifiers();

        if (isNoModifier(mod)) {
            rotateXY(deltaX, deltaY);
        } else if (isCmdModifier(mod)) {
            rotateZ(deltaZ);
        } else if (isCtrlModifier(mod)) {
            zoom(deltaZ);
        } else if (isShiftModifier(mod)) {
            pan(deltaX, deltaY);
        }
    }

    public void enableNormals(boolean normalsEnabled) {
        this.normalsEnabled = normalsEnabled;
    }

    public boolean isNormalsEnabled() {
        return normalsEnabled;
    }

    public void enableTangents(boolean tangentsEnabled) {
        this.tangentsEnabled = tangentsEnabled;
    }

    public boolean isTangentsEnabled() {
        return tangentsEnabled;
    }


    public void enablePrincipalTangent(boolean principalTangentEnabled) {
        this.principalTangentEnabled = principalTangentEnabled;
    }

    public boolean isPrincipalTangentEnabled() {
        return principalTangentEnabled;
    }

    @Override
    public void mouseWheelMoved(MouseEvent e) {
    }

    //
    // Shader support
    //
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