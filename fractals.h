#pragma once

#include <string>
#include <vector>

struct FractalVariable
{
	std::string name {};
	const double default_value = 0.0;
	double min = 0.0;
	double max = 0.0;
	double value = 0.0;
};

struct Fractal
{
	std::string name {};
	std::string source_path {};
	double default_x_pos = 0.0;
	double default_y_pos = 0.0;
	double default_zoom  = 0.0;
	std::vector<FractalVariable> variables {};
};

enum struct FractalType
{
	Mandelbrot,
	Julia,
	BurningShip,
	Eclipse
};

extern const std::vector<Fractal> fractals;

Fractal GetFractal(FractalType type);