void kernel calculate(global int* out, constant int* fractal_settings, constant double* position, constant float* color_settings, constant double* args) 
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	
	int width = fractal_settings[1];
	int height = fractal_settings[2];
	
	double zx = (x - width / 2.0) * position[0] + position[1];
	double zy = (y - height / 2.0) * position[0] + position[2];
	
	double cx = args[0];
	double cy = args[1];
	
	int max_iterations = fractal_settings[0];
	int num_iterations = 0;
	
    double x_temp = 0.0;
	while (zx * zx + zy * zy <= 4.0 && num_iterations < max_iterations)
	{
		x_temp = zx * zx - zy * zy + cx;
		zy = 2.0 * zx * zy + cy;
		zx = x_temp;
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