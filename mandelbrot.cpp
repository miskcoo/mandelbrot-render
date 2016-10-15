#include "mandelbrot.h"
#include "color.h"

namespace mandelbrot
{
	void render_t::init()
	{
		mpfr_inits(x0, y0, step, MPFR_NULL);
	}

	void render_t::clear()
	{
		mpfr_clears(x0, y0, step, MPFR_NULL);
	}

	void render(
		render_t          ri,
		color_picker_t    color_picker,
		render_callback_t callback,
		render_control_t  control)
	{
		// set presison for calculation
		mpfr_prec_t prec = std::max(ri.prec + 20, ri.prec * 1.15);
		mpfr_set_default_prec(prec);

		mpfr_t Re, Im, Re2, Im2, T, c_imag, c_real;
		mpfr_inits(Re, Im, Re2, Im2, T, MPFR_NULL);

		// initialize parameter c
		mpfr_init_set(c_real, ri.x0, MPFR_RNDN);
		mpfr_init_set(c_imag, ri.y0, MPFR_RNDN);

		auto iterate_mpfr = [&]() -> int
		{
			// initialize 
			mpfr_set_ui(Im,  0, MPFR_RNDN);
			mpfr_set_ui(Re,  0, MPFR_RNDN);
			mpfr_set_ui(Im2, 0, MPFR_RNDN);
			mpfr_set_ui(Re2, 0, MPFR_RNDN);

			int iter = 1;
			for(; iter <= ri.max_iter; ++iter)
			{
				// z_{n + 1} = z_n^2 + c;

				// calculate the imagine part of z
				mpfr_mul      (Im, Im, Re,     MPFR_RNDN);
				mpfr_mul_2exp (Im, Im, 1,      MPFR_RNDN);
				mpfr_add      (Im, Im, c_imag, MPFR_RNDN);

				// calculate  the real part of z
				mpfr_sub      (Re, Re2, Im2,   MPFR_RNDN);
				mpfr_add      (Re, Re, c_real, MPFR_RNDN);

				// calculate  the square
				mpfr_mul      (Re2, Re, Re,    MPFR_RNDN);
				mpfr_mul      (Im2, Im, Im,    MPFR_RNDN);

				// check for exit
				mpfr_add      (T, Re2, Im2,    MPFR_RNDN);

				if(mpfr_cmp_ui(T, 4) > 0) 
					break;
			}

			return iter;
		};

		auto iterate_double = [&]() -> int
		{
			double im = 0, re = 0, im2 = 0, re2 = 0;
			double c_im = mpfr_get_d(c_imag, MPFR_RNDN);
			double c_re = mpfr_get_d(c_real, MPFR_RNDN);

			int iter = 1;
			for(; iter <= ri.max_iter; ++iter)
			{
				im  = 2.0 * im * re + c_im;
				re  = re2 - im2     + c_re;
				re2 = re * re;
				im2 = im * im;

				if(re2 + im2 > 4.0)
					break;
			}

			mpfr_set_d(Im2, im2, MPFR_RNDN);
			mpfr_set_d(Re2, re2, MPFR_RNDN);
			mpfr_set_d(T, re2 + im2, MPFR_RNDN);

			return iter;
		};

		for(int i = 0; i != ri.height; ++i)
		{
			if(control && !control(i)) 
			{
				mpfr_add(c_imag, c_imag, ri.step, MPFR_RNDN);
				continue;
			}

			for(int j = 0; j != ri.width; ++j)
			{
				int iter = ri.prec < 50 ? iterate_double() : iterate_mpfr();
				ri.buffer[ i * ri.width + j ] = color_picker(iter, ri.max_iter, Re2, Im2, T);

				// calculate next parameter c
				mpfr_add(c_real, c_real, ri.step, MPFR_RNDN);
			}

			// calculate next parameter c
			mpfr_set(c_real, ri.x0,           MPFR_RNDN);
			mpfr_add(c_imag, c_imag, ri.step, MPFR_RNDN);
			if(callback) callback(i);
		}

		mpfr_clears(Re, Im, Re2, Im2, T, c_imag, c_real, MPFR_NULL);
	}
}
