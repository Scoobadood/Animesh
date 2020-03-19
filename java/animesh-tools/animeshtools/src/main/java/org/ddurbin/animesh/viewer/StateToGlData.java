package org.ddurbin.animesh.viewer;

import com.google.common.collect.ImmutableList;
import org.ddurbin.animesh.tools.State;
import org.ddurbin.animesh.tools.StateUtilities;
import org.ddurbin.common.Camera;
import org.ddurbin.common.Pair;
import org.ddurbin.common.Vector3f;

import java.io.IOException;
import java.util.List;

public class StateToGlData {
  private static final float NORM_SCALE = 0.05f;
  private static final float TAN_SCALE = 0.05f;

  public static float[] convertStateToGlData(State state, int frame) {
    ImmutableList.Builder<Float> listBuilder = ImmutableList.builder();

    Camera camera;
    try {
      camera = Camera.loadFromFile(String.format("camera_F%02d.txt", frame));
    } catch (IOException e) {
      throw new RuntimeException("Couldn't load camera file");
    }

    for (State.Surfel s : state.surfels) {
      s.frameData.forEach((fd) -> {
        if (fd.pixelInFrame.frameIndex == frame) {
          // Get tangent and normal
          Pair<Vector3f, Vector3f> p = StateUtilities.projectSurfelToFrame(s, fd);
          Vector3f pointInSpace = camera.toWorldCoordinates((int)fd.pixelInFrame.x, (int)fd.pixelInFrame.y, fd.depth);

            Vector3f normend = pointInSpace.plus(p.second);
            System.out.println(String.format("%4.4f, %4.4f, %4.4f, %4.4f, %4.4f, %4.4f;", pointInSpace.x, pointInSpace.y, pointInSpace.z, normend.x, normend.y, normend.z));

          // Get the point in 3-space
          listBuilder.add(pointInSpace.x);
          listBuilder.add(pointInSpace.y);
          listBuilder.add(pointInSpace.z);

          // Push normal
          listBuilder.add(pointInSpace.x + p.second.x * NORM_SCALE);
          listBuilder.add(pointInSpace.y + p.second.y * NORM_SCALE);
          listBuilder.add(pointInSpace.z + p.second.z * NORM_SCALE);

          // Get the point in 3-space
          listBuilder.add(pointInSpace.x);
          listBuilder.add(pointInSpace.y);
          listBuilder.add(pointInSpace.z);

          // Primary tangent as given
          listBuilder.add(pointInSpace.x + p.first.x * TAN_SCALE);
          listBuilder.add(pointInSpace.y + p.first.y * TAN_SCALE);
          listBuilder.add(pointInSpace.z + p.first.z * TAN_SCALE);

          // Do opposite tangent
          listBuilder.add(pointInSpace.x);
          listBuilder.add(pointInSpace.y);
          listBuilder.add(pointInSpace.z);
          listBuilder.add(pointInSpace.x - p.first.x * TAN_SCALE);
          listBuilder.add(pointInSpace.y - p.first.y * TAN_SCALE);
          listBuilder.add(pointInSpace.z - p.first.z * TAN_SCALE);

          // And now do perpendicular tangents
          // axis is k (normal), vector is v
          // k x v + k(k.v)
          Vector3f kCrossV = p.second.cross(p.first);
          float kDotV = p.second.dot(p.first);
          Vector3f kkv = p.second.times(kDotV);
          Vector3f tan90 = kCrossV.plus(kkv);

          listBuilder.add(pointInSpace.x - tan90.x * TAN_SCALE);
          listBuilder.add(pointInSpace.y - tan90.y * TAN_SCALE);
          listBuilder.add(pointInSpace.z - tan90.z * TAN_SCALE);
          listBuilder.add(pointInSpace.x + tan90.x * TAN_SCALE);
          listBuilder.add(pointInSpace.y + tan90.y * TAN_SCALE);
          listBuilder.add(pointInSpace.z + tan90.z * TAN_SCALE);
        }
      });
    }
    List<Float> lf = listBuilder.build();
    float[] f = new float[lf.size()];
    float minX = Float.MAX_VALUE;
    float minY = Float.MAX_VALUE;
    float minZ = Float.MAX_VALUE;
    float maxX = -Float.MAX_VALUE;
    float maxY = -Float.MAX_VALUE;
    float maxZ = -Float.MAX_VALUE;
    for (int i = 0; i < lf.size(); i++) {
      f[i] = lf.get(i);
      switch( i%3) {
        case 0:
          minX = Math.min(minX, f[i]);
          maxX = Math.max(maxX, f[i]);
          break;
        case 1:
          minY = Math.min(minY, f[i]);
          maxY = Math.max(maxY, f[i]);
          break;
        case 2:
          minZ = Math.min(minZ, f[i]);
          maxZ = Math.max(maxZ, f[i]);
          break;
      }
    }
    System.out.println( String.format("X [%.3f -> %.3f]", minX, maxX));
    System.out.println( String.format("Y [%.3f -> %.3f]", minY, maxY));
    System.out.println( String.format("Z [%.3f -> %.3f]", minZ, maxZ));
    float scale = Math.max( Math.abs(minX), Math.abs(maxX));
    scale = Math.max( scale, Math.max( Math.abs(minY), Math.abs(maxY)));
    scale = Math.max( scale, Math.max( Math.abs(minZ), Math.abs(maxZ)));
    for (int i = 0; i < f.length; i++) {
      f[i] /= scale;
    }
    return f;
  }
}
