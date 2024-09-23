#pragma once

#include <CL/cl.hpp>
#include <string>
#include <vector>

// Create an OpenCL context
// Returns true (1) on success, and false (0) on failure
bool CreateContext(cl::Context& context);

// Build an OpenCL program from source
// Returns true (1) on success, and false (0) on failure
bool BuildProgram(cl::Program& program, cl::Context context, const std::vector<std::string>& sources);

// Builds an OpenCL kernel from a cl::Program with the given kernel name
cl::Kernel BuildKernel(cl::Program& program, const std::string& kernel_name);

// Returns the first cl::Device used in a cl::Context object
cl::Device GetContextDevice(const cl::Context& context);