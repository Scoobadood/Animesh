package org.ddurbin.animesh.viewer;

import java.nio.FloatBuffer;

public class MatrixHelper {
  /*
   * Multiply Left by Right giving Result.
   * Matrices are assumed to be column major
   */
  public static void glMultMatrixf(FloatBuffer left, FloatBuffer right, FloatBuffer result) {
    final int leftPosition = left.position();
    final int rightPosition = right.position();
    final int resultPosition = result.position();
    result.put(resultPosition + 0,
        (left.get(leftPosition + 0) * right.get(rightPosition + 0))
            + (left.get(leftPosition + 4) * right.get(rightPosition + 1))
            + (left.get(leftPosition + 8) * right.get(rightPosition + 2))
            + (left.get(leftPosition + 12) * right.get(rightPosition + 3)));
    result.put(resultPosition + 4,
        (left.get(leftPosition + 0) * right.get(rightPosition + 4))
            + (left.get(leftPosition + 4) * right.get(rightPosition + 5))
            + (left.get(leftPosition + 8) * right.get(rightPosition + 6))
            + (left.get(leftPosition + 12) * right.get(rightPosition + 7)));
    result.put(resultPosition + 8,
        (left.get(leftPosition + 0) * right.get(rightPosition + 8))
            + (left.get(leftPosition + 4) * right.get(rightPosition + 9))
            + (left.get(leftPosition + 8) * right.get(rightPosition + 10))
            + (left.get(leftPosition + 12) * right.get(rightPosition + 11)));
    result.put(resultPosition + 12,
        (left.get(leftPosition + 0) * right.get(rightPosition + 12))
            + (left.get(leftPosition + 4) * right.get(rightPosition + 13))
            + (left.get(leftPosition + 8) * right.get(rightPosition + 14))
            + (left.get(leftPosition + 12) * right.get(rightPosition + 15)));

    result.put(resultPosition + 1,
        (left.get(leftPosition + 1) * right.get(rightPosition + 0))
            + (left.get(leftPosition + 5) * right.get(rightPosition + 1))
            + (left.get(leftPosition + 9) * right.get(rightPosition + 2))
            + (left.get(leftPosition + 13) * right.get(rightPosition + 3)));
    result.put(resultPosition + 5,
        (left.get(leftPosition + 1) * right.get(rightPosition + 4))
            + (left.get(leftPosition + 5) * right.get(rightPosition + 5))
            + (left.get(leftPosition + 9) * right.get(rightPosition + 6))
            + (left.get(leftPosition + 13) * right.get(rightPosition + 7)));
    result.put(resultPosition + 9,
        (left.get(leftPosition + 1) * right.get(rightPosition + 8))
            + (left.get(leftPosition + 5) * right.get(rightPosition + 9))
            + (left.get(leftPosition + 9) * right.get(rightPosition + 10))
            + (left.get(leftPosition + 13) * right.get(rightPosition + 11)));
    result.put(resultPosition + 13,
        (left.get(leftPosition + 1) * right.get(rightPosition + 12))
            + (left.get(leftPosition + 5) * right.get(rightPosition + 13))
            + (left.get(leftPosition + 9) * right.get(rightPosition + 14))
            + (left.get(leftPosition + 13) * right.get(rightPosition + 15)));

    result.put(resultPosition + 2,
        (left.get(leftPosition + 2) * right.get(rightPosition + 0))
            + (left.get(leftPosition + 6) * right.get(rightPosition + 1))
            + (left.get(leftPosition + 10) * right.get(rightPosition + 2))
            + (left.get(leftPosition + 14) * right.get(rightPosition + 3)));
    result.put(resultPosition + 6,
        (left.get(leftPosition + 2) * right.get(rightPosition + 4))
            + (left.get(leftPosition + 6) * right.get(rightPosition + 5))
            + (left.get(leftPosition + 10) * right.get(rightPosition + 6))
            + (left.get(leftPosition + 14) * right.get(rightPosition + 7)));
    result.put(resultPosition + 10,
        (left.get(leftPosition + 2) * right.get(rightPosition + 8))
            + (left.get(leftPosition + 6) * right.get(rightPosition + 9))
            + (left.get(leftPosition + 10) * right.get(rightPosition + 10))
            + (left.get(leftPosition + 14) * right.get(rightPosition + 11)));
    result.put(resultPosition + 14,
        (left.get(leftPosition + 2) * right.get(rightPosition + 12))
            + (left.get(leftPosition + 6) * right.get(rightPosition + 13))
            + (left.get(leftPosition + 10) * right.get(rightPosition + 14))
            + (left.get(leftPosition + 14) * right.get(rightPosition + 15)));

    result.put(resultPosition + 3,
        (left.get(leftPosition + 3) * right.get(rightPosition + 0))
            + (left.get(leftPosition + 7) * right.get(rightPosition + 1))
            + (left.get(leftPosition + 11) * right.get(rightPosition + 2))
            + (left.get(leftPosition + 15) * right.get(rightPosition + 3)));
    result.put(resultPosition + 7,
        (left.get(leftPosition + 3) * right.get(rightPosition + 4))
            + (left.get(leftPosition + 7) * right.get(rightPosition + 5))
            + (left.get(leftPosition + 11) * right.get(rightPosition + 6))
            + (left.get(leftPosition + 15) * right.get(rightPosition + 7)));
    result.put(resultPosition + 11,
        (left.get(leftPosition + 3) * right.get(rightPosition + 8))
            + (left.get(leftPosition + 7) * right.get(rightPosition + 9))
            + (left.get(leftPosition + 11) * right.get(rightPosition + 10))
            + (left.get(leftPosition + 15) * right.get(rightPosition + 11)));
    result.put(resultPosition + 15,
        (left.get(leftPosition + 3) * right.get(rightPosition + 12))
            + (left.get(leftPosition + 7) * right.get(rightPosition + 13))
            + (left.get(leftPosition + 11) * right.get(rightPosition + 14))
            + (left.get(leftPosition + 15) * right.get(rightPosition + 15)));
  }

  /**
   * Helper to multiply matrix from floats array
   */
  public static float[] multiply(float[] a, float[] b) {
    float[] tmp = new float[16];
    glMultMatrixf(FloatBuffer.wrap(a), FloatBuffer.wrap(b), FloatBuffer.wrap(tmp));
    return tmp;
  }

  public static float[] translate(float[] m, float x, float y, float z) {
    float[] t = {1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        x, y, z, 1.0f};
    return multiply(t, m);
  }

  /**
   * Construct a rotation matrix for a rotation about an axis given in XYZ through a radians
   */
  public static float[] rotate(float[] m, float a, float x, float y, float z) {
    float s, c;
    s = (float) Math.sin(a);
    c = (float) Math.cos(a);
    float[] r = {
        x * x * (1.0f - c) + c, y * x * (1.0f - c) + z * s, x * z * (1.0f - c) - y * s, 0.0f,
        x * y * (1.0f - c) - z * s, y * y * (1.0f - c) + c, y * z * (1.0f - c) + x * s, 0.0f,
        x * z * (1.0f - c) + y * s, y * z * (1.0f - c) - x * s, z * z * (1.0f - c) + c, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f};
    return multiply(r, m);
  }

  public static void identity(float[] m) {
    assert (m != null);
    assert (m.length == 16);
    for (int i = 0; i < 16; i++) {
      m[i] = 0.0f;
    }
    m[0] = m[5] = m[10] = m[15] = 1.0f;
  }

  public static float[] invert(float[] m) {
    assert (m != null);
    assert (m.length == 16);
    float[] tmp = new float[16];
    for (int i = 0; i < 4; i++) {
      tmp[0 + i] = m[i * 4 + 0];
      tmp[4 + i] = m[i * 4 + 1];
      tmp[8 + i] = m[i * 4 + 2];
      tmp[12 + i] = m[i * 4 + 3];
    }
    return tmp;
  }

}
