#include <bits/stdc++.h>
#include <cairo.h>
#include <gtk/gtk.h>
#include "mandelbrot.h"
#include "color.h"

using namespace mandelbrot;

const int FLUSH_TIMEOUT = 50;
const int IMG_SIZE = 600, MAX_ITER = 255;
GtkWidget *window, *draw_area;
int T[IMG_SIZE][IMG_SIZE];
uint32_t G[IMG_SIZE][IMG_SIZE];
render_t ri;
std::atomic<bool> expose_required;

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

void update_image(mouse_state_t ms)
{
	if(ms.x < ms.px) std::swap(ms.x, ms.px);
	if(ms.y < ms.py) std::swap(ms.y, ms.py);

	int size = std::min(ms.x - ms.px, ms.y - ms.py);
	if(size < 3) return;
	int x = (ms.x + ms.px - size) / 2;
	int y = (ms.y + ms.py - size) / 2;

	ri.prec += std::log2(1.0 * IMG_SIZE / size + 0.01);
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
		render(ri, 
		color_scale_gray, 
		[=](int row) {
			std::copy(T[row], T[row] + IMG_SIZE, G[row]);
			expose_required = true;
		},
		[=](int row) -> bool {
			return row % thread_num == id;
		} );
	};

	for(int i = 0; i != 3; ++i)
		std::thread(calc_procdure, ri, i, 3).detach();
}

gboolean expose_event(
	GtkWidget *widget,
	GdkEventExpose *event, 
	gpointer data) 
{
	cairo_t *cr;

	cr = gdk_cairo_create(widget->window);

	int stride = cairo_format_stride_for_width(
			CAIRO_FORMAT_RGB24, IMG_SIZE);

	cairo_surface_t *surface = 
		cairo_image_surface_create_for_data(
			 (unsigned char*)G, 
			CAIRO_FORMAT_RGB24, 
			IMG_SIZE,
			IMG_SIZE,
			stride
		);

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
	if(event->type == GDK_BUTTON_PRESS)
	{
		ms.pressed = true;
		ms.x = ms.px = event->x;
		ms.y = ms.py = event->y;
	} else if(event->type == GDK_BUTTON_RELEASE) {
		ms.pressed = false;
		update_image(ms);
		gtk_widget_queue_draw(widget);

		char title[1024];
		std::sprintf(title, "Mandelbrot Set ( prec = %.5lf )", ri.prec);
		gtk_window_set_title(GTK_WINDOW(window), title);
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
	ri.max_iter  = MAX_ITER;
	ri.prec      = 10;
	mpfr_init_set_d(ri.x0,   -2.0,           MPFR_RNDN);
	mpfr_init_set_d(ri.y0,   -2.0,           MPFR_RNDN);
	mpfr_init_set_d(ri.step, 4.0 / IMG_SIZE, MPFR_RNDN);

	mouse_state_t ms0;
	ms0.px = ms0.py = 0;
	ms0.x  = ms0.y  = IMG_SIZE;
	update_image(ms0);
}


int main(int argc, char *argv[])
{
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

	// set window info
	gtk_window_set_title(GTK_WINDOW(window), "Mandelbrot Set Render");
	gtk_window_set_default_size(GTK_WINDOW(window), IMG_SIZE, IMG_SIZE);

	// set flush timer
	g_timeout_add(FLUSH_TIMEOUT, (GSourceFunc)expose_checker, NULL);

	init_image();
	gtk_widget_show_all(window);
	gtk_main();
	return 0;
}
