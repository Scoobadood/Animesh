package org.ddurbin.animesh.viewer;

public abstract class OrientationSurfelColourer extends AbstractSurfelColourer {
    protected Colour[] normalColour;

    protected OrientationSurfelColourer( float[] vertices) {
        super( );
        generateNormalColours(vertices);
    }

    protected OrientationSurfelColourer( float[] vertices, Colour primaryTangentColour, Colour secondaryTangentColour) {
        super( primaryTangentColour, secondaryTangentColour);
        generateNormalColours(vertices);
    }

    private void generateNormalColours( float[] vertices) {
        int numSurfels = vertices.length / (Constants.VERTICES_FOR_FULL_SURFEL * 3);
        normalColour = new Colour[numSurfels * Constants.VERTICES_FOR_FULL_SURFEL];
        for( int i=0; i<numSurfels; i++ ) {
            int srcVertexIndex = i * Constants.VERTICES_FOR_FULL_SURFEL * 3;

            // Colour the normal based on the orientation
            float dx = vertices[srcVertexIndex + 3] - vertices[srcVertexIndex + 0];
            float dy = vertices[srcVertexIndex + 4] - vertices[srcVertexIndex + 1];
            float dz = vertices[srcVertexIndex + 5] - vertices[srcVertexIndex + 2];
            float size = (float) Math.sqrt(dx * dx + dy * dy + dz * dz);
            float nr = 0.5f + ((dx / size) * 0.5f);
            float ng = 0.5f + ((dy / size) * 0.5f);
            float nb = 0.5f + ((dz / size) * 0.5f);

            normalColour[i] = new Colour(nr, ng, nb);
        }
    }
}
