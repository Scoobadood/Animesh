package org.ddurbin.animesh.viewer;

public interface Constants {
    int VERTICES_FOR_NORMAL = 2;
    int VERTICES_FOR_MAIN_TANGENT = 2;
    int VERTICES_FOR_SECONDARY_TANGENTS = 4;
    int VERTICES_FOR_TANGENTS = VERTICES_FOR_MAIN_TANGENT + VERTICES_FOR_SECONDARY_TANGENTS;
    int NUM_COLOUR_PLANES = 4;
    int VERTICES_FOR_FULL_SURFEL = VERTICES_FOR_NORMAL + VERTICES_FOR_TANGENTS;
    int BYTES_PER_FLOAT = 4;
    int FLOATS_FOR_NORMAL = VERTICES_FOR_NORMAL * 3;
    int FLOATS_FOR_TANGENTS = VERTICES_FOR_TANGENTS * 3;

}
