package org.ddurbin.common;

import org.ddurbin.animesh.tools.State;

import java.util.Objects;

/**
 * A 3D vector.
 */
public class Vector2f {
    public final float x;
    public final float y;

    public Vector2f(float x, float y) {
        this.x = x;
        this.y = y;
    }

    /*
     * Compute the dot product of this vector with another
     */
    public float dot(Vector2f other) {
        return this.x * other.x + this.y * other.y;
    }

    /*
     * Multiply by a scalar
     */
    public Vector2f times(float scalar) {
        return new Vector2f(
                this.x * scalar,
                this.y * scalar
        );
    }

    /*
     * Add another vec 3
     */
    public Vector2f plus(Vector2f other) {
        return new Vector2f(
                this.x + other.x,
                this.y + other.y
        );
    }

    /**
     * Subtract another vec 3
     */
    public Vector2f minus(Vector2f other) {
        return new Vector2f(
                this.x - other.x,
                this.y - other.y
        );
    }

    /*
     * Return a normalized version of this vector
     */
    public Vector2f normalized() {
        double scale = this.x * this.x + this.y * this.y;
        if (scale == 1) {
            return this;
        }

        scale = Math.sqrt(scale);
        return new Vector2f(
                (float) (this.x / scale),
                (float) (this.y / scale));
    }

    /**
     * Compute the angle between two vectors
     */
    public float angleWith(Vector2f otherVector) {
        Vector2f thisUnitVector = this.normalized();
        Vector2f otherUnitVector = otherVector.normalized();
        // dp = |a|.|b|.cos(theta)
        //    = cos(theta) in this case

        double dotProduct = thisUnitVector.dot(otherUnitVector);
        dotProduct = Math.min(1.0, dotProduct);
        double angle = Math.acos(dotProduct);
        return (float) Math.toDegrees(angle);
    }

    public boolean equals(Object otherObject) {
        if (otherObject == null) {
            return false;
        }
        if (this == otherObject) {
            return true;
        }
        if (!(otherObject instanceof Vector2f)) {
            return false;
        }
        Vector2f otherVector = (Vector2f) otherObject;
        return ((x == otherVector.x)
                && (y == otherVector.y));
    }

    public int hashCode() {
        return Objects.hash(x, y);
    }

}
