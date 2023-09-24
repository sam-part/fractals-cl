#define SDL_MAIN_HANDLED
#include "fractal_viewer.h"

int main(int argc, char* args[])
{
	FractalViewer fractal_viewer(1200, 720);
	if (!fractal_viewer.IsInitialized())
		return -1;

	fractal_viewer.Run();

	return 0;
}