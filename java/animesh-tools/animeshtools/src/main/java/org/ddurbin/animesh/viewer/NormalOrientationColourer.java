package org.ddurbin.animesh.viewer;

public class NormalOrientationColourer extends AbstractOrientationColourer {
    public NormalOrientationColourer( float[] vertices) {
        super(vertices);
    }

    public NormalOrientationColourer( float[] vertices, Colour primaryTangentColour, Colour secondaryTangentColour) {
        super( vertices, primaryTangentColour, secondaryTangentColour);
    }
    /**
     *
     */
    public float[] generateColoursForSurfels(int[] surfelIndices, boolean normalsEnabled, boolean tangentsEnabled, boolean principalTangentEnabled) {
        int numSurfels = surfelIndices.length;
        // Compute size of each item (surfel) in vertices
        int verticesPerSurfel = (normalsEnabled ? Constants.VERTICES_FOR_NORMAL : 0) + (tangentsEnabled ? Constants.VERTICES_FOR_TANGENTS : 0);
        int linesPerSurfel = verticesPerSurfel / 2;
        if( verticesPerSurfel == 0 ) return new float[0];

        float[] colours = new float[ verticesPerSurfel * numSurfels * Constants.NUM_COLOUR_PLANES];

        for( int i=0; i<numSurfels; i++ ) {
            Colour c = colourForSurfel(surfelIndices[i]);
            int lineIndex = i * linesPerSurfel;
            if(normalsEnabled) {
                setLineColourAtIndex(colours, lineIndex++, c);
            }
            if( tangentsEnabled) {
                if( principalTangentEnabled) {
                    setLineColourAtIndex(colours, lineIndex++, primaryTangentColour);
                } else {
                    setLineColourAtIndex(colours, lineIndex++, secondaryTangentColour);
                }
                setLineColourAtIndex(colours, lineIndex++, secondaryTangentColour);
                setLineColourAtIndex(colours, lineIndex++, secondaryTangentColour);
            }
        }
        return colours;
    }
}
