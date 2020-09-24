// Getting started with OpenCL tutorial
// by Sam Lapere, 2016, http://raytracey.blogspot.com
// Code based on http://simpleopencl.blogspot.com/2013/06/tutorial-simple-start-with-opencl-and-c.html

/*
 * Usage dat_gen file.obj <optional_cam_file> <optional_suffix>
 */

#include "model_cl.hpp"
#include "utility.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cmath>
#include <Camera/Camera.h>

#include <GeomFileUtils/ObjFileParser.h>

const std::string CAMERA_FILE = "camera.txt";

void saveDepthImage(const std::string &fileName,
                    unsigned int width,
                    unsigned int height,
                    cl_float *data) {
    using namespace std;

    cout << "Saving depth image" << endl;
    std::ofstream saveFile{fileName, std::ios::out};
    std::size_t idx = 0;
    for (unsigned int row = 0; row < height; row++) {
        for (unsigned int col = 0; col < width; col++) {
            saveFile << data[idx] << " ";
            idx++;
        }
        saveFile << endl;
    }
    saveFile.close();
    cout << "...done" << endl;
}


template<typename Func>
void saveImage(const std::string &fileName, unsigned int width, unsigned int height, Func dataFunction, int maxValue) {
    using namespace std;

    std::ofstream saveFile{fileName, std::ios::out};
    saveFile << "P2" << endl;
    saveFile << width << " " << height << endl;

    if (maxValue == 0) {
        for (int i = 0; i < height * width; i++) {
            if (dataFunction(i) > maxValue) {
                maxValue = dataFunction(i);
            }
        }
    }
    saveFile << maxValue << endl;

    // loop over pixels, write greyscale values
    int i = 0;
    for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w++) {
            int v = dataFunction(i);
#ifdef DEBUG_FILE_WRITE
            if ( i % 10000 == 0 ) cout << "i: " << i << ", data[i] : " << v << endl;
#endif
            saveFile << v << " ";
            i++;
        }
        saveFile << endl;
    }
    saveFile.close();
}

void loadMesh(const std::string &filename,
              cl_float3 **cpuVertices,
              cl_int3 **cpuFaces,
              cl_float3 **cpuFaceNormals,
              unsigned int *pNumVertices,
              unsigned int *pNumFaces) {

    using namespace std;
    using namespace Eigen;

    animesh::ObjFileParser parser;
    pair<vector<Eigen::Vector3f>, vector<pair<vector<std::size_t>, Vector3f>>> object_data = parser.parse_file_raw_with_normals(
            filename);

    unsigned int numVertices = object_data.first.size();
    unsigned int numFaces = object_data.second.size();

    *cpuVertices = new cl_float3[numVertices];
    *cpuFaces = new cl_int3[numFaces];
    *cpuFaceNormals = new cl_float3[numFaces];

    for (int i = 0; i < numVertices; ++i) {
        (*cpuVertices)[i] = cl_float3{
                object_data.first[i].x(),
                object_data.first[i].y(),
                object_data.first[i].z()};
    }
    for (int i = 0; i < numFaces; ++i) {
        pair<vector<std::size_t>, Vector3f> face_data = object_data.second[i];
        vector<std::size_t> face_vertex_indices = face_data.first;
        Vector3f face_normal = face_data.second;

        (*cpuFaces)[i] = cl_int3{
                (int) face_vertex_indices[0],
                (int) face_vertex_indices[1],
                (int) face_vertex_indices[2]};

        (*cpuFaceNormals)[i] = cl_float3{
                face_normal.x(),
                face_normal.y(),
                face_normal.z()};
    }
    *pNumVertices = numVertices;
    *pNumFaces = numFaces;
}


// cl::Kernel makeKernel( cl::Program& program, std::string& name) {
// 	using namespace std;
// 	using namespace cl;

// 	// Create a kernel (entry point in the OpenCL source program)
// 	// kernels are the basic units of executable code that run on the OpenCL device
// 	// the kernel forms the starting point into the OpenCL program, analogous to main() in CPU code
// 	// kernels can be called from the host (CPU)
// 	cl_int err;
// 	Kernel kernel{program, name, &err};
// 	if ( err != CL_SUCCESS) {
// 		cerr << "Failed to create kernel " << err << endl;
// 		throw runtime_error( "Failed to create kernel");
// 	}
// 	cout << "Kernel created" << endl;
// 	return kernel;
// }

int main(int argc, char *argv[]) {
    using namespace std;
    using namespace cl;

    if (argc < 2) {
        cerr << "ERROR::NOMODEL" << endl;
        exit(-1);
    } else if (argc > 4) {
        cerr << "ERROR::INVALID_ARGS" << endl;
    }

    string model_filename = argv[1];

    string camera_filename = (argc > 2)
                             ? argv[2]
                             : CAMERA_FILE;

    string suffix = (argc > 3)
                    ? argv[3]
                    : "";

    Platform platform = selectPlatform();
    Device device = selectDevice(platform);
    Context context{device};

    // Create an OpenCL program by performing runtime source compilation
    string kernelSource = loadCode("ray_trace.cl");
    Program program{context, kernelSource};

    // Build the program and check for compilation errors (exit on fail)
    buildProgram(program, device);

    cl_int err;
    Kernel kernel{program, "ray_trace", &err};
    if (err != CL_SUCCESS) {
        cerr << "Failed to create kernel " << err << endl;
        throw runtime_error("Failed to create kernel");
    }
    cout << "Kernel created" << endl;

    // Create data arrays on the host (= CPU)
    Camera cpuCamera = loadCameraFromFile(camera_filename);
    cout << "Camera loaded" << endl;


    const unsigned int numElements = cpuCamera.resolution().x() * cpuCamera.resolution().y();
    auto *cpuDepthData = new cl_float[numElements];
    auto *cpuVertexData = new cl_int[numElements];
    cl_float3 *cpuVertices;
    cl_int3 *cpuFaces;
    cl_float3 *cpuFaceNormals;
    unsigned int numVertices;
    unsigned int numFaces;

    loadMesh(model_filename, &cpuVertices, &cpuFaces, &cpuFaceNormals, &numVertices, &numFaces);
    cout << "Mesh loaded " << numVertices << "v " << numFaces << "f" << endl;

    // Create buffers (memory objects) on the OpenCL device, allocate memory and copy input data to device.
    // Flags indicate how the buffer should be used e.g. read-only, write-only, read-write
    Buffer gpuDepthBuffer{context, CL_MEM_WRITE_ONLY, numElements * sizeof(cl_float)};
    Buffer gpuVertexBuffer{context, CL_MEM_WRITE_ONLY, numElements * sizeof(cl_int)};
    Buffer gpuVertices{context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, numVertices * sizeof(cl_float3), cpuVertices};
    Buffer gpuFaces{context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, numFaces * sizeof(cl_int3), cpuFaces};
    Buffer gpuFaceNormals{context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, numFaces * sizeof(cl_float3),
                          cpuFaceNormals};
    delete[] cpuVertices;
    delete[] cpuFaces;
    delete[] cpuFaceNormals;

    // Copy the CPU camera into GPU data structures - adding padding as needed to handle the fact that
    // cl_float3 are 16 bytes long
    struct {
        cl_float3 position;
        cl_float3 view;
        cl_float3 up;
        cl_float2 resolution;
        cl_float2 fov;
        cl_float focalDistance;
    } gpu_cam;

    memcpy((void *) &(gpu_cam.position), cpuCamera.origin().data(), 3 * sizeof(float));
    memcpy((void *) &(gpu_cam.view), cpuCamera.look_at().data(), 3 * sizeof(float));
    memcpy((void *) &(gpu_cam.up), cpuCamera.up().data(), 3 * sizeof(float));
    memcpy((void *) &(gpu_cam.resolution), cpuCamera.resolution().data(), 2 * sizeof(float));
    memcpy((void *) &(gpu_cam.fov), cpuCamera.field_of_view().data(), 2 * sizeof(float));
    gpu_cam.focalDistance = cpuCamera.focal_length();
    Buffer gpuCamera{context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(gpu_cam), (void *) &gpu_cam};

    // Specify the arguments for the OpenCL kernel
    //
    kernel.setArg(0, numVertices);
    kernel.setArg(1, gpuVertices);            // Vertices of mesh
    kernel.setArg(2, numFaces);
    kernel.setArg(3, gpuFaces);                // Faces of mesh as 3 vertex indices CCW
    kernel.setArg(4, gpuFaceNormals);        // Faces of mesh as 3 vertex indices CCW
    kernel.setArg(5, gpuCamera);            // Camera
    kernel.setArg(6, gpuDepthBuffer);        // Depth image rendered to here
    kernel.setArg(7, gpuVertexBuffer);        // Vertex image rendered to here
    kernel.setArg(8, (int)cpuCamera.resolution().x());                // Width of output images
    kernel.setArg(9, (int)cpuCamera.resolution().y());                // Height of output images

    // Create a command queue for the OpenCL device
    // the command queue allows kernel execution commands to be sent to the device
    CommandQueue queue = CommandQueue(context, device);

    // Determine the global and local number of "work items"
    // The global work size is the total number of work items (threads) that execute in parallel
    // Work items executing together on the same compute unit are grouped into "work groups"
    // The local work size defines the number of work items in each work group
    // Important: global_work_size must be an integer multiple of local_work_size
    std::size_t global_work_size = numElements;
    std::size_t local_work_size = 32; // could also be 1, 2 or 5 in this example

    // Launch the kernel and specify the global and local number of work items (threads)
    err = queue.enqueueNDRangeKernel(kernel, NULL, global_work_size, local_work_size);
    if (err != CL_SUCCESS) {
        cerr << "Failed to enqueue kernel " << err << endl;
    }

    // Read and copy Depth data back to CPU and save
    // the "CL_TRUE" flag blocks the read operation until all work items have finished their computation
    err = queue.enqueueReadBuffer(gpuDepthBuffer, CL_TRUE, 0, numElements * sizeof(cl_float), cpuDepthData);
    if (err != CL_SUCCESS) {
        cerr << "Failed to read buffer " << err << endl;
    }
    err = queue.enqueueReadBuffer(gpuVertexBuffer, CL_TRUE, 0, numElements * sizeof(cl_int), cpuVertexData);
    if (err != CL_SUCCESS) {
        cerr << "Failed to read buffer " << err << endl;
    }
    cout << "Kernel finished" << endl;

    // export as text format float file
    string depthFileName = "depth" + suffix + ".mat";
    saveDepthImage(depthFileName,
                   cpuCamera.resolution().x(),
                   cpuCamera.resolution().y(),
                   cpuDepthData);

    depthFileName = "depth" + suffix + ".pgm";
    saveImage(depthFileName,
              cpuCamera.resolution().x(),
              cpuCamera.resolution().y(),
              [cpuDepthData](int i) {
                  float depth = cpuDepthData[i];
                  if (isinf(depth)) return 0;
                  return (int) (depth * 255);
              }, 0);

    string vertexFileName = "vertex" + suffix + ".pgm";
    saveImage(vertexFileName,
              cpuCamera.resolution().x(),
              cpuCamera.resolution().y(),
              [cpuVertexData](int i) {
                  return cpuVertexData[i];
              }, 0);

    delete[] cpuDepthData;
    delete[] cpuVertexData;
}