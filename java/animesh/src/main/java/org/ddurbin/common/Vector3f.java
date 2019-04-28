package org.ddurbin.common;

/**
 * A 3D vector.
 */
public class Vector3f {
    public final float x;
    public final float y;
    public final float z;

    /**
     * The Y-Axis
     */
    public static final Vector3f Y_AXIS = new Vector3f(0.0f, 1.0f, 0.0f);

    public Vector3f(float x, float y, float z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    /**
     * Compute the cross product of this vector with another
     */
    public Vector3f cross(Vector3f other) {
        return new Vector3f(
                this.y * other.z - this.z * other.y,
                this.z * other.x - this.x * other.z,
                this.x * other.y - this.y * other.x
        );
    }

    /*
     * Compute the dot product of this vector with another
     */
    public float dot(Vector3f other) {
        return this.x * other.x + this.y * other.y + this.z * other.z;
    }

    /*
     * Multiply by a scalar
     */
    public Vector3f times(float scalar) {
        return new Vector3f(
                this.x * scalar,
                this.y * scalar,
                this.z * scalar);
    }

    /*
     * Return a normalized version of this vector
     */
    public Vector3f normalized() {
        double scale = this.x * this.x + this.y * this.y + this.z * this.z;
        if( scale == 1 ) {
            return this;
        }

        scale = Math.sqrt(scale);
        return new Vector3f(
                (float) (this.x / scale),
                (float) (this.y / scale),
                (float) (this.z / scale));
    }

    /**
     * Compute the angle between two vectors
     */
    public float angleWith(Vector3f otherVector) {
        Vector3f thisUnitVector = this.normalized();
        Vector3f otherUnitVector = otherVector.normalized();
        // dp = |a|.|b|.cos(theta)
        //    = cos(theta) in this case

        double dotProduct = thisUnitVector.dot(otherUnitVector);
        dotProduct = Math.min(1.0, dotProduct);
        double angle = Math.acos(dotProduct);
        return (float)Math.toDegrees(angle);
    }
}
