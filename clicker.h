#ifndef __clicker_h__
#define __clicker_h__

#include <X11/Xlib.h>
#include <glib.h>

typedef struct Clicker {
	GRWLock lock;
	gchar *displayname;
	guint initial_delay_ms;
	guint mousebutton;
	guint mousebutton_interval_ms;
	guint mousebutton_count;
	gchar *keypress;
	guint keypress_interval_ms;
	guint keypress_count;
	Display *display;
	GMainLoop *main;
	guint mousebutton_event_id;
	guint keypress_event_id;
	guint pause_event_id;
	guint sigint_event_id;
	guint sighup_event_id;
	guint sigterm_event_id;
} t_clicker;

int start_events(t_clicker *ctx);

#endif /* __clicker_h__ */
