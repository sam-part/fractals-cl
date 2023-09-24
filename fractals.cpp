#include "fractals.h"

const std::vector<Fractal> fractals = {
	Fractal {
		"Mandelbrot Set",
		"fractals/mandelbrot.cl",
		-0.35,
		0.0,
		0.0035
	},

	Fractal {
		"Mandelbrot Set (Eclipse)",
		"fractals/eclipse.cl",
		-0.35,
		0.0,
		0.0035,
		{
			FractalVariable {"zx",    -0.7, -2.0, 2.0},
			FractalVariable {"zy", 0.27015, -2.0, 2.0}
		}
	},

	Fractal {
		"Julia Set",
		"fractals/julia.cl",
		0.0,
		0.0,
		0.003,
		{
			FractalVariable {"cx", -0.7,    -1.5, 1.5},
			FractalVariable {"cy", 0.27015, -1.5, 1.5}
		}
	},

	Fractal {
		"Burning Ship Fractal",
		"fractals/burning_ship.cl",
		-0.35,
		-0.7,
		0.0035
	}
};

Fractal GetFractal(FractalType type)
{
	return fractals[static_cast<size_t>(type)];
}