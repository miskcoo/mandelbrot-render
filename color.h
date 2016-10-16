#ifndef __COLOR_H
#define __COLOR_H

#include <mpfr.h>
#include <stdint.h>
#include <functional>

namespace mandelbrot
{
	typedef std::function<uint32_t(int, int, double)> color_picker_t;

	/* 
	 * Convert RGB to integer representation
	 *
	 * Input ranges:
	 *   0 <= R, G, B <= 255
	 */
	uint32_t get_rgb(int r, int g, int b);

	/* 
	 * Convert hue-saturation-value/luminosity to RGB
	 *
	 * Input ranges:
	 *   0   <= H    <= 360
	 *   0.0 <= S, V <= 1.0
	 */
	uint32_t hsv_to_rgb(int h, double s, double v);

	uint32_t color_scale_gray(int, int, double);
	uint32_t color_fixed_gray(int, int, double);
	uint32_t color_hsv1(int, int, double);
}

#endif // __COLOR_H
