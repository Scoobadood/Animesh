package org.ddurbin.animesh.viewer;

abstract class AbstractSurfelColourer implements SurfelColourer{
    protected Colour primaryTangentColour = new Colour( 0.0f, 1.0f, 0.0f );
    protected Colour secondaryTangentColour = new Colour( 1.0f, 1.0f, 1.0f );

    AbstractSurfelColourer( ) {
    }

    AbstractSurfelColourer( Colour primaryTangentColour, Colour secondaryTangentColour) {
        this.primaryTangentColour = primaryTangentColour;
        this.secondaryTangentColour = secondaryTangentColour;
    }

    protected void setVertexColourAtIndex( float[] colours, int vertexIndex, Colour colour ) {
        colours[vertexIndex*4 + 0] = colour.red;
        colours[vertexIndex*4 + 1] = colour.green;
        colours[vertexIndex*4 + 2] = colour.blue;
        colours[vertexIndex*4 + 3] = colour.alpha;
    }

    protected void setLineColourAtIndex( float[] colours, int lineIndex, Colour colour ) {
        setVertexColourAtIndex(colours, lineIndex * 2, colour);
        setVertexColourAtIndex(colours, lineIndex * 2 + 1, colour);
    }
}
