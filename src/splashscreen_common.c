#include "splashscreen_common.h"

#include "rlottie_capi.h"

#include <malloc.h>
#include <unistd.h>

static Lottie_Animation * animation = NULL;

static cairo_surface_t * cairo_surface = NULL;
static cairo_surface_t * cairo_background = NULL;

static int width = 0;
static int height = 0;
static int stride = 0;
static uint32_t * buffer = NULL;

void load_background(const char * path, int * width_, int * height_)
{
	cairo_background = cairo_image_surface_create_from_png(path);

	if (cairo_background)
	{
		*width_ = cairo_image_surface_get_width(cairo_background);
		*height_ = cairo_image_surface_get_height(cairo_background);
	}
}

void load_animation(const char * path, int width_, int height_)
{
	if (width_ > 0 && height_ > 0)
	{
		width = width_;
		height = height_;
		stride = width_ * sizeof(uint32_t);
		buffer = calloc(width*height, sizeof(uint32_t));
		animation = lottie_animation_from_file(path);
	}
}

bool update_animation()
{
	bool updated = false;
	useconds_t duration = 1000000 / 30;

	if (animation)
	{
		static size_t frame = 0;
		++frame;

		if (frame >= lottie_animation_get_totalframe(animation))
		{
			frame = 0; // loop
		}

		lottie_animation_render(animation, frame, buffer, (size_t)width, (size_t)height, (size_t)stride);

		cairo_surface_destroy(cairo_surface);
		cairo_surface = cairo_image_surface_create_for_data((unsigned char*)buffer, CAIRO_FORMAT_ARGB32, width, height, stride);

		updated = true;
		duration = (useconds_t)(1000000.0 / lottie_animation_get_framerate(animation));
	}

	usleep(duration); // hackish
	return updated;
}

void redraw(cairo_t * cr, cairo_surface_t * surface, int width_, int height_)
{
	cairo_push_group(cr);

	if (cairo_background)
	{
		cairo_set_source_surface(cr, cairo_background, 0, 0);
		cairo_paint(cr);
	}
	else
	{
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_paint(cr);
	}

	if (cairo_surface)
	{
		double pos_x = (width_ - cairo_image_surface_get_width(cairo_surface));
		double pos_y = (height_ - cairo_image_surface_get_height(cairo_surface));

		cairo_set_source_surface(cr, cairo_surface, pos_x, pos_y);
		cairo_paint(cr);
	}

	cairo_pop_group_to_source(cr);
	cairo_paint(cr);

	cairo_surface_flush(surface);
}
