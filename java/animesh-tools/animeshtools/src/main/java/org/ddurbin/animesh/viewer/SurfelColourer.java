package org.ddurbin.animesh.viewer;

public interface SurfelColourer {
    float[] generateColoursForSurfels( int[] surfelIndices, boolean normalsEnabled, boolean tangentsEnabled, boolean principalTangentEnabled);
}
