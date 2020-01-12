package org.ddurbin.animesh.viewer;

public class DefaultSurfelColourer {
    private static final int VERTICES_FOR_NORMAL = 2;
    private static final int VERTICES_FOR_MAIN_TANGENT = 2;
    private static final int VERTICES_FOR_SECONDARY_TANGENTS = 4;
    private static final int VERTICES_FOR_TANGENTS = VERTICES_FOR_MAIN_TANGENT + VERTICES_FOR_SECONDARY_TANGENTS;
    private static final int NUM_COLOUR_PLANES = 4;

    // Default colours
    private static final Colour NORMAL_COLOUR = new Colour( 1.0f, 0.0f, 0.0f );
    private static final Colour PRIMARY_TANGENT_COLOUR = new Colour( 0.0f, 1.0f, 0.0f );
    private static final Colour ORTH_TANGENT_COLOUR = new Colour( 1.0f, 1.0f, 1.0f );

    /**
     *
     */
    public float[] generateVertexColours(int numSurfels, boolean normalsEnabled, boolean tangentsEnabled, boolean principalTangentEnabled) {
        // Compute size of each item (surfel) in vertices
        int verticesPerSurfel = (normalsEnabled ? VERTICES_FOR_NORMAL : 0) + (tangentsEnabled ? VERTICES_FOR_TANGENTS : 0);
        if( verticesPerSurfel == 0 ) return new float[0];

        // Number of colours depends on a what's on display
        int srcColourIndex = 0;
        int destColourIndex = 0;
        for( int i=0; i<numSurfels; i++ ) {
            if( normalsEnabled) {
                System.arraycopy(sourceColours, srcColourIndex, colours, destColourIndex, VERTICES_FOR_NORMAL * NUM_COLOUR_PLANES);
                destColourIndex += (VERTICES_FOR_NORMAL * NUM_COLOUR_PLANES);
            }
            srcColourIndex += (VERTICES_FOR_NORMAL * NUM_COLOUR_PLANES);

            if( tangentsEnabled) {
                if( principalTangentEnabled) {
                    System.arraycopy(sourceColours, srcColourIndex, colours, destColourIndex, VERTICES_FOR_MAIN_TANGENT * NUM_COLOUR_PLANES);
                    srcColourIndex += (VERTICES_FOR_MAIN_TANGENT * NUM_COLOUR_PLANES);
                } else {
                    srcColourIndex += (VERTICES_FOR_MAIN_TANGENT * NUM_COLOUR_PLANES);
                    System.arraycopy(sourceColours, srcColourIndex, colours, destColourIndex, VERTICES_FOR_MAIN_TANGENT * NUM_COLOUR_PLANES);
                }
                destColourIndex += (VERTICES_FOR_MAIN_TANGENT * NUM_COLOUR_PLANES);

                System.arraycopy(sourceColours, srcColourIndex, colours, destColourIndex, VERTICES_FOR_SECONDARY_TANGENTS * NUM_COLOUR_PLANES);
                srcColourIndex += (VERTICES_FOR_SECONDARY_TANGENTS * NUM_COLOUR_PLANES);
                destColourIndex += (VERTICES_FOR_SECONDARY_TANGENTS * NUM_COLOUR_PLANES);
            } else {
                srcColourIndex += (VERTICES_FOR_TANGENTS * NUM_COLOUR_PLANES);
            }
        }
        return colours;
    }

}
