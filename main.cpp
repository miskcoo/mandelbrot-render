#include <bits/stdc++.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include "mandelbrot.h"
#include "color.h"

using namespace mandelbrot;

const int FLUSH_TIMEOUT = 50;
const int IMG_SIZE = 600, MAX_ITER = 500;
GtkWidget *window, *draw_area;
int THREAD_NUM = 4;
int T[IMG_SIZE][IMG_SIZE];
double S[IMG_SIZE][IMG_SIZE];
uint32_t G[IMG_SIZE][IMG_SIZE];
render_t ri;
std::atomic<bool> expose_required;
std::atomic<int> working_thread, current_iter_depth;

struct mouse_state_t
{
	bool pressed;
	int x, y, px, py;
} ms;

void mpfr_extend_prec(mpfr_t rop, mpfr_t tmp, mpfr_prec_t prec)
{
	mpfr_set(tmp, rop, MPFR_RNDN);
	mpfr_set_prec(rop, prec);
	mpfr_set(rop, tmp, MPFR_RNDN);
}

void update_window_title()
{
	static char title[1024];
	double x0 = mpfr_get_d(ri.x0, MPFR_RNDN);
	double y0 = mpfr_get_d(ri.y0, MPFR_RNDN);
	double s0 = mpfr_get_d(ri.step, MPFR_RNDN);
	sprintf(title, "Mandelbrot Set ( prec = %.5lf, iter = %d ) at (%.6lf, %.6lf)",
			ri.prec, ri.max_iter, x0 + s0 * IMG_SIZE / 2, y0 + s0 * IMG_SIZE / 2);
	gtk_window_set_title(GTK_WINDOW(window), title);
}

void update_image(mouse_state_t ms)
{
	if(ms.x < ms.px) std::swap(ms.x, ms.px);
	if(ms.y < ms.py) std::swap(ms.y, ms.py);

	int size = std::min(ms.x - ms.px, ms.y - ms.py);
	if(size < 3) return;
	int x = (ms.x + ms.px - size) / 2;
	int y = (ms.y + ms.py - size) / 2;

	ri.prec += log2(1.0 * IMG_SIZE / size + 0.01);
	mpfr_prec_t prec = ri.prec * 2;

	// initialize temporary variable
	mpfr_t tmp;
	mpfr_init2  (tmp, prec);

	// reset the precision 
	mpfr_extend_prec(ri.x0,   tmp, prec);
	mpfr_extend_prec(ri.y0,   tmp, prec);
	mpfr_extend_prec(ri.step, tmp, prec);

	// update left-top point
	mpfr_mul_ui (tmp, ri.step, x,   MPFR_RNDN);
	mpfr_add    (ri.x0, ri.x0, tmp, MPFR_RNDN);

	mpfr_mul_ui (tmp, ri.step, y,   MPFR_RNDN);
	mpfr_add    (ri.y0, ri.y0, tmp, MPFR_RNDN);

	// update step
	mpfr_set_d  (tmp, 1.0 * size / IMG_SIZE, MPFR_RNDN);
	mpfr_mul    (ri.step, ri.step, tmp,      MPFR_RNDN);

	mpfr_clear  (tmp);

	// render image
	auto calc_procdure = [] (render_t ri, int id, int thread_num) {
		int cur_iter = render(ri, 
		[=](int row) {
			for(int i = 0; i != ri.width; ++i)
				G[row][i] = color_fixed_gray(T[row][i], ri.max_iter, S[row][i]);
			expose_required = true;
		},
		[=](int row) -> bool {
			return row % thread_num == id;
		} );

		if(cur_iter > current_iter_depth)
			current_iter_depth = cur_iter;

		if(--working_thread == 0 && current_iter_depth > ri.max_iter * 0.7)
			::ri.max_iter = current_iter_depth * 2;
	};

	working_thread = THREAD_NUM;
	for(int i = 0; i != THREAD_NUM; ++i)
		std::thread(calc_procdure, ri, i, THREAD_NUM).detach();
}

cairo_surface_t *cairo_get_surface()
{
	int stride = cairo_format_stride_for_width(
			CAIRO_FORMAT_RGB24, IMG_SIZE);

	return cairo_image_surface_create_for_data(
		 (unsigned char*)G, 
		CAIRO_FORMAT_RGB24, 
		IMG_SIZE,
		IMG_SIZE,
		stride
	);
}

gboolean on_key_press(
	GtkWidget *widget, 
	GdkEventKey *event, 
	gpointer data)
{
	if(event->keyval == GDK_s && (event->state & GDK_CONTROL_MASK))
	{
		cairo_surface_t *surface = cairo_get_surface();
		cairo_surface_write_to_png(surface, "mandelbrot.png");
		cairo_surface_destroy(surface);
	}

	return TRUE;
}

gboolean expose_event(
	GtkWidget *widget,
	GdkEventExpose *event, 
	gpointer data) 
{
	cairo_t *cr;

	cr = gdk_cairo_create(widget->window);

	cairo_surface_t *surface = cairo_get_surface();
	cairo_set_source_surface(cr, surface, 0, 0);
	cairo_paint(cr);
	cairo_surface_destroy(surface); 

	// draw mouse rectangle
	if(ms.pressed)
	{
		double dashes[] = { 3.0, 1.0, 3.0 };
		cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.9);
		cairo_set_dash(cr, dashes, 3, 0.0);
		cairo_set_line_width(cr, 0.8);
		cairo_rectangle(
			cr, 
			std::min(ms.x, ms.px),
			std::min(ms.y, ms.py),
			std::abs(ms.x - ms.px),
			std::abs(ms.y - ms.py)
		);

		cairo_stroke(cr);
	}

	cairo_destroy(cr);

	return FALSE;
}

void destroy_event()
{
	ri.clear();
	gtk_main_quit();
}

void mouse_hit_event(
	GtkWidget *widget,
	GdkEventButton *event, 
	gpointer data)
{
	if(event->type == GDK_BUTTON_PRESS && !working_thread)
	{
		ms.pressed = true;
		ms.x = ms.px = event->x;
		ms.y = ms.py = event->y;
	} else if(event->type == GDK_BUTTON_RELEASE && ms.pressed) {
		ms.pressed = false;
		update_image(ms);
		gtk_widget_queue_draw(widget);
		update_window_title();
	}
}

void mouse_motion_event(
	GtkWidget *widget,
	GdkEventMotion *event, 
	gpointer data)
{
	ms.x = event->x;
	ms.y = event->y;
	gtk_widget_queue_draw(widget);
}

gboolean expose_checker(gpointer)
{
	if(expose_required)
	{
		gtk_widget_queue_draw(window);
		expose_required = false;
	}

	return TRUE;
}

void init_image()
{
	// initialize image
	ri.width     = ri.height = IMG_SIZE;
	ri.buffer    = (int*)T;
	ri.smooth    = (double*)S;
	ri.max_iter  = MAX_ITER;
	ri.prec      = 10;
	mpfr_init_set_d(ri.x0,   -2.5,           MPFR_RNDN);
	mpfr_init_set_d(ri.y0,   -2.0,           MPFR_RNDN);
	mpfr_init_set_d(ri.step, 4.0 / IMG_SIZE, MPFR_RNDN);

	mouse_state_t ms0;
	ms0.px = ms0.py = 0;
	ms0.x  = ms0.y  = IMG_SIZE;
	update_window_title();
	update_image(ms0);
}

void parse_arguments(int argc, char *argv[])
{
	for(int i = 1; i != argc; ++i)
	{
		int len = std::strlen(argv[i]);
		if(len >= 3 && argv[i][0] == '-' && argv[i][1] == 'j')
		{
			int t = std::atoi(argv[i] + 2);
			if(t >= 1) THREAD_NUM = t;
		}
	}
}

int main(int argc, char *argv[])
{
	parse_arguments(argc, argv);
	gtk_init(&argc, &argv);

	// create window
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(window), "destroy",
			G_CALLBACK(destroy_event), NULL);

	// create drawing area
	draw_area = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(window), draw_area);
	g_signal_connect(G_OBJECT(draw_area), "expose-event",
			G_CALLBACK(expose_event), NULL);

	// add mouse event
	gtk_widget_add_events(window, 
			GDK_BUTTON_PRESS_MASK
		  | GDK_BUTTON_RELEASE_MASK
		  | GDK_POINTER_MOTION_MASK
	);
	g_signal_connect(G_OBJECT(window), "button-press-event", 
			G_CALLBACK(mouse_hit_event), NULL);
	g_signal_connect(G_OBJECT(window), "button-release-event", 
			G_CALLBACK(mouse_hit_event), NULL);
	g_signal_connect(G_OBJECT(window), "motion-notify-event", 
			G_CALLBACK(mouse_motion_event), NULL);
	g_signal_connect(G_OBJECT(window), "key-press-event", 
			G_CALLBACK(on_key_press), NULL);

	// set window info
	gtk_window_set_default_size(GTK_WINDOW(window), IMG_SIZE, IMG_SIZE);

	// set flush timer
	g_timeout_add(FLUSH_TIMEOUT, (GSourceFunc)expose_checker, NULL);

	init_image();
	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}
