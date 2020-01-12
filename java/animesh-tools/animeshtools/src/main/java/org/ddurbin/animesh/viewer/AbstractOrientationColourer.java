package org.ddurbin.animesh.viewer;

/*
 * Constructor extracts normal data and builds an array of Colours
 */

public abstract class AbstractOrientationColourer extends AbstractSurfelColourer {
    // One colour per Surfel
    private Colour[] colours;

    protected AbstractOrientationColourer(float[] vertices) {
        super( );
        generateColours(vertices);
    }

    protected AbstractOrientationColourer(float[] vertices, Colour primaryTangentColour, Colour secondaryTangentColour) {
        super( primaryTangentColour, secondaryTangentColour);
        generateColours(vertices);
    }

    protected Colour colourForSurfel( int i ) {
        return colours[i];
    }

    /**
     * Vertices  contains all vertices for all Surfels. These are:
     * 2 for Normal
     * 2 for Primary tangent
     * 2 for Secondarytangent
     * 2 for orthogonal tangents
     * We extract the Normal vector, unitise it and generat RGB values im the range 0 .. 1
     * @param vertices
     */
    private void generateColours(float[] vertices) {
        int floatsPerSurfel = Constants.VERTICES_FOR_FULL_SURFEL * 3;
        int numSurfels = vertices.length / floatsPerSurfel;
        colours = new Colour[numSurfels];
        for( int i=0; i<numSurfels; i++ ) {
            int srcVertexIndex = i * floatsPerSurfel;

            // Colour the normal based on the orientation
            float dx = vertices[srcVertexIndex + 3] - vertices[srcVertexIndex + 0];
            float dy = vertices[srcVertexIndex + 4] - vertices[srcVertexIndex + 1];
            float dz = vertices[srcVertexIndex + 5] - vertices[srcVertexIndex + 2];
            float size = (float) Math.sqrt(dx * dx + dy * dy + dz * dz);
            float nr = 0.5f + ((dx / size) * 0.5f);
            float ng = 0.5f + ((dy / size) * 0.5f);
            float nb = 0.5f + ((dz / size) * 0.5f);

            colours[i] = new Colour(nr, ng, nb);
        }
    }
}
