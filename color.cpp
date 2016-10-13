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

	inline double smooth_color(int n, mpfr_t T)
	{
		using std::log;
		static const double inv_log2 = 1.0 / log(2.0);
		double t = mpfr_get_d(T, MPFR_RNDN);
		return 2 + n - log(log(t)) * inv_log2;
	}

	uint32_t color_scale_gray(int iter, int max_iter, mpfr_t, mpfr_t, mpfr_t T)
	{
		if(iter > max_iter) return 0;
		double v = smooth_color(iter, T);
		int x = 255 * v / max_iter;
		return get_rgb(x, x, x);
	}

	uint32_t color_hsv1(int iter, int max_iter, mpfr_t, mpfr_t, mpfr_t T)
	{
		if(iter > max_iter) return 0;

		double v = smooth_color(iter, T);
		return hsv_to_rgb(360.0 * v / max_iter, 1.0, 10.0 * v / max_iter);
	}
}
