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


Platform selectPlatform() {
	using namespace std;

	// Find all available OpenCL platforms (e.g. AMD, Nvidia, Intel)
	vector<Platform> platforms;
	Platform::get(&platforms);

	if( platforms.size() == 0 ) {
		cerr << "No OpenCL platform available" << endl;
		exit(-1);
	}

	Platform platform;
	if( platforms.size() == 1 ) {
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

		platform = platforms[input-1];
	}
	cout << "Using OpenCL platform: \t" << platform.getInfo<CL_PLATFORM_NAME>() << endl;
	return platform;
}

Device selectDevice(Platform& platform) {
	using namespace std;

	// Find all available OpenCL devices (e.g. CPU, GPU or integrated GPU)
	vector<Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

	if( devices.size() == 0 ) {
		cerr << "No devices available on platform " << platform.getInfo<CL_PLATFORM_NAME>() << endl;
		exit(-1);
	}

	Device device;

	if( devices.size() == 1 ) {
		device = devices[0];
	} else {
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
	}
	cout << endl << "Using OpenCL device: \t" << device.getInfo<CL_DEVICE_NAME>() << endl << endl;

	return device;
}


inline float clamp(float x){ return x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x; }

// convert RGB float in range [0,1] to int in range [0, 255]
inline int toInt(float x){ return int(clamp(x) * 255 + .5); }


template<typename Func>
void saveImage(const std::string& fileName, unsigned int width, unsigned int height, Func dataFunction, int maxValue ) {
	using namespace std;
	
	std::ofstream saveFile{fileName, std::ios::out};
	saveFile << "P2" << endl;
	saveFile << width << " " << height << endl;

	if( maxValue == 0 ) {
		for (int i = 0; i < height * width; i++){
			if( dataFunction(i) > maxValue ) {
				maxValue = dataFunction(i);
			}
		}
	}
	saveFile << maxValue << endl;

	// loop over pixels, write greyscale values
	int i = 0;
	for (int h = 0; h < height; h++){
		for (int w = 0; w < width; w++){
			saveFile << dataFunction(i) << " ";
			i++;
		}
		saveFile << endl;
	}
	saveFile.close();


}

void saveDepthImage(const std::string& fileName, const cl_float* data, unsigned int width, unsigned int height) {
	using namespace std;

	saveImage( fileName, width, height, 
		 [data](int i) {
            return (toInt(data[i] * 255));
         }, 255);
}

void saveVertexImage(const std::string& fileName, const cl_int* data, unsigned int width, unsigned int height) {
	using namespace std;

	saveImage( fileName, width, height, 
		 [data](int i) {
            return (data[i]);
         }, 0);

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
	if( err != CL_SUCCESS) {
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

	// Create buffers (memory objects) on the OpenCL device, allocate memory and copy input data to device.
	// Flags indicate how the buffer should be used e.g. read-only, write-only, read-write
	Buffer gpuDepthBuffer{context, CL_MEM_WRITE_ONLY, numElements * sizeof(cl_float)};
	Buffer gpuVertexBuffer{context, CL_MEM_WRITE_ONLY, numElements * sizeof(cl_int)};

	// Specify the arguments for the OpenCL kernel
	// (the arguments are __global float* x, __global float* y and __global float* z)
	kernel.setArg(0, gpuDepthBuffer);		// first argument
	kernel.setArg(1, gpuVertexBuffer);		// first argument
	kernel.setArg(2, width);				// second argument
	kernel.setArg(3, height);  				// third argument

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

	// Read and copy Depth data back to CPU and save
	// the "CL_TRUE" flag blocks the read operation until all work items have finished their computation
	err = queue.enqueueReadBuffer(gpuDepthBuffer, CL_TRUE, 0, numElements * sizeof(cl_float), cpuDepthData);
	if( err != CL_SUCCESS) {
		cerr << "Failed to read buffer " << err << endl;
	}
	saveDepthImage("/Users/dave/Desktop/depth.pgm", cpuDepthData, width, height);
	delete[] cpuDepthData;

	err = queue.enqueueReadBuffer(gpuVertexBuffer, CL_TRUE, 0, numElements * sizeof(cl_int), cpuVertexData);
	if( err != CL_SUCCESS) {
		cerr << "Failed to read buffer " << err << endl;
	}
	saveVertexImage("/Users/dave/Desktop/vertex.pgm", cpuVertexData, width, height);
	delete[] cpuVertexData;
}