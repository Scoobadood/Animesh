#pragma once

#include <vector>
#include <string>

#ifdef __APPLE__
#include "cl.hpp"
#else
// Linux OpenCL
#include <CL/cl.hpp>
#endif

// Mesh classes
struct Vertex {
    cl_float3 position;
    cl_float3 normal;
};

class Mesh {
    public:
        /*  Mesh Data  */
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        /*  Functions  */
        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
};