#include "color.h"

namespace mandelbrot
{
	uint32_t get_rgb(int r, int g, int b)
	{
		return r << 16 | g << 8 | b;
	}
}
