#include "splashscreen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <cairo/cairo-xlib.h>

#include "splashscreen_common.h"

static volatile int done = 0;

bool show_splashscreen(const char * title, const char * background, const char * animation, const char * icon)
{
	int width = DEFAULT_WINDOW_WIDTH;
	int height = DEFAULT_WINDOW_HEIGHT;

	if (background)
	{
		load_background(background, &width, &height);
	}

	if (animation)
	{
		load_animation(animation, ANIM_W, ANIM_H);
	}

	(void)icon; // TODO

	Display * d = XOpenDisplay(NULL);

	if (!d)
	{
		fprintf(stderr, "Cannot open display\n");
		return false;
	}

	int s = DefaultScreen(d);

	Window w = XCreateSimpleWindow(d, RootWindow(d, s), 10, 10, width, height, 1, BlackPixel(d, s), BlackPixel(d, s));

	XSelectInput(d, w, ExposureMask | KeyPressMask);

	const char * names [] =
	{
		"_MOTIF_WM_HINTS",
		"_NET_WM_WINDOW_TYPE",
		"_NET_WM_WINDOW_TYPE_SPLASH",
		"_NET_WM_NAME",
		"_NET_WM_VISIBLE_NAME",
		"_NET_WM_STATE",
		"_NET_WM_STATE_STICKY",
		"_NET_WM_STATE_ABOVE",
		"UTF8_STRING"
	};

	Atom properties [sizeof(names)/sizeof(*names)];

	for (unsigned int i = 0; i < sizeof(names)/sizeof(*names); ++i)
	{
		properties[i] = XInternAtom(d, names[i], true);
	}

	typedef struct MotifHints
	{
		uint32_t flags;
		uint32_t functions;
		uint32_t decorations;
		int32_t  input_mode;
		uint32_t status;
	} Hints;

	Hints hints;
	hints.flags = 2;
	hints.functions = 0;
	hints.decorations = 0;
	hints.input_mode = 0;
	hints.status = 0;

	/* Set window hints */
	XChangeProperty(d, w, properties[0], XA_INTEGER, 32, PropModeReplace, (void*)&hints, 5);

	/* Set the title of the window */
	XTextProperty text_property;
	if (XStringListToTextProperty((char**)&title, 1, &text_property))
		XSetWMName(d, w, &text_property);
	XChangeProperty(d, w, properties[3], properties[8], 8, PropModeReplace, (void*)title, strlen(title));
	XChangeProperty(d, w, properties[4], properties[8], 8, PropModeReplace, (void*)title, strlen(title));

	/* Set the title of the window icon */
	//xcb_change_property(c, XCB_PROP_MODE_REPLACE, w, XCB_ATOM_WM_ICON_NAME, XCB_ATOM_STRING, 8, strlen(title_icon), title_icon);

	/* Set window type */
	XChangeProperty(d, w, properties[1], XA_ATOM, 32, PropModeReplace, (void*)&properties[2], 1);

	/* Set window state */
	XChangeProperty(d, w, properties[5], XA_ATOM, 32, PropModeReplace, (void*)&properties[6], 1);
	XChangeProperty(d, w, properties[5], XA_ATOM, 32, PropModeReplace, (void*)&properties[7], 1);

	XMapWindow(d, w);

	XFlush(d);

	Visual * visual = DefaultVisual(d, s);
	if (visual == NULL)
	{
		fprintf(stderr, "Some weird internal error...?!");
		XCloseDisplay(d);
		return false;
	}

	cairo_surface_t * surface = cairo_xlib_surface_create(d, w, visual, width, height);
	cairo_t * cr = cairo_create(surface);
	XFlush(d);

	while (!done)
	{
		bool bExposeEventReceived = false;

		while (XPending(d))
		{
			XEvent event;
			XNextEvent(d, &event);

			if (event.type == Expose)
			{
				bExposeEventReceived = true;
			}
		}

		bool bAnimUpdated = update_animation();

		if (bExposeEventReceived || bAnimUpdated)
		{
			redraw(cr, surface, width, height);
			XFlush(d);
		}

#if 0
		if (xcb_connection_has_error(c))
		{
			done = 0;
		}
#endif
	}

	cairo_destroy(cr);
	cairo_surface_destroy(surface);

	XCloseDisplay(d);

	return true;
}

bool hide_splashscreen(void)
{
	done = 1;
	return true;
}
