#include "fractal_viewer.h"
#include "cl_helper.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_sdlrenderer2.h"
#include <iostream>
#include <chrono>

FractalViewer::FractalViewer(int width, int height)
	: window_width(width), window_height(height)
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		std::cout << "SDL could not initialize, SDL_Error: " << SDL_GetError() << "\n";
		return;
	}

	// Create window
	Uint32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
	window = SDL_CreateWindow("Fractal Viewer by Sam Partington", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, window_flags);
	if (window == nullptr)
	{
		std::cout << "SDL could not create window, SDL_Error: " << SDL_GetError() << "\n";
		return;
	}

	Uint32 renderer_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
	renderer = SDL_CreateRenderer(window, -1, renderer_flags);
	if (renderer == nullptr)
	{
		std::cout << "SDL could not create renderer, SDL_Error: " << SDL_GetError() << "\n";
		return;
	}

	// Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer2_Init(renderer);

	// Create OpenCL context 
	if (!CreateContext(cl_context))
		return;

	BuildFractalGenerator();

	cl_queue = cl::CommandQueue(cl_context, GetContextDevice(cl_context));

	CreateTexture(width, height);
	CreateOutputBuffer(width, height);

	cl_settings_buffer_size = sizeof(int) * 3;
	cl_settings_buffer = cl::Buffer(cl_context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, cl_settings_buffer_size);

	cl_position_buffer_size = sizeof(double) * 3;
	cl_position_buffer = cl::Buffer(cl_context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, cl_position_buffer_size);

	cl_color_buffer_size = sizeof(float) * 4;
	cl_color_buffer = cl::Buffer(cl_context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, cl_color_buffer_size);

	is_initalized = true;
}

FractalViewer::~FractalViewer()
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();

	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	window = nullptr;
	renderer = nullptr;
}

void FractalViewer::Generate(int x, int y, int width, int height)
{
	auto start = std::chrono::high_resolution_clock::now();

	Uint32* pixels = nullptr;
	int pitch = 0;
	SDL_LockTexture(texture, nullptr, reinterpret_cast<void**>(&pixels), &pitch);

	fractal_settings = { max_iterations, window_width, window_height };
	position_info = { zoom, position_x, position_y };
	color_settings = { color_scale, static_cast<float>(offset_r), static_cast<float>(offset_g), static_cast<float>(offset_b)};

	cl::Event event_1, event_2, event_3;
	cl_queue.enqueueWriteBuffer(cl_settings_buffer, CL_FALSE, 0, cl_settings_buffer_size, fractal_settings.data(), nullptr, &event_1);
	cl_queue.enqueueWriteBuffer(cl_position_buffer, CL_FALSE, 0, cl_position_buffer_size, position_info.data(), nullptr, &event_2);
	cl_queue.enqueueWriteBuffer(cl_color_buffer, CL_FALSE, 0, cl_color_buffer_size, color_settings.data(), nullptr, &event_3);

	if (fractal_has_args)
	{
		std::vector<double> args(fractal.variables.size());

		for (int i = 0; i < fractal.variables.size(); i++)
			args[i] = fractal.variables[i].value;

		cl_queue.enqueueWriteBuffer(cl_args_buffer, CL_TRUE, 0, cl_args_buffer_size, args.data());
		cl_kernel.setArg(4, cl_args_buffer);
	}

	event_1.wait();
	event_2.wait();
	event_3.wait();

	cl_kernel.setArg(0, cl_img_buffer);
	cl_kernel.setArg(1, cl_settings_buffer);
	cl_kernel.setArg(2, cl_position_buffer);
	cl_kernel.setArg(3, cl_color_buffer);

	cl_queue.enqueueNDRangeKernel(cl_kernel, cl::NDRange(x, y), cl::NDRange(width, height));

	cl_queue.enqueueReadBuffer(cl_img_buffer, CL_TRUE, 0, cl_img_buffer_size, pixels);

	auto end = std::chrono::high_resolution_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
	std::cout << "Took " << (static_cast<double>(diff) / 1000.0) << "ms to generate\n";

	SDL_UnlockTexture(texture);
}

void FractalViewer::Generate()
{
	Generate(0, 0, window_width, window_height);
}

void FractalViewer::Pan(int dx, int dy)
{
	Uint32* pixels = nullptr;
	int pitch = 0;
	SDL_LockTexture(texture, nullptr, reinterpret_cast<void**>(&pixels), &pitch);

	const int num_pixels = window_width * window_height;
	std::vector<Uint32> pixels_copy(pixels, pixels + num_pixels);

	int index = 0;
	const int index_y_offset = dy * window_width;

	for (int y = 0; y < window_height; y++)
	{
		for (int x = 0; x < window_width; x++)
		{
			int copy_index = index + index_y_offset + dx;

			if (copy_index < cl_img_buffer_size)
				pixels[index] = pixels_copy[copy_index];

			index++;
		}
	}

	position_x += zoom * dx;
	position_y += zoom * dy;

	SDL_UnlockTexture(texture);

	cl_queue.enqueueWriteBuffer(cl_img_buffer, CL_TRUE, 0, cl_img_buffer_size, pixels);

	if (dx > 0)
		Generate(window_width - std::abs(dx), 0, std::abs(dx), window_height);
	else if(dx < 0)
		Generate(0, 0, std::abs(dx), window_height);

	if (dy > 0)
		Generate(0, window_height - std::abs(dy), window_width, std::abs(dy));
	else if (dy < 0)
		Generate(0, 0, window_width, std::abs(dy));
}

void FractalViewer::CreateTexture(int width, int height)
{
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);
}

void FractalViewer::CreateOutputBuffer(int width, int height)
{
	cl_img_buffer_size = width * height * sizeof(Uint32);
	cl_img_buffer = cl::Buffer(cl_context, CL_MEM_READ_WRITE, cl_img_buffer_size);
}

void FractalViewer::BuildFractalGenerator()
{
	fractal = GetFractal(fractal_type);

	cl::Program cl_program {};

	// Build program
	if (!BuildProgram(cl_program, cl_context, { "fractals/color.cl", fractal.source_path }))
		return;

	// Create kernel
	cl_kernel = BuildKernel(cl_program, "calculate");

	// Create fractal args buffer
	if (!fractal.variables.empty())
	{
		cl_args_buffer_size = sizeof(double) * fractal.variables.size();
		cl_args_buffer = cl::Buffer(cl_context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR, cl_args_buffer_size);

		fractal_has_args = true;
	}
	else
	{
		fractal_has_args = false;
	}

	// Set fractal defaults
	ResetFractal();
}

void FractalViewer::ResetFractal()
{
	position_x = fractal.default_x_pos;
	position_y = fractal.default_y_pos;
	zoom = fractal.default_zoom;

	for (FractalVariable& variable : fractal.variables)
		variable.value = variable.default_value;
}

bool FractalViewer::IsInitialized() const
{
	return is_initalized;
}

void FractalViewer::Menu()
{
	if (!menu_open)
		return;

	if (!ImGui::Begin("Fractal Generator (Tab to toggle)", &menu_open))
	{
		ImGui::End();
		return;
	}

	if (ImGui::TreeNodeEx("Fractal", ImGuiTreeNodeFlags_DefaultOpen))
	{
		int selected_fractal = static_cast<int>(fractal_type);
		for (int fractal_idx = 0; fractal_idx < fractals.size(); fractal_idx++)
		{
			if (ImGui::Selectable(fractals[fractal_idx].name.c_str(), fractal_idx == selected_fractal))
			{
				fractal_type = static_cast<FractalType>(fractal_idx);
				BuildFractalGenerator();
				regenerate = true;
			}
		}
		ImGui::TreePop();
	}

	ImGui::NewLine();
	ImGui::PushItemWidth(160.0f);

	if (ImGui::InputDouble("X", &position_x, 0.0, 0.0, double_format, ImGuiInputTextFlags_EnterReturnsTrue))
		regenerate = true;

	if (ImGui::InputDouble("Y", &position_y, 0.0, 0.0, double_format, ImGuiInputTextFlags_EnterReturnsTrue))
		regenerate = true;

	if (ImGui::InputDouble("Zoom", &zoom, 0.0, 0.0, double_format, ImGuiInputTextFlags_EnterReturnsTrue))
		regenerate = true;

	ImGui::NewLine();

	if (ImGui::InputInt("Escape Max Iterations", &max_iterations, 0, 100, ImGuiInputTextFlags_EnterReturnsTrue))
		regenerate = true;

	if (ImGui::SliderInt("##slider-maxiterations", &max_iterations, 2, 2048))
		regenerate = true;

	ImGui::NewLine();

	if (ImGui::TreeNodeEx("Color", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (ImGui::InputFloat("Color Scale", &color_scale, 0.01f, 0.1f, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
			regenerate = true;

		if (ImGui::InputInt("R Offset", &offset_r, 1, 10, ImGuiInputTextFlags_EnterReturnsTrue))
			regenerate = true;

		if (ImGui::InputInt("G Offset", &offset_g, 1, 10, ImGuiInputTextFlags_EnterReturnsTrue))
			regenerate = true;

		if (ImGui::InputInt("B Offset", &offset_b, 1, 10, ImGuiInputTextFlags_EnterReturnsTrue))
			regenerate = true;

		ImGui::TreePop();
	}

	ImGui::PopItemWidth();

	ImGui::PushItemWidth(240.0f);

	if (!fractal.variables.empty())
	{
		ImGui::NewLine();

		if (ImGui::TreeNodeEx(fractal.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (int i = 0; i < fractal.variables.size(); i++)
			{
				FractalVariable& variable = fractal.variables[i];
				const char* name = variable.name.c_str();

				if (ImGui::InputDouble(name, &variable.value, variable.min, variable.max, "%.6f", ImGuiInputTextFlags_EnterReturnsTrue))
					regenerate = true;

				std::string temp_slider_name = "##slider" + std::to_string(i);

				if (ImGui::SliderScalar(temp_slider_name.c_str(), ImGuiDataType_Double, &variable.value, &variable.min, &variable.max, "%.6f"))
					regenerate = true;

				ImGui::NewLine();
			}

			ImGui::TreePop();
		}
	}

	ImGui::PopItemWidth();

	ImGui::End();
}

void FractalViewer::Run()
{
	if (!IsInitialized())
	{
		std::cout << "FractalViewer error: FractalViewer is uninitialized.\n";
		return;
	}

	Generate();

	ImGuiIO& io = ImGui::GetIO();

	bool running = true;
	while (running)
	{
		Uint32 mouse_state = SDL_GetMouseState(&mouse_x, &mouse_y);
		left_mouse_down = (mouse_state & SDL_BUTTON_LEFT);

		regenerate = false;

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);

			if (event.type == SDL_QUIT)
			{
				running = false;
			}
			else if (event.type == SDL_WINDOWEVENT)
			{
				if (event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					window_width = event.window.data1;
					window_height = event.window.data2;

					CreateTexture(window_width, window_height);
					CreateOutputBuffer(window_width, window_height);

					regenerate = true;
				}
			}
			else if (event.type == SDL_KEYDOWN)
			{
				const int key = event.key.keysym.sym;

				if (key == SDLK_TAB)
					menu_open = !menu_open;

				if (io.WantCaptureKeyboard)
					break;

				switch (key)
				{
				case SDLK_r:
					ResetFractal();
					regenerate = true;
					break;
				case SDLK_w:
					Pan(0, -pan_amount);
					break;
				case SDLK_a:
					Pan(-pan_amount, 0);
					break;
				case SDLK_s:
					Pan(0, pan_amount);
					break;
				case SDLK_d:
					Pan(pan_amount, 0);
					break;
				case SDLK_EQUALS:
				case SDLK_PLUS:
					zoom *= zoom_per_scroll;
					regenerate = true;
					break;
				case SDLK_MINUS:
					zoom /= zoom_per_scroll;
					regenerate = true;
					break;
				default:
					break;
				}
			}
			else if (event.type == SDL_MOUSEWHEEL && !io.WantCaptureMouse)
			{
				if (event.wheel.y == 0)
					break;

				double zoom_amount = std::pow(zoom_per_scroll, std::abs(event.wheel.y));
				double delta_x = zoom * ((2.0 * static_cast<double>(mouse_x) - static_cast<double>(window_width)) * (1.0 - zoom_amount)) / 2.0;
				double delta_y = zoom * ((2.0 * static_cast<double>(mouse_y) - static_cast<double>(window_height)) * (1.0 - zoom_amount)) / 2.0;

				if (event.wheel.y > 0) // Mouse wheel up, zoom in
				{
					position_x += delta_x;
					position_y += delta_y;

					zoom *= zoom_amount;
				}
				else if (event.wheel.y < 0) // Mouse wheel down, zoom out
				{
					zoom /= zoom_amount;

					position_x -= delta_x;
					position_y -= delta_y;
				}

				regenerate = true;
			}
		}

		if (!io.WantCaptureMouse && left_mouse_down && (mouse_x != prev_mouse_x || mouse_y != prev_mouse_y))
		{
			Pan(prev_mouse_x - mouse_x, prev_mouse_y - mouse_y);
		}

		prev_mouse_x = mouse_x;
		prev_mouse_y = mouse_y;

		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		Menu();

		ImGui::Render();

		if (regenerate)
			Generate();

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		SDL_RenderCopy(renderer, texture, nullptr, nullptr);

		if(menu_open)
			ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

		SDL_RenderPresent(renderer);
	}
}