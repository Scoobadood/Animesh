package org.ddurbin.animesh.tools;

import static org.ddurbin.common.Check.CheckException;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Vector;

import org.ddurbin.common.Check;
import org.ddurbin.common.Matrix3f;
import org.ddurbin.common.Vector3f;


/**
 * State of the surfel mesh.
 */
public class State {
    private static final ByteOrder BYTE_ORDER = ByteOrder.LITTLE_ENDIAN;

    public static class PointWithNormal {
        public final Vector3f point;
        public final Vector3f normal;

        PointWithNormal(Vector3f point, Vector3f normal) {
            this.point = point;
            this.normal = normal;
        }
    }

    public static class PixelInFrame {
        public final long x;
        public final long y;
        public final long frameIndex;

        public PixelInFrame(long x, long y, long frameIndex) {
            this.x = x;
            this.y = y;
            this.frameIndex = frameIndex;
        }

        /**
         * Return true if the have the same index.
         */
        public boolean equals(Object otherObject) {
            if (otherObject == null) {
                return false;
            }
            if (this == otherObject) {
                return true;
            }
            if (!(otherObject instanceof PixelInFrame)) {
                return false;
            }
            PixelInFrame otherPixelInFrame = (PixelInFrame) otherObject;
            return ((frameIndex == otherPixelInFrame.frameIndex)
                    && (x == otherPixelInFrame.x)
                    && (y == otherPixelInFrame.y));
        }

        public int hashCode() {
            return Objects.hash(frameIndex, x, y);
        }
    }

    public static class FrameData {
        public final PixelInFrame pixelInFrame;    // x, y, frame
        public final Matrix3f transform;
        public final Vector3f normal;
        public final float depth;


        FrameData(PixelInFrame pixelInFrame, Matrix3f transform, Vector3f normal, float depth) {
            this.pixelInFrame = pixelInFrame;
            this.transform = transform;
            this.normal = normal;
            this.depth = depth;
        }

        /**
         * Return true if the have the same index.
         */
        public boolean equals(Object otherObject) {
            if (otherObject == null) {
                return false;
            }
            if (this == otherObject) {
                return true;
            }
            if (!(otherObject instanceof FrameData)) {
                return false;
            }
            FrameData otherFrameData = (FrameData) otherObject;
            return (pixelInFrame.equals(otherFrameData.pixelInFrame)
                    && (transform.equals(otherFrameData.transform))
                    && (normal.equals(otherFrameData.normal)));
        }

        public int hashCode() {
            return Objects.hash(pixelInFrame, transform, normal);
        }
    }

    public static class Surfel {
        public final long id;
        public final List<FrameData> frameData;
        public final List<Integer> neighbours;
        public final Vector3f tangent;

        Surfel(long id, List<FrameData> frameData, List<Integer> neighbours, Vector3f tangent) {
            this.id = id;
            this.frameData = frameData;
            this.neighbours = neighbours;
            this.tangent = tangent;
        }

        /**
         * Return True of they havethe same ID.
         */
        public boolean equals(Object otherObject) {
            if (otherObject == null) {
                return false;
            }
            if (this == otherObject) {
                return true;
            }
            if (!(otherObject instanceof Surfel)) {
                return false;
            }
            Surfel otherSurfel = (Surfel) otherObject;
            return otherSurfel.id == this.id;
        }

        public int hashCode() {
            return Objects.hashCode(id);
        }

    }

    public final List<Surfel> surfels;

    /**
     * Construct the State.
     *
     * @param surfels Surfel data
     */
    private State(List<Surfel> surfels) {
        this.surfels = surfels;
    }

    /**
     * Read a vector of 3 floating point numbers from the input.
     */
    private static Vector3f readVector3f(InputStream in) throws IOException {
        return new Vector3f(
                readFloat(in), //
                readFloat(in), //
                readFloat(in)  //
        );
    }

    /**
     * Read a floating point number from the input.
     */
    private static float readFloat(InputStream in) throws IOException {
        byte[] floatBuffer = new byte[4];
        if (in.read(floatBuffer) != 4) {
            throw new IOException("Out of data reading float");
        }
        return ByteBuffer.wrap(floatBuffer).order(BYTE_ORDER).getFloat();
    }

    /**
     * Read a big-endian integer from file.
     *
     * @param in InputStream to read from.
     * @return The intger read
     * @throws IOException If there is a problem reading the data.
     */
    private static int readInt(InputStream in) throws IOException {
        byte[] intBuffer = new byte[Integer.BYTES];
        if (in.read(intBuffer) != Integer.BYTES) {
            throw new IOException("Out of data reading int");
        }
        return ByteBuffer.wrap(intBuffer).order(BYTE_ORDER).getInt();
    }

    /**
     * Read a big-endian long from file.
     *
     * @param in InputStream to read from.
     * @return The long read
     * @throws IOException If there is a problem reading the data.
     */
    private static long readLong(InputStream in) throws IOException {
        byte[] longBuffer = new byte[Long.BYTES];
        if (in.read(longBuffer) != Long.BYTES) {
            throw new IOException("Out of data reading int");
        }
        return ByteBuffer.wrap(longBuffer).order(BYTE_ORDER).getLong();
    }

    /**
     * Read a vector of 3 floating point numbers from the input.
     */
    private static Matrix3f readMatrix3f(InputStream in) throws IOException {
        return new Matrix3f(
                new float[]{
                        readFloat(in), readFloat(in), readFloat(in),
                        readFloat(in), readFloat(in), readFloat(in),
                        readFloat(in), readFloat(in), readFloat(in)
                }
        );
    }


    /**
     * Read a Pixel in frame object.
     */
    private static PixelInFrame readPixelInFrame(InputStream in) throws IOException {
        long x = readLong(in);
        long y = readLong(in);
        long frameIdx = readLong(in);
        return new PixelInFrame(x, y, frameIdx);
    }

    /**
     * Read a FrameData object and construct it.
     * FrameData consists of a frame index, and a point index. The point index
     * is an index into the list of all points in that frame. We read it and then lookup up the
     * correct actual PointWithNormal.
     */
    private static FrameData readFrameData(InputStream in)
            throws IOException {
        PixelInFrame pif = readPixelInFrame(in);
        System.out.println(String.format("> f:%d x:%d y:%d", pif.frameIndex, pif.x, pif.y));
        float depth = readFloat(in);
        Matrix3f transform = readMatrix3f(in);
        Vector3f normal = readVector3f(in);
        return new FrameData(pif, transform, normal, depth);
    }

    /**
     * Read a Surfel from the InputStream and construct it.
     */
    private static Surfel readSurfel(InputStream in)
            throws IOException {
        long surfelId = readLong(in);
        System.out.println("Surfel " + surfelId);
        int numFrames = readInt(in);
        if (numFrames == 0) {
            throw new RuntimeException("Expected at least one frame for surfel id " + surfelId);
        }
        if (numFrames == 1) {
//            System.out.println("** WARNING: Surfel " + surfelId + " has only one frame");
        }
        List<FrameData> frameData = Lists.newArrayList();
        while (numFrames > 0) {
            FrameData fd = readFrameData(in);
            frameData.add(fd);
            numFrames--;
        }

        int numNeighbours = readInt(in);
        if (numNeighbours == 0) {
//            System.out.println("** WARNING: Surfel " + surfelId + " has no neighbours");
        }
        List<Integer> surfelNeighbours = Lists.newArrayList();
        while (numNeighbours > 0) {
            surfelNeighbours.add((int) readLong(in));
            numNeighbours--;
        }

        Vector3f tangent = readVector3f(in);

        return new Surfel(surfelId, frameData, surfelNeighbours, tangent);
    }

    public static State read(InputStream in) throws CheckException, IOException {
        Check.notNull(in, "Input stream cannot be null");
        List<Surfel> surfels = Lists.newArrayList();
        int numSurfels = readInt(in);
        if (numSurfels == 0) {
            throw new RuntimeException("Expected at least one surfel");
        }

        int total = numSurfels;
        int count = 0;
        while (numSurfels > 0) {
//            System.out.print(String.format("\rReading %d of %d    ", ++count, total));
            Surfel surfel = readSurfel(in);
            surfels.add(surfel);
            numSurfels--;
        }
        System.out.println();

        return new State(surfels);
    }
}
