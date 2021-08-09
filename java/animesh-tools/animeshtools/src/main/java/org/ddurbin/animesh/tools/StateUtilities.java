package org.ddurbin.animesh.tools;

import static org.ddurbin.animesh.tools.State.FrameData;
import static org.ddurbin.animesh.tools.State.Surfel;

import java.io.InputStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;
import org.ddurbin.common.Pair;
import org.ddurbin.common.Vector3f;

public class StateUtilities {
  /**
   * Use the transformation given in FrameData to project the given Surfel's normal and tangent
   * into the Frame's frame of reference
   */
  public static Pair<Vector3f, Vector3f>
  projectSurfelToFrame(Surfel s, FrameData fd) {
    // Transform the surfel tangent and normal into this frame space
    Vector3f tangentInFrame = fd.transform.times(s.tangent);
    Vector3f normalInFrame = fd.transform.times(Vector3f.Y_AXIS);
    return new Pair<>(tangentInFrame, normalInFrame);
  }

  /**
   * Load State from a file.
   */
  public static State
  loadState(String directory, String fileName) throws Exception {
    Path path = Paths.get(directory, fileName);
    InputStream in = Files.newInputStream(path, StandardOpenOption.READ);
    return State.read(in);
  }


}
