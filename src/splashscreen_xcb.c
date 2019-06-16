#include "splashscreen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>

#include <cairo/cairo-xcb.h>

#include "splashscreen_common.h"

static volatile int done = 0;

static xcb_visualtype_t * find_visual(xcb_connection_t *c, xcb_visualid_t visual)
{
	xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(xcb_get_setup(c));

	for (; screen_iter.rem; xcb_screen_next(&screen_iter))
	{
		xcb_depth_iterator_t depth_iter = xcb_screen_allowed_depths_iterator(screen_iter.data);
		for (; depth_iter.rem; xcb_depth_next(&depth_iter))
		{
			xcb_visualtype_iterator_t visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
			for (; visual_iter.rem; xcb_visualtype_next(&visual_iter))
			{
				if (visual == visual_iter.data->visual_id)
				{
					return visual_iter.data;
				}
			}
		}
	}

	return NULL;
}

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

	/* open connection with the server */

	xcb_connection_t * c = xcb_connect (NULL, NULL);

	if (xcb_connection_has_error(c) > 0)
	{
		fprintf(stderr, "Cannot open display\n");
		return false;
	}

	xcb_screen_t * s = xcb_setup_roots_iterator(xcb_get_setup (c)).data;

	if (s == NULL)
	{
		fprintf(stderr, "No screen\n");
		xcb_disconnect(c);
		return false;
	}

	/* create window */
	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_BORDER_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t values[3] =
	{
		0,
		0,
		XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS
	};

	xcb_window_t w = xcb_generate_id(c);

	xcb_create_window(c, XCB_COPY_FROM_PARENT, w, s->root, 10, 10, (uint16_t)width, (uint16_t)height, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT, s->root_visual, mask, values);

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

	xcb_intern_atom_cookie_t cookies [sizeof(names)/sizeof(*names)];
	xcb_intern_atom_reply_t * replies [sizeof(names)/sizeof(*names)];

	for (unsigned int i = 0; i < sizeof(names)/sizeof(*names); ++i)
	{
		cookies[i] = xcb_intern_atom(c, 0, strlen(names[i]), names[i]);
	}

	for (unsigned int i = 0; i < sizeof(names)/sizeof(*names); ++i)
	{
		replies[i] = xcb_intern_atom_reply(c, cookies[i], NULL);
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
	xcb_change_property(c, XCB_PROP_MODE_REPLACE, w, replies[0]->atom, XCB_ATOM_INTEGER, 32, 5, &hints);

	/* Set the title of the window */
	xcb_change_property(c, XCB_PROP_MODE_REPLACE, w, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(title), title);
	xcb_change_property(c, XCB_PROP_MODE_REPLACE, w, replies[3]->atom, replies[8]->atom, 8, strlen(title), title);
	xcb_change_property(c, XCB_PROP_MODE_REPLACE, w, replies[4]->atom, replies[8]->atom, 8, strlen(title), title);

	/* Set the title of the window icon */
	//xcb_change_property(c, XCB_PROP_MODE_REPLACE, w, XCB_ATOM_WM_ICON_NAME, XCB_ATOM_STRING, 8, strlen(title_icon), title_icon);

	/* Set window type */
	xcb_change_property(c, XCB_PROP_MODE_REPLACE, w, replies[1]->atom, XCB_ATOM_ATOM, 32, 1, &replies[2]->atom);

	/* Set window state */
	xcb_change_property(c, XCB_PROP_MODE_REPLACE, w, replies[5]->atom, XCB_ATOM_ATOM, 32, 1, &replies[6]->atom);
	xcb_change_property(c, XCB_PROP_MODE_REPLACE, w, replies[5]->atom, XCB_ATOM_ATOM, 32, 1, &replies[7]->atom);

	xcb_map_window(c, w);

	xcb_flush(c);

	xcb_visualtype_t * visual = find_visual(c, s->root_visual);
	if (visual == NULL)
	{
		fprintf(stderr, "Some weird internal error...?!");
		xcb_disconnect(c);
		return false;
	}

	cairo_surface_t * surface = cairo_xcb_surface_create(c, w, visual, width, height);
	cairo_t * cr = cairo_create(surface);
	xcb_flush(c);

	while (!done)
	{
		bool bExposeEventReceived = false;

		xcb_generic_event_t * event;

		while ((event = xcb_poll_for_event(c)) != NULL)
		{
			if (event->response_type == XCB_EXPOSE)
			{
				bExposeEventReceived = true;
			}

			free(event);
		}

		bool bAnimUpdated = update_animation();

		if (bExposeEventReceived || bAnimUpdated)
		{
			redraw(cr, surface, width, height);
			xcb_flush(c);
		}

		if (xcb_connection_has_error(c))
		{
			done = 0;
		}
	}

	cairo_destroy(cr);
	cairo_surface_destroy(surface);

	xcb_disconnect (c);

	return true;
}

bool hide_splashscreen(void)
{
	done = 1;
	return true;
}
