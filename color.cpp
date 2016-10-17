#include <cmath>
#include "color.h"

namespace mandelbrot
{
	uint32_t get_rgb(int r, int g, int b)
	{
		return r << 16 | g << 8 | b;
	}

	uint32_t hsv_to_rgb(int h, double s, double v)
	{
		if(v > 1.0) v = 1.0;
		int hp = h / 60;
		double c = v * s;
		double x = c * (1 - std::abs((hp & 1) - 1));

		auto rgb = [=] (double r, double g, double b) -> uint32_t
		{
			double m = v - c;
			return get_rgb((r + m) * 255, (g + m) * 255, (b + m) * 255);
		};

		if(hp < 1)      return rgb(c, x, 0);
		else if(hp < 2) return rgb(x, c, 0);
		else if(hp < 3) return rgb(0, c, x);
		else if(hp < 4) return rgb(0, x, c);
		else if(hp < 5) return rgb(x, 0, c);
		else            return rgb(c, 0, x);
	}

	uint32_t color_scale_gray(int iter, int max_iter, double smooth)
	{
		if(iter > max_iter) return 0;
		int x = 255 * smooth / max_iter;
		return get_rgb(x, x, x);
	}

	uint32_t color_fixed_gray(int iter, int max_iter, double smooth)
	{
		const int ruler = 150;
		if(iter > max_iter) return 0;
		// ruler * (1, 3, 5, ...)
		int s = sqrt(smooth / ruler);
		int base = s * s * ruler;
		int x = 255.0 * (smooth - base) / ((2.0 * s + 1) * ruler);
		if(s & 1) x = 255.0 - x;
		return get_rgb(x, x, x);
	}

	uint32_t color_hsv1(int iter, int max_iter, double smooth)
	{
		if(iter > max_iter) return 0;

		return hsv_to_rgb(
			360.0 * smooth / max_iter,
			1.0, 
			10.0 * smooth / max_iter
		);
	}
}
