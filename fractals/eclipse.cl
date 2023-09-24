void kernel calculate(global int* out, constant int* fractal_settings, constant double* position, constant float* color_settings, constant double* args) 
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	
	int width = fractal_settings[1];
	int height = fractal_settings[2];
	
	double x_scaled = (x - width / 2.0) * position[0] + position[1];
	double y_scaled = (y - height / 2.0) * position[0] + position[2];
	
	int max_iterations = fractal_settings[0];
	int num_iterations = 0;
	
	double zx = args[0];
	double zy = args[1];
	double zx2 = 0.0;
	double zy2 = 0.0;
	
    double x_temp = 0.0;
	while (zx2 + zy2 <= 4.0 && num_iterations < max_iterations)
	{
		zy = 2.0 * zx * zy + y_scaled;
		zx = zx2 - zy2 + x_scaled;
		zx2 = zx * zx;
		zy2 = zy * zy;
		num_iterations++;
	}
	
    if(num_iterations == max_iterations)
    {
		out[y * width + x] = 0;
    }
    else
    {
		out[y * width + x] = get_color(num_iterations, max_iterations, zx, zy, color_settings);
    }
}