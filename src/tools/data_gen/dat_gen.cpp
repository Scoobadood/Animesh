// Getting started with OpenCL tutorial
// by Sam Lapere, 2016, http://raytracey.blogspot.com
// Code based on http://simpleopencl.blogspot.com/2013/06/tutorial-simple-start-with-opencl-and-c.html

#include "model_cl.hpp"

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <math.h>
#include <Camera/Camera.h>

#ifdef __APPLE__
#include "cl.hpp"
#else /// your stuff for linux
#include <CL/cl.hpp> // main OpenCL include file 
#endif

#include <FileUtils/ObjFileParser.h>

#define DEFAULT_DEVICE 3

using namespace cl;

const std::string CAMERA_FILE = "camera.txt";

float inline deg2rad(float deg) {
	return (deg * M_PI) / 180.0;
}

float inline rad2deg(float rad) {
	return (rad * 180.0) / M_PI;
}

std::string loadCode( const std::string& kernelPath ) {
	using namespace std;

	string code;
	ifstream file;
	file.exceptions (ifstream::failbit | ifstream::badbit);
	try {
		file.open(kernelPath);
		stringstream stream;
		stream << file.rdbuf();
		file.close();
		code   = stream.str();
	}
	catch (ifstream::failure& e) {
		cout << "ERROR::FILE_NOT_SUCCESFULLY_READ" << endl;
	}
	return code;
}


Platform selectPlatform() {
	using namespace std;

	// Find all available OpenCL platforms (e.g. AMD, Nvidia, Intel)
	vector<Platform> platforms;
	Platform::get(&platforms);

	if ( platforms.empty() ) {
		cerr << "No OpenCL platform available" << endl;
		exit(-1);
	}

	Platform platform;
	if ( platforms.size() == 1 ) {
		platform = platforms[0];
	} else {
		// Show the names of all available OpenCL platforms
		cout << "Available OpenCL platforms: \n\n";
		for (unsigned int i = 0; i < platforms.size(); i++) {
			cout << "\t" << i + 1 << ": " << platforms[i].getInfo<CL_PLATFORM_NAME>() << endl;
		}

		// Choose and create an OpenCL platform
		cout << endl << "Enter the number of the OpenCL platform you want to use: ";
		unsigned int input = 0;
		cin >> input;

		// Handle incorrect user input
		while (input < 1 || input > platforms.size()) {
			cin.clear(); //clear errors/bad flags on cin
			cin.ignore(cin.rdbuf()->in_avail(), '\n'); // ignores exact number of chars in cin buffer
			cout << "No such platform." << endl << "Enter the number of the OpenCL platform you want to use: ";
			cin >> input;
		}

		platform = platforms[input - 1];
	}
	cout << "Using OpenCL platform: \t" << platform.getInfo<CL_PLATFORM_NAME>() << endl;
	return platform;
}

Device selectDevice(Platform& platform) {
	using namespace std;

	// Find all available OpenCL devices (e.g. CPU, GPU or integrated GPU)
	vector<Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

	if ( devices.size() == 0 ) {
		cerr << "No devices available on platform " << platform.getInfo<CL_PLATFORM_NAME>() << endl;
		exit(-1);
	}

	Device device;

	if ( devices.size() == 1 ) {
		device = devices[0];
	} else {
	#ifdef DEFAULT_DEVICE
		device = devices[DEFAULT_DEVICE - 1];
	#else
		// Print the names of all available OpenCL devices on the chosen platform
		cout << "Available OpenCL devices on this platform: " << endl << endl;
		for (unsigned int i = 0; i < devices.size(); i++) {
			cout << "\t" << i + 1 << ": " << devices[i].getInfo<CL_DEVICE_NAME>() << endl;
		}

		// Choose an OpenCL device
		cout << endl << "Enter the number of the OpenCL device you want to use: ";
		unsigned int input = 0;
		cin >> input;

		// Handle incorrect user input
		while (input < 1 || input > devices.size()) {
			cin.clear(); //clear errors/bad flags on cin
			cin.ignore(cin.rdbuf()->in_avail(), '\n'); // ignores exact number of chars in cin buffer
			cout << "No such device. Enter the number of the OpenCL device you want to use: ";
			cin >> input;
		}
		device = devices[input - 1];
	#endif
	}
	cout << endl << "Using OpenCL device: \t" << device.getInfo<CL_DEVICE_NAME>() << endl << endl;

	return device;
}


inline float clamp(float x) { return x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x; }

// convert RGB float in range [0,1] to int in range [0, 255]
inline int toInt(float x) { return int(clamp(x) * 255 + .5); }

void saveDepthImage(const std::string& fileName, 
					unsigned int width, 
					unsigned int height,
					cl_float * data) {
	using namespace std;

	std::ofstream saveFile{fileName, std::ios::out};
	std::size_t idx = 0;
	for( unsigned int row = 0; row < height; row++ ) {
		for( unsigned int col = 0; col < width; col++ ) {
			saveFile << data[idx] << " ";
			idx++;
		}
		saveFile << endl;
	}
	saveFile.close();
}


template<typename Func>
void saveImage(const std::string& fileName, unsigned int width, unsigned int height, Func dataFunction, int maxValue ) {
	using namespace std;

	std::ofstream saveFile{fileName, std::ios::out};
	saveFile << "P2" << endl;
	saveFile << width << " " << height << endl;

	if ( maxValue == 0 ) {
		for (int i = 0; i < height * width; i++) {
			if ( dataFunction(i) > maxValue ) {
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

void buildProgram( Program& program, const Device& device ) {
	using namespace std;

	cl_int result = program.build({ device }, "");
	if (result) {
		if (result == CL_BUILD_PROGRAM_FAILURE) {
			string name     = device.getInfo<CL_DEVICE_NAME>();
			string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
			cerr << "Build log for " << name << ":" << endl
			     << buildlog << endl;    		// Print the log
		}
		else {
			cout << "Error during compilation! (" << result << ")" << endl;
		}
		exit( -1 );
	}
}


void loadMesh( const std::string& filename,
               cl_float3 ** cpuVertices,
               cl_int3   ** cpuFaces,
               cl_float3 ** cpuFaceNormals,
               unsigned int* pNumVertices,
               unsigned int* pNumFaces) {

	using namespace std;
	using namespace Eigen;

    animesh::ObjFileParser parser;
    pair<vector<Eigen::Vector3f>, vector<pair<vector<std::size_t>,Vector3f>>> object_data = parser.parse_file_raw_with_normals(filename);

    unsigned int numVertices = object_data.first.size();
    unsigned int numFaces	 = object_data.second.size();

	*cpuVertices = new cl_float3[numVertices];
	*cpuFaces = new cl_int3[numFaces];
	*cpuFaceNormals = new cl_float3[numFaces];

	for (int i = 0; i < numVertices; ++i) {
		(*cpuVertices)[i] = cl_float3{
			object_data.first[i].x(), 
			object_data.first[i].y(), 
			object_data.first[i].z() };
	}
	for( int i=0; i< numFaces; ++i ) {
		pair<vector<std::size_t>,Vector3f> face_data = object_data.second[i];
		vector<std::size_t> face_vertex_indices = face_data.first;
		Vector3f face_normal = face_data.second;

		(*cpuFaces)[i] = cl_int3{
			(int)face_vertex_indices[0], 
			 (int)face_vertex_indices[1], 
			 (int)face_vertex_indices[2]};

		(*cpuFaceNormals)[i] = cl_float3{
			face_normal.x(), 
			face_normal.y(), 
			face_normal.z()};
	}
	*pNumVertices = numVertices;
	*pNumFaces = numFaces;
}

int main(int argc, char * argv[]) {
	using namespace std;

	if( argc < 2 ) {
		cerr << "ERROR::NOMODEL" << endl;
		exit(-1);
	}

	std::string model_filename = argv[1];
	std::string camera_filename;
	if( argc > 2 ) {
		camera_filename = argv[2];
	} else {
		camera_filename = CAMERA_FILE;
	}
	string suffix = "";
	if( argc > 4 ) {
		cerr << "ERROR::INVALID_ARGS" << endl;
	} else {
		suffix = argv[3];
	}


	Platform platform = selectPlatform( );
	Device device = selectDevice(platform);

	// Create an OpenCL context on that device to manage all the OpenCL resources
	Context context{device};

	// Create an OpenCL program by performing runtime source compilation
	string kernelSource = loadCode( "ray_trace.cl");
	Program program{context, kernelSource};

	// Build the program and check for compilation errors (exit on fail)
	buildProgram(program, device);

	// Create a kernel (entry point in the OpenCL source program)
	// kernels are the basic units of executable code that run on the OpenCL device
	// the kernel forms the starting point into the OpenCL program, analogous to main() in CPU code
	// kernels can be called from the host (CPU)
	cl_int err;
	Kernel kernel{program, "ray_trace", &err};
	if ( err != CL_SUCCESS) {
		cerr << "Failed to create kernel " << err << endl;
	} else {
		cout << "Kernel created" << endl;
	}

	// Create data arrays on the host (= CPU)
	const unsigned int width = 640;
	const unsigned int height = 480;
	const unsigned int numElements = width * height;
	cl_float * cpuDepthData  = new cl_float[numElements];
	cl_int   * cpuVertexData = new cl_int[numElements];
	cl_float3* cpuVertices;
	cl_int3  * cpuFaces;
	cl_float3 * cpuFaceNormals;
	unsigned int numVertices;
	unsigned int numFaces;
	Camera cpuCamera = loadCameraFromFile(camera_filename);

	loadMesh( model_filename, &cpuVertices, &cpuFaces, &cpuFaceNormals, &numVertices, &numFaces);

	// Create buffers (memory objects) on the OpenCL device, allocate memory and copy input data to device.
	// Flags indicate how the buffer should be used e.g. read-only, write-only, read-write
	Buffer gpuDepthBuffer{context, CL_MEM_WRITE_ONLY, numElements * sizeof(cl_float)};
	Buffer gpuVertexBuffer{context, CL_MEM_WRITE_ONLY, numElements * sizeof(cl_int)};
	Buffer gpuVertices{context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, numVertices * sizeof(cl_float3), cpuVertices};
	Buffer gpuFaces{context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, numFaces * sizeof(cl_int3), cpuFaces};
	Buffer gpuFaceNormals{context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, numFaces * sizeof(cl_float3), cpuFaceNormals};

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
	memcpy((void *)&(gpu_cam.position), cpuCamera.position, sizeof(cpuCamera.position));
	memcpy((void *)&(gpu_cam.view), cpuCamera.view, sizeof(cpuCamera.view));
	memcpy((void *)&(gpu_cam.up), cpuCamera.up, sizeof(cpuCamera.up));
	memcpy((void *)&(gpu_cam.resolution), cpuCamera.resolution, sizeof(cpuCamera.resolution));
	memcpy((void *)&(gpu_cam.fov), cpuCamera.fov, sizeof(cpuCamera.fov));
	gpu_cam.focalDistance = cpuCamera.focalDistance;
	Buffer gpuCamera{context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(gpu_cam), (void *)&gpu_cam};

	delete[] cpuVertices;
	delete[] cpuFaces;
	delete[] cpuFaceNormals;


	// Specify the arguments for the OpenCL kernel
	//
	kernel.setArg(0, numVertices);
	kernel.setArg(1, gpuVertices);			// Vertices of mesh
	kernel.setArg(2, numFaces);
	kernel.setArg(3, gpuFaces);				// Faces of mesh as 3 vertex indices CCW
	kernel.setArg(4, gpuFaceNormals);		// Faces of mesh as 3 vertex indices CCW
	kernel.setArg(5, gpuCamera);			// Camera
	kernel.setArg(6, gpuDepthBuffer);		// Depth image rendered to here
	kernel.setArg(7, gpuVertexBuffer);		// Vertex image rendered to here
	kernel.setArg(8, width);				// Width of output images
	kernel.setArg(9, height);  				// Height of output images

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
	if ( err != CL_SUCCESS) {
		cerr << "Failed to enqueue kernel " << err << endl;
	}

	// Read and copy Depth data back to CPU and save
	// the "CL_TRUE" flag blocks the read operation until all work items have finished their computation
	err = queue.enqueueReadBuffer(gpuDepthBuffer, CL_TRUE, 0, numElements * sizeof(cl_float), cpuDepthData);
	if ( err != CL_SUCCESS) {
		cerr << "Failed to read buffer " << err << endl;
	}
	err = queue.enqueueReadBuffer(gpuVertexBuffer, CL_TRUE, 0, numElements * sizeof(cl_int), cpuVertexData);
	if ( err != CL_SUCCESS) {
		cerr << "Failed to read buffer " << err << endl;
	}

	// export as text format float file
	string depthFileName = "/Users/dave/Desktop/depth" + suffix + ".mat";
	saveDepthImage(depthFileName, width, height, cpuDepthData);

	depthFileName = "/Users/dave/Desktop/depth" + suffix + ".pgm";
	saveImage( depthFileName, width, height,
	[cpuDepthData](int i) {
		float depth = cpuDepthData[i];
		if(isinf(depth)) return 0;
		return (int)(depth * 255);
	}, 0);
	string vertexFileName = "/Users/dave/Desktop/vertex" + suffix + ".pgm";
	saveImage( vertexFileName, width, height,
	[cpuVertexData](int i) {
		return cpuVertexData[i];
	}, 0);


	delete[] cpuDepthData;
	delete[] cpuVertexData;
}