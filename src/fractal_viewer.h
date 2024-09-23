#pragma once

#include <CL/opencl.hpp>
#include <SDL2/SDL.h>
#include "fractals.h"

class FractalViewer
{
private:
	bool is_initalized = false;

	SDL_Window* window     = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_Texture* texture   = nullptr;

	int window_width  = 0;
	int window_height = 0;

	int mouse_x = 0;
	int mouse_y = 0;
	int prev_mouse_x = 0;
	int prev_mouse_y = 0;
	bool left_mouse_down = false;

	FractalType fractal_type = FractalType::Mandelbrot;
	Fractal fractal {};
	bool fractal_has_args = false;

	cl::Context cl_context		{};
	cl::Kernel  cl_kernel		{};
	cl::CommandQueue cl_queue	{};

	cl::Buffer cl_img_buffer	  {};
	cl::Buffer cl_settings_buffer {};
	cl::Buffer cl_position_buffer {};
	cl::Buffer cl_color_buffer    {};
	cl::Buffer cl_args_buffer     {};

	size_t cl_img_buffer_size      = 0;
	size_t cl_settings_buffer_size = 0;
	size_t cl_position_buffer_size = 0;
	size_t cl_color_buffer_size    = 0;
	size_t cl_args_buffer_size     = 0;

	std::vector<int> fractal_settings;
	std::vector<double> position_info;
	std::vector<float> color_settings;

	int max_iterations = 512;
	double position_x = -0.35;
	double position_y = 0.0;
	double zoom = 0.0035;

	float color_scale = 0.5f;
	int offset_r = 0;
	int offset_g = 0;
	int offset_b = 0;

	const double zoom_per_scroll = 0.90;
	const int pan_amount = 10;
	bool regenerate = false;

	bool menu_open = true;
	const char* double_format = "%.17f";
	
	void CreateTexture(int width, int height);
	void CreateOutputBuffer(int width, int height);

	void BuildFractalGenerator();
	void ResetFractal();

	void Generate(int x, int y, int width, int height);
	void Generate();

	void Pan(int dx, int dy);

	void Menu();

public:
	FractalViewer(int width, int height);
	~FractalViewer();

	bool IsInitialized() const;

	void Run();
};