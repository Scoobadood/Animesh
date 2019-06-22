package org.ddurbin.common;

import java.io.*;

public class Camera {
    private final Vector3f position;
    private final Vector3f view;
    private final Vector3f up;
    private final Vector2f resolution;
    private final Vector2f fov;
    private final float focalDistance;
    private Matrix3f rotation;
    private Matrix3f k;
    private Vector3f translation;

    public Camera(Vector3f position, Vector3f view, Vector3f up, Vector2f resolution, Vector2f fov, float focalDistance) {
        this.position = position;
        this.view = view;
        this.up = up;
        this.resolution = resolution;
        this.fov = fov;
        this.focalDistance = focalDistance;
    }

    /**
     * Read x,y, z coordinates from a string.
     */
    private static Vector3f parseVec3f(String str) {
        String[] parts = str.split(",");
        if (parts.length != 3) {
            throw new IllegalArgumentException(String.format("[%s] does not have x, y and z", str));
        }
        try {
            float x = Float.valueOf(parts[0]);
            float y = Float.valueOf(parts[0]);
            float z = Float.valueOf(parts[0]);
            return new Vector3f(x, y, z);
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException(String.format("[%s] has non-numeric x, y or z", str));
        }
    }

    /**
     * Read x,y coordinates from a string.
     */
    private static Vector2f parseVec2(String str) {
        String[] parts = str.split(",");
        if (parts.length != 3) {
            throw new IllegalArgumentException(String.format("[%s] does not have x, y and z", str));
        }
        try {
            float x = Float.valueOf(parts[0]);
            float y = Float.valueOf(parts[0]);
            return new Vector2f(x, y);
        } catch (NumberFormatException e) {
            throw new IllegalArgumentException(String.format("[%s] has non-numeric x, y or z", str));
        }
    }

    /**
     * Load camera settings from a file.
     */
    public static Camera loadFromFile(String filename) throws IOException {
        boolean flPosition, flView, flUp, flResolution, flFov, flF;
        flPosition = flView = flUp = flResolution = flFov = flF = false;

        Vector3f position = null, view = null, up = null;
        Vector2f resolution = null, fov = null;
        float focalDistance = 0.0f;

        LineNumberReader lnr = new LineNumberReader(new FileReader(new File(filename)));

        String line;
        while ((line = lnr.readLine()) != null) {
            String[] keyValue = line.split("=");
            if (keyValue.length != 2) {
                throw new IllegalArgumentException("Bad line in camera file : " + line);
            }
            switch (keyValue[0]) {
                case "position":
                    position = parseVec3f(keyValue[1]);
                    flPosition = true;
                    break;
                case "view":
                    view = parseVec3f(keyValue[1]);
                    flView = true;
                    break;
                case "up":
                    up = parseVec3f(line);
                    flUp = true;
                    break;
                case "resolution":
                    resolution = parseVec2(line);
                    flResolution = true;
                    break;
                case "fov":
                    fov = parseVec2(line);
                    flFov = true;
                    break;
                case "f":
                    focalDistance = Float.valueOf(keyValue[1]);
                    flF = true;
                    break;
                default:
                    throw new IllegalArgumentException("CAMFILE::UNKNOWN_KEY " + keyValue[0]);
            }
        }
        if (!(flPosition && flView && flUp && flResolution && flFov && flF)) {
            throw new IllegalArgumentException("CAMFILE::MISSING_KEY");
        }

        return new Camera(position, view, up, resolution, fov, focalDistance);
    }

    /*
     * Get the camera matrix
     */
    Matrix3f getIntrinsicMatrix() {
        if (k == null) {
            float cx = resolution.x / 2.0f;
            float cy = resolution.y / 2.0f;
            float fx = (float) (resolution.x / Math.tan(fov.x / 2.0f));
            float fy = (float) (resolution.y / Math.tan(fov.y / 2.0f));
            float skew = 0.0f;

            k = new Matrix3f(fx, skew, cx, 0.0f, fy, cy, 0.0f, 0.0f, 1.0f);
        }
        return k;
    }

    /**
     * Based on https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluLookAt.xml
     */
    public Matrix3f getRotationMatrix() {
        if (rotation == null) {
            Vector3f forward = view.minus(position);
            forward = forward.normalized();

            Vector3f upx = up.normalized();
            Vector3f s = forward.cross(upx);
            s = s.normalized();

            Vector3f u = s.cross(forward);

            rotation = new Matrix3f(s.x, u.x, -forward.x,
                    s.y, u.y, -forward.y,
                    s.z, u.z, -forward.z);
        }
        return rotation;
    }

    public Vector3f getTranslationVector() {
        if (translation == null) {
            translation = getRotationMatrix().times(position).times(-1);
        }
        return translation;
    }

    private class CamCoordSystem {
        Vector3f origin;
        Vector3f n;
        Vector3f u;
        Vector3f v;
        Vector3f imagePlaneOrigin;
        Vector2f imagePlaneDimensions;

    }

    ;

    /**
     * Construct an eye coordinate system
     * Input: camera position, center of interest, view-up vector
     * Returns: new origin and three basis vectors
     * <p>
     * /|
     * / |
     * /  |
     * /   |
     * / ^  |
     * / v|  |
     * |  |-----> n
     * | /  /
     * |Lu /
     * |  /
     * | /
     * |/
     */
    private CamCoordSystem constructCameraCoordinateSystem() {
        CamCoordSystem ccs = new CamCoordSystem();

        // n is normal to image plane, points in opposite direction of view point
        Vector3f N = new Vector3f(position.x - view.x,
                position.y - view.y,
                position.z - view.z);
        ccs.n = N.normalized();

        // u is a vector that is perpendicular to the plane spanned by
        // N and view up vector (cam->up)
        Vector3f U = up.cross(ccs.n);
        ccs.u = U.normalized();

        // v is a vector perpendicular to N and U
        ccs.v = ccs.n.cross(ccs.u);

        // origin is cam centre
        ccs.origin = position;
        return ccs;
    }


    private void constructImagePlaneOrigin(CamCoordSystem ccs) {
        float imagePlaneHeight = (float) (Math.tan(fov.y * 0.5f * (Math.PI / 180)) * 2.0f * focalDistance);
        float imagePlaneWidth = (float) (Math.tan(fov.x * 0.5f * (Math.PI / 180)) * 2.0f * focalDistance);

        Vector3f imagePlaneCentre = position.minus((ccs.n.times(focalDistance)));

        ccs.imagePlaneOrigin = imagePlaneCentre.minus((ccs.u.times(imagePlaneWidth * 0.5f)).minus(ccs.v.times(imagePlaneHeight * 0.5f)));
        ccs.imagePlaneDimensions = new Vector2f(imagePlaneWidth, imagePlaneHeight);
    }

    /*
     * Compute the backprojection of a point from X,Y and depth plus camera
     */
    public Vector3f backproject(int pixelX, int pixelY, float depth) {
        CamCoordSystem ccs = constructCameraCoordinateSystem();

        constructImagePlaneOrigin(ccs);

        // Compute pixel coordinate in world space
        float pixelWidth = ccs.imagePlaneDimensions.x / resolution.x;
        float pixelHeight = ccs.imagePlaneDimensions.y / resolution.y;
        Vector3f pixelCoordinate = ccs.imagePlaneOrigin
                .plus(ccs.u.times((pixelX + 0.5f) * pixelWidth))
                .plus(ccs.v.times((pixelY + 0.5f) * pixelHeight));

        Vector3f rayDirection = (pixelCoordinate.minus(ccs.origin)).normalized();
        ;
        return position.plus(rayDirection.times(depth));
    }

    public String toString() {
        return String.format("pos : (%.3f, %.3f, %.3f)\n vew : (%.3f, %.3f, %.3f)\n up : (%.3f, %.3f, %.3f)\nres : (%.3f, %.3f)\nfov : (%.3f, %.3f)\nfoc : %f\n",
                position.x, position.y, position.z,
                view.x, view.y, view.z,
                up.x, up.y, up.z,
                resolution.x, resolution.y,
                fov.x, fov.y, focalDistance);
    }
}
