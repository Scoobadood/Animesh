// Getting started with OpenCL tutorial
// by Sam Lapere, 2016, http://raytracey.blogspot.com
// Code based on http://simpleopencl.blogspot.com/2013/06/tutorial-simple-start-with-opencl-and-c.html

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#ifdef __APPLE__
#include "cl.hpp"
#else /// your stuff for linux
#include <CL/cl.hpp> // main OpenCL include file 
#endif

using namespace cl;

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
	catch (ifstream::failure e) {
		cout << "ERROR::FILE_NOT_SUCCESFULLY_READ" << endl;
	}
	return code;
}


unsigned int selectPlatform(const std::vector<Platform>& platforms) {
	using namespace std;

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

	return input;
}

unsigned int selectDevice(const std::vector<Device>& devices) {
	using namespace std;

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

	return input;
}


inline float clamp(float x){ return x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x; }

// convert RGB float in range [0,1] to int in range [0, 255]
inline int toInt(float x){ return int(clamp(x) * 255 + .5); }


void saveImage(const std::string& fileName, const cl_float3* data, unsigned int width, unsigned int height) {
	using namespace std;

	// write image to PPM file, a very simple image file format
	// PPM files can be opened with IrfanView (download at www.irfanview.com) or GIMP
	std::ofstream saveFile{fileName, std::ios::out};
	saveFile << "P3" << endl;
	saveFile << width << " " << height << endl << 255 << endl;
	// loop over pixels, write RGB values
	int i = 0;
	for (int h = 0; h < height; h++){
		for (int w = 0; w < width; w++){
			if( i % 10000 == 0) {
				cout << "i: " << i << ",  data[i] " << data[i].x << endl;
			}
			saveFile << toInt(data[i].s[0]) << " " 
					 << toInt(data[i].s[1]) << " " 
					 << toInt(data[i].s[2]) << " ";
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

int main() {
	using namespace std;

	// Find all available OpenCL platforms (e.g. AMD, Nvidia, Intel)
	vector<Platform> platforms;
	Platform::get(&platforms);

	// Select one
	unsigned int input = selectPlatform(platforms);
	Platform platform = platforms[input - 1];
	cout << "Using OpenCL platform: \t" << platform.getInfo<CL_PLATFORM_NAME>() << endl;

	// Find all available OpenCL devices (e.g. CPU, GPU or integrated GPU)
	vector<Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

	// Select one
	input = selectDevice(devices);
	Device device = devices[input - 1];
	cout << endl << "Using OpenCL device: \t" << device.getInfo<CL_DEVICE_NAME>() << endl << endl;

	// Create an OpenCL context on that device to manage all the OpenCL resources
	Context context = Context(device);

	// Create an OpenCL program by performing runtime source compilation
	string kernelSource = loadCode( "red_green.cl");
	Program program{context, kernelSource};

	// Build the program and check for compilation errors (exit on fail)
	buildProgram(program, device);

	// Create a kernel (entry point in the OpenCL source program)
	// kernels are the basic units of executable code that run on the OpenCL device
	// the kernel forms the starting point into the OpenCL program, analogous to main() in CPU code
	// kernels can be called from the host (CPU)
	cl_int err;
	Kernel kernel{program, "red_green", &err};
	if( err != CL_SUCCESS) {
		cerr << "Failed to create kernel " << err << endl;
	} else {
		cout << "Kernel created" << endl;
	}

	// Create input data arrays on the host (= CPU)
	const unsigned int width = 640;
	const unsigned int height = 480;
	const unsigned int numElements = width * height;
	cl_float3 *cpuImageData = new cl_float3[numElements];
	for( int i=0; i<numElements; i++ ) {
		cpuImageData[i].x = 99.;
		cpuImageData[i].y = 53.;
		cpuImageData[i].z = 17.;
		cpuImageData[i].w = 42.;
	}

	// Create buffers (memory objects) on the OpenCL device, allocate memory and copy input data to device.
	// Flags indicate how the buffer should be used e.g. read-only, write-only, read-write
	Buffer gpuImageBuffer{context, CL_MEM_WRITE_ONLY, numElements * sizeof(cl_float3)};

	// Specify the arguments for the OpenCL kernel
	// (the arguments are __global float* x, __global float* y and __global float* z)
	kernel.setArg(0, gpuImageBuffer); // first argument
	kernel.setArg(1, width); // second argument
	kernel.setArg(2, height);  // third argument

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
	if( err != CL_SUCCESS) {
		cerr << "Failed to enqueue kernel " << err << endl;
	}
//	queue.finish();

	// Read and copy OpenCL output to CPU
	// the "CL_TRUE" flag blocks the read operation until all work items have finished their computation
	err = queue.enqueueReadBuffer(gpuImageBuffer, CL_TRUE, 0, numElements * sizeof(cl_float3), cpuImageData);
	if( err != CL_SUCCESS) {
		cerr << "Failed to read buffer " << err << endl;
	}

	// Save result to image file
	saveImage("/Users/dave/Desktop/red_green.ppm", cpuImageData, width, height);
	delete[] cpuImageData;
}