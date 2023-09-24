inline int get_color(int num_iterations, int max_iterations, double zx, double zy, constant float* color_settings)
{
	num_iterations = pow((float)num_iterations / (float)max_iterations, color_settings[0]) * (float)max_iterations;
	float modulus = sqrt(zx * zx + zy * zy);
	float mu = num_iterations - (log(log(modulus))) / log(2.0f);
	float t = mu / max_iterations;

	int r = (int)((9.0f * (1.0 - t) * t * t * t * 255) + color_settings[1]) % 255;
	int g = (int)((15.0f * (1.0 - t) * (1.0 - t) * t * t * 255) + color_settings[2]) % 255;
	int b = (int)((8.5f * (1.0 - t) * (1.0 - t) * (1.0 - t) * t * 255) + color_settings[3]) % 255;
	
	return r | (g << 8) | (b << 16);
}