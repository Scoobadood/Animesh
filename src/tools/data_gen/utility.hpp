#include <string>

#ifdef __APPLE__
#include "cl.hpp"
#else /// your stuff for linux
#include <CL/cl.hpp> // main OpenCL include file 
#endif

/*
 * Load an OpenCL kernel
 */
std::string loadCode( const std::string& kernelPath );

cl::Platform selectPlatform();

cl::Device selectDevice(cl::Platform& platform);

void buildProgram( cl::Program& program, const cl::Device& device );
