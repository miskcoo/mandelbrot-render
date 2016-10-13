#ifndef __MANDELBROT_H
#define __MANDELBROT_H

#include <mpfr.h>
#include <stdint.h>
#include <functional>
#include "color.h"

#define MPFR_NULL ((mpfr_ptr)0)

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
		// precision of floating-point variables
		double prec;
		// coordinate of left-top point
		mpfr_t x0, y0;
		// distance between two adjacent point
		mpfr_t step;

		void init();
		void clear();
	};

	void render(
		render_t render_info, 
		color_picker_t color_picker, 
		render_callback_t = nullptr, 
		render_control_t  = nullptr);

}

#endif // __MANDELBROT_H
