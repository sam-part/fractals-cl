void kernel calculate(global int* out, constant int* fractal_settings, constant double* position, constant float* color_settings) 
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	
	int width = fractal_settings[1];
	int height = fractal_settings[2];
	
	double x_scaled = (x - width / 2.0) * position[0] + position[1];
	double y_scaled = (y - height / 2.0) * position[0] + position[2];
	
	int max_iterations = fractal_settings[0];
	int num_iterations = 0;

	double zx = 0.0;
	double zy = 0.0;
    double x_temp = 0.0;
	while (zx * zx + zy * zy <= 4.0 && num_iterations < max_iterations)
	{
		x_temp = zx * zx - zy * zy + x_scaled;
		zy = fabs(2.0 * zx * zy + y_scaled);
		zx = fabs(x_temp);
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