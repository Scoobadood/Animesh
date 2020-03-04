package org.ddurbin.animesh.viewer;

public class DefaultSurfelColourer extends AbstractSurfelColourer {
    // Default colours
    private Colour normalColour = new Colour( 1.0f, 0.0f, 0.0f );

    DefaultSurfelColourer() {
        super( );
        this.normalColour = new Colour( 1.0f, 0.0f, 0.0f);
    }

    public DefaultSurfelColourer( Colour normalColour, Colour primaryTangentColour, Colour secondaryTangentColour) {
        super( primaryTangentColour, secondaryTangentColour);
        this.normalColour = normalColour;
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
            int lineIndex = i * linesPerSurfel;
            if(normalsEnabled) {
                setLineColourAtIndex(colours, lineIndex, normalColour);
            }
            if( tangentsEnabled) {
                if (principalTangentEnabled) {
                    setLineColourAtIndex(colours, lineIndex + 1, primaryTangentColour);
                } else {
                    setLineColourAtIndex(colours, lineIndex + 1, secondaryTangentColour);
                }
                setLineColourAtIndex(colours, lineIndex + 2, secondaryTangentColour);
                setLineColourAtIndex(colours, lineIndex + 3, secondaryTangentColour);
            }
        }
        return colours;
    }
}
