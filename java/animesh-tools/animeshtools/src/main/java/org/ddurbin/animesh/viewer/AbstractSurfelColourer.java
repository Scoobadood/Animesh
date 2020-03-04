package org.ddurbin.animesh.viewer;

abstract class AbstractSurfelColourer implements SurfelColourer{
    Colour primaryTangentColour = new Colour( 0.0f, 1.0f, 0.0f );
    Colour secondaryTangentColour = new Colour( 1.0f, 1.0f, 1.0f );

    AbstractSurfelColourer( ) {
    }

    AbstractSurfelColourer(Colour primaryTangentColour, Colour secondaryTangentColour) {
        this.primaryTangentColour = primaryTangentColour;
        this.secondaryTangentColour = secondaryTangentColour;
    }

    private void setVertexColourAtIndex(float[] colours, int vertexIndex, Colour colour) {
        int colourIndex = vertexIndex * Constants.NUM_COLOUR_PLANES;
        colours[colourIndex    ] = colour.red;
        colours[colourIndex + 1] = colour.green;
        colours[colourIndex + 2] = colour.blue;
        colours[colourIndex + 3] = colour.alpha;
    }

    void setLineColourAtIndex( float[] colours, int lineIndex, Colour colour ) {
        setVertexColourAtIndex(colours, lineIndex * 2, colour);
        setVertexColourAtIndex(colours, lineIndex * 2 + 1, colour);
    }
}
