//------------------------------------------------------------------
#version 410 core
//------------------------------------------------------------------
// Ray tracer ver: 1.000
//------------------------------------------------------------------
in vec3      rayStart;           // ray start position
in vec3      rayDirection;       // ray start direction

uniform sampler2D   sceneTexture;       // scene mesh data texture

out vec4 rgba;
// out int index;
//---------------------------------------------------------------------------
void main(void) {
    int index;
    // Scene texture is x, y, z, idx per vertex, repeated 
    // with vertices arranged in triangles
    const int sceneTextureSize = 512;
    vec2 textureCoord = vec2(0.0, 0.0);
    float deltaTexture = 1.0 / float(sceneTextureSize - 1);
    int rowCoordinate = 0;

    #define nextVertex texture(sceneTexture,textureCoord); \
        textureCoord.s += deltaTexture; \
        rowCoordinate++; \
        if (rowCoordinate == sceneTextureSize) { \
            rowCoordinate=0; \
            textureCoord.s=0.0; \
            textureCoord.t += deltaTexture; \
        }

    vec4 data =  nextVertex;

    int numTriangles = int(data.r);
    const float ZERO = 1e-6;   // to avoid intrsection with start point of ray

    // Default background
    index = (numTriangles * 3) + 1;

    /*
     * Read all triangles in scene
     * Each will be a vec3 (scene texture is RGB32F)
     * with one vertex in each of RG and B
     * make sure we wrap at end of each row.
     */
    for ( int tIndex = 0; tIndex < numTriangles; tIndex++ ) {
        vec4 vertex = nextVertex;
        vec3 v0 = vertex.xyz;
        int  i0 = int(vertex.w);

        vertex = nextVertex;
        vec3 v1 = vertex.xyz;
        int  i1 = int(vertex.w);

        vertex = nextVertex;
        vec3 v2 = vertex.xyz;
        int  i2 = int(vertex.w);

        // Check for intersection using 
        // Moller - Trumbore's algorithm
        vec3 edge1 = v1 - v0;
        vec3 edge2 = v2 - v0;

        // Check for ray parallel to triangle
        vec3 h = cross(rayDirection, edge2);
        float a = dot(edge1, h);
        if (abs(a) < 1e-8) {
            continue;
        }

        float f = 1.0 / a;
        vec3 s = rayStart - v0;
        float u = f * dot(s, h);
        if ((u < 0.0) || (u > 1.0)) {
            continue;
        }

        vec3 q = cross(s, edge1);
        float v = f * dot(rayDirection, q);
        if ((v < 0.0) || (u + v > 1.0)) {
            continue;
        }
        // Comput et to find intersection
        float t = f * dot(edge2, q);
        if ( t > ZERO) {
            vec3 intersection = rayStart + rayDirection * t;

            // Stash the nearest vertex
            float d0 = distance(intersection, v0);
            float d1 = distance(intersection, v1);
            float d2 = distance(intersection, v2);

            index = (d0 < d1 && d0 < d2 ) 
                ? i0 
                : (d1 < d0 && d1 < d2)
                    ? i1 
                    : i2;
        }
    }
    index = 99;
    rgba = vec4(0.4, 0.3, 0.2, 1.0);
}
//---------------------------------------------------------------------------
