//
// Created by Dave Durbin on 7/3/20.
//

#ifndef ANIMESH_NORMALS_H
#define ANIMESH_NORMALS_H

typedef enum {
    NONE,
    DERIVED,
    NATURAL
} tNormal;

// A normal to the depth map
struct NormalWithType {
    tNormal type;
    float x;
    float y;
    float z;
    NormalWithType(tNormal t, float xx, float yy, float zz) : type{t}, x{xx}, y{yy}, z{zz} {};
};



#endif //ANIMESH_NORMALS_H
