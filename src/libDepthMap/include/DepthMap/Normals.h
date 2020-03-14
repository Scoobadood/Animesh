//
// Created by Dave Durbin on 7/3/20.
//

#ifndef ANIMESH_NORMALS_H
#define ANIMESH_NORMALS_H

class DepthMap;

typedef enum {
    NONE,
    DERIVED,
    NATURAL
} tNormal;

typedef enum {
    CROSS,
    PCL,
} tNormalMethod;

// A normal to the depth map
struct NormalWithType {
    tNormal type;
    float x;
    float y;
    float z;
    NormalWithType(tNormal t, float xx, float yy, float zz) : type{t}, x{xx}, y{yy}, z{zz} {};
};

void
validate_normals(const DepthMap* depth_map);

#endif //ANIMESH_NORMALS_H
