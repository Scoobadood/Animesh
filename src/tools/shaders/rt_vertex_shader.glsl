#version 330 core

// For each vertex, create a ray from that pixel through camera centre. storing
// orientation and origin.

uniform float aspect;
uniform float focal_length;

// perspective * view * world
uniform mat4x4 tm_eye;

// Each pixel has screen space coord
layout(location=0) in vec2 pos;

out smooth vec2 txt_pos;    // frag position on screen <-1,+1> for debug prints
out smooth vec3 ray_pos;    // ray start position
out smooth vec3 ray_dir;    // ray start direction
//------------------------------------------------------------------
void main(void) {
    vec4 p;
    txt_pos=pos;
    // perspective projection
    p=tm_eye*vec4(pos.x/aspect,pos.y,0.0,1.0);
    ray_pos=p.xyz;
    p-=tm_eye*vec4(0.0,0.0,-focal_length,1.0);
    ray_dir=normalize(p.xyz);

    gl_Position=vec4(pos,0.0,1.0);
}
//------------------------------------------------------------------