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

    public static class FrameData {
        public final long frameIdx;
        public final PointWithNormal point;
        public final Matrix3f transform;

        FrameData(long frameIndex, PointWithNormal point, Matrix3f transform) {
            this.frameIdx = frameIndex;
            this.point = point;
            this.transform = transform;
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
            return otherFrameData.frameIdx == this.frameIdx;
        }

        public int hashCode() {
            return Objects.hashCode(frameIdx);
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
     * Read a Map of frame->List{point/normals}.
     * Note that we don't use a Multimap because we depend on the order of items in the List.
     */
    private static Map<Long, List<PointWithNormal>> readPointsForFrames(InputStream in)
            throws IOException {
        ImmutableMap.Builder<Long, List<PointWithNormal>> pointsForFrames = ImmutableMap.builder();
        int numFrames = readInt(in);
        for (int i = 0; i < numFrames; i++) {
            int numPoints = readInt(in);
            ImmutableList.Builder<PointWithNormal> pointsForThisFrame = ImmutableList.builder();
            for (int j = 0; j < numPoints; j++) {
                Vector3f point = readVector3f(in);
                Vector3f normal = readVector3f(in);
                pointsForThisFrame.add(new PointWithNormal(point, normal));
            }
            pointsForFrames.put((long) i, pointsForThisFrame.build());
        }
        return pointsForFrames.build();
    }

    /**
     * Read a FrameData object and construct it.
     * FrameData consists of a frame index, and a point index. The point index
     * is an index into the list of all points in that frame. We read it and then lookup up the
     * correct actual PointWithNormal.
     */
    private static FrameData readFrameData(InputStream in, //
                                           Map<Long, List<PointWithNormal>> pointsForFrames)
            throws IOException {
        long frameIdx = readLong(in);
        long pointIdx = readLong(in);
        List<PointWithNormal> pointsForThisFrame = pointsForFrames.get(frameIdx);
        PointWithNormal point = pointsForThisFrame.get((int) pointIdx);
        Matrix3f transform = readMatrix3f(in);
        return new FrameData(frameIdx, point, transform);
    }

    private static Surfel readSurfel(InputStream in, Map<Long, List<PointWithNormal>> pointsForFrames)
            throws IOException {
        long surfelId = readLong(in);
        int numFrames = readInt(in);
        List<FrameData> frameData = Lists.newArrayList();
        while (numFrames > 0) {
            FrameData fd = readFrameData(in, pointsForFrames);
            frameData.add(fd);
            numFrames--;
        }

        int numNeighbours = readInt(in);
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
        Map<Long, List<PointWithNormal>> pointsForFrames = readPointsForFrames(in);
        List<Surfel> surfels = Lists.newArrayList();
        int numSurfels = readInt(in);
        while (numSurfels > 0) {
            Surfel surfel = readSurfel(in, pointsForFrames);
            surfels.add(surfel);
            numSurfels--;
        }

        return new State(surfels);
    }
}
