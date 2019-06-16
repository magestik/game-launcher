#ifndef SPLASHSCREEN_COMMON_H
#define SPLASHSCREEN_COMMON_H

#include <inttypes.h>
#include <stdbool.h>
#include <cairo/cairo.h>

void load_background(const char * path, int * width, int * height);
void load_animation(const char * path, int width, int height);
bool update_animation(void);
void redraw(cairo_t * cr, cairo_surface_t * surface, int width_, int height_);

#define ANIM_W (72)
#define ANIM_H (72)

#define DEFAULT_WINDOW_WIDTH (640)
#define DEFAULT_WINDOW_HEIGHT (480)

#endif // SPLASHSCREEN_COMMON_H
