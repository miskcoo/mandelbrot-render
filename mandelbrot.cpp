#include <gmp.h>
#include "mandelbrot.h"
#include "color.h"

namespace mandelbrot
{
	void render_t::init()
	{
		mpf_inits(x0, y0, step, NULL);
	}

	void render_t::clear()
	{
		mpf_clears(x0, y0, step, NULL);
	}

	void render(
		render_t ri,
		render_callback_t callback,
		render_control_t  control)
	{
		mpf_t Re, Im, Re2, Im2, T, c_imag, c_real;
		mpf_inits(Re, Im, Re2, Im2, T, NULL);

		// initialize parameter c
		mpf_init_set(c_real, ri.x0);
		mpf_init_set(c_imag, ri.y0);

		for(int i = 0; i != ri.height; ++i)
		{
			if(control && !control(i)) 
			{
				mpf_add(c_imag, c_imag, ri.step);
				continue;
			}

			for(int j = 0; j != ri.width; ++j)
			{
				// initialize 
				mpf_set_d(Im, 0.0);
				mpf_set_d(Re, 0.0);
				mpf_set_d(Im2, 0.0);
				mpf_set_d(Re2, 0.0);

				int iter = 1;
				for(; iter <= ri.max_iter; ++iter)
				{
					// z_{n + 1} = z_n^2 + c;

					// calculate the imagine part of z
					mpf_mul      (Im, Im, Re);
					mpf_mul_2exp (Im, Im, 1);
					mpf_add      (Im, Im, c_imag);

					// calculate  the real part of z
					mpf_sub      (Re, Re2, Im2);
					mpf_add      (Re, Re, c_real);

					// calculate  the square
					mpf_mul      (Re2, Re, Re);
					mpf_mul      (Im2, Im, Im);

					// check for exit
					mpf_add      (T, Re2, Im2);
					if(mpf_cmp_ui(T, 4) > 0) 
						break;
				}

				ri.buffer[ i * ri.width + j ] = iter;

				// calculate next parameter c
				mpf_add(c_real, c_real, ri.step);
			}

			// calculate next parameter c
			mpf_set(c_real, ri.x0);
			mpf_add(c_imag, c_imag, ri.step);
			if(callback) callback(i);
		}

		mpf_clears(Re, Im, Re2, Im2, T, c_imag, c_real, NULL);
	}

	void color_gray(int size, uint32_t *dest, int *src)
	{
		for(int i = 0; i != size; ++i)
		{
			int c = 256 - src[i];
			dest[i] = get_rgb(c, c, c);
		}
	}
}
