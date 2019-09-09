#include "utility.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

const std::string KERNEL_DIRECTORY = "/Users/dave/Animesh/src/tools/data_gen/";
#define DEFAULT_DEVICE 3


/*
 * Load an OpenCL kernel
 */
std::string loadCode( const std::string& kernelPath ) {
	using namespace std;

	string code;
	ifstream file;
	string sourceFile;
	file.exceptions (ifstream::failbit | ifstream::badbit);
	try {
		sourceFile = KERNEL_DIRECTORY + kernelPath;
		file.open(sourceFile);
		stringstream stream;
		stream << file.rdbuf();
		file.close();
		code   = stream.str();
	}
	catch (ifstream::failure& e) {
		cout << "ERROR::FILE_NOT_SUCCESFULLY_READ [" << sourceFile << "]" << endl;
	}
	return code;
}



cl::Platform selectPlatform() {
	using namespace std;
	using namespace cl;

	// Find all available OpenCL platforms (e.g. AMD, Nvidia, Intel)
	vector<Platform> platforms;
	Platform::get(&platforms);

	if ( platforms.empty() ) {
		cerr << "No OpenCL platform available" << endl;
		throw runtime_error("No OpenCL platform available");
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

cl::Device selectDevice(cl::Platform& platform) {
	using namespace std;
	using namespace cl;

	// Find all available OpenCL devices (e.g. CPU, GPU or integrated GPU)
	vector<Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

	if ( devices.size() == 0 ) {
		cerr << "No devices available on platform " << platform.getInfo<CL_PLATFORM_NAME>() << endl;
		throw runtime_error("No devices available on platform");
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


void buildProgram( cl::Program& program, const cl::Device& device ) {
	using namespace std;
	using namespace cl;

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
		throw runtime_error("Error during compilation");
	}
}



