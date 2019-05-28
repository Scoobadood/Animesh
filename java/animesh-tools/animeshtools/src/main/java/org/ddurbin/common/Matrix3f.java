package org.ddurbin.common;

public class Matrix3f {
    private final float[] m;

    public Matrix3f( float m00, float m01, float m02,
                     float m10, float m11, float m12,
                     float m20, float m21, float m22)
    {
        m = new float[]{
            m00, m01, m02,
            m10, m11, m12,
            m20, m21, m22
        };
    }

    public Matrix3f( final float[] m )
    {
        this.m = m.clone();
    }

    /**
     * Multiply this Matrix by the given Vector
     * @param source
     * @return
     */
    public Vector3f times(Vector3f source)
    {
        return new Vector3f(
                m[0] * source.x + m[1] * source.y + m[2] * source.z,
                m[3] * source.x + m[4] * source.y + m[5] * source.z,
                m[6] * source.x + m[7] * source.y + m[8] * source.z
        );
    }
}
