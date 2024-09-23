#include "cl_helper.h"
#include <iostream>
#include <fstream>
#include <sstream>

bool CreateContext(cl::Context& context)
{
	std::vector<cl::Platform> platforms;
	cl::Platform::get(&platforms);

	if (platforms.empty())
	{
		std::cout << "OpenCL error: No platforms found\n";
		return 0;
	}

	std::vector<cl::Device> gpu_devices_all;
	std::vector<cl::Device> cpu_devices_all;

	for (cl::Platform& platform : platforms)
	{
		std::vector<cl::Device> gpu_devices;
		std::vector<cl::Device> cpu_devices;

		platform.getDevices(CL_DEVICE_TYPE_GPU, &gpu_devices);
		platform.getDevices(CL_DEVICE_TYPE_CPU, &cpu_devices);

		gpu_devices_all.insert(gpu_devices_all.end(), gpu_devices.begin(), gpu_devices.end());
		cpu_devices_all.insert(cpu_devices_all.end(), cpu_devices.begin(), cpu_devices.end());
	}

	cl::Device device {};

	// Select GPU first if available
	if (!gpu_devices_all.empty())
	{
		device = gpu_devices_all[0];
	}
	else if (!cpu_devices_all.empty())
	{
		device = cpu_devices_all[0];
	}
	else
	{
		std::cout << "OpenCL error: No devices found\n";
		return 0;
	}

	std::string device_name = device.getInfo<CL_DEVICE_NAME>();
	std::cout << "Using device: " << device_name << "\n";

	context = cl::Context({ device });

	return 1;
}

bool BuildProgram(cl::Program& program, cl::Context context, const std::vector<std::string>& source_paths)
{
	std::string program_source {};

	for (const std::string& source_path : source_paths)
	{
		std::ifstream input(source_path);

		if (!input.is_open())
		{
			std::cout << "Error: Could not open file '" << source_path << "'\n";
			return 0;
		}

		std::stringstream input_buffer;
		input_buffer << input.rdbuf();

		program_source += (input_buffer.str() + "\n");
	}

	cl::Program::Sources sources;
	sources.push_back({ program_source.c_str(), program_source.length() });

	cl::Device device = GetContextDevice(context);

	program = cl::Program(context, sources);
	if (program.build({ device }) != CL_SUCCESS)
	{
		std::cout << "OpenCL error: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device) << "\n";
		return 0;
	}

	return 1;
}

cl::Kernel BuildKernel(cl::Program& program, const std::string& kernel_name)
{
	int error = 0;
	cl::Kernel kernel(program, kernel_name.c_str(), &error);

	if (error)
	{
		std::cout << "OpenCL error: Error creating kernel (Error " << error << ")\n";
		return {};
	}

	return kernel;
}

cl::Device GetContextDevice(const cl::Context& context)
{
	return context.getInfo<CL_CONTEXT_DEVICES>()[0];
}
