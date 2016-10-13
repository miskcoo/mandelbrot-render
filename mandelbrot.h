#ifndef __MANDELBROT_H
#define __MANDELBROT_H

#include <gmp.h>
#include <stdint.h>
#include <functional>

namespace mandelbrot
{
	typedef std::function<void(int)> render_callback_t;
	typedef std::function<bool(int)> render_control_t;

	struct render_t
	{
		int width, height;
		// linearized height * width matrix
		// its value is the iteration depth
		int *buffer;
		// maximum iteration depth
		int max_iter;
		// coordinate of left-top point
		mpf_t x0, y0;
		// distance between two adjacent point
		mpf_t step;

		void init();
		void clear();
	};

	void render(
		render_t render_info, 
		render_callback_t = nullptr, 
		render_control_t  = nullptr);

	void color_gray(int size, uint32_t *dest, int *src);
}

#endif // __MANDELBROT_H
