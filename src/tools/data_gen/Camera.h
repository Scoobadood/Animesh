#include <string>

#ifdef __APPLE__
#include "cl.hpp"
#else
#include <CL/cl.hpp>
#endif

typedef struct Camera {
	cl_float3 position;
	cl_float3 view;
	cl_float3 up;
	cl_float2 resolution;
	cl_float2 fov;
	cl_float focalDistance;
} Camera;

Camera loadCameraFromFile( const std::string& filename);
