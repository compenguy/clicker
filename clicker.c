#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/extensions/XTest.h>
#include <X11/XKBlib.h>
#include <signal.h>
#include <glib-unix.h>
#include "clicker.h"

/********** FUNCTION PROTOTYPES ************/

/* Event handlers */
static gboolean mouse_event(gpointer user_data);
static gboolean keypress_event(gpointer user_data);
static gboolean pause_event(gpointer user_data);
static gboolean sigquit_handler(gpointer user_data);

/* Event enabler/disablers */
static int add_mouse_events(t_clicker *ctx);
static int remove_mouse_events(t_clicker *ctx);
static int add_key_events(t_clicker *ctx);
static int remove_key_events(t_clicker *ctx);

/* X11 display management */
static int init_display(t_clicker *ctx);
static int close_display(t_clicker *ctx);

/* Print a configuration banner for the user */
static void print_config(t_clicker *ctx);

/********** FUNCTION DEFINITION ************/
static gboolean mouse_event(gpointer user_data) {
	t_clicker *ctx = (t_clicker *)user_data;
	g_rw_lock_reader_lock(&ctx->lock);
	XTestFakeButtonEvent(ctx->display, ctx->mousebutton, 1, 0);
	XTestFakeButtonEvent(ctx->display, ctx->mousebutton, 0, 0);
	XFlush(ctx->display);
	g_rw_lock_reader_unlock(&ctx->lock);

	g_rw_lock_reader_lock(&ctx->lock);
	ctx->mousebutton_count++;
	g_rw_lock_reader_unlock(&ctx->lock);
	return G_SOURCE_CONTINUE;
}

static gboolean keypress_event(gpointer user_data) {
	t_clicker *ctx = (t_clicker *)user_data;
	g_rw_lock_reader_lock(&ctx->lock);
	guint keysym = XStringToKeysym(ctx->keypress);
	guint keycode = XKeysymToKeycode(ctx->display, keysym);

	XTestFakeKeyEvent(ctx->display, keycode, 1, 0);
	XTestFakeKeyEvent(ctx->display, keycode, 0, 0);
	XFlush(ctx->display);
	g_rw_lock_reader_unlock(&ctx->lock);

	g_rw_lock_reader_lock(&ctx->lock);
	ctx->keypress_count++;
	g_rw_lock_reader_unlock(&ctx->lock);
	return G_SOURCE_CONTINUE;
}

static gboolean pause_event(gpointer user_data) {
	t_clicker *ctx = (t_clicker *)user_data;
	guint indicators = 0;

	g_rw_lock_reader_lock(&ctx->lock);
	XkbGetIndicatorState(ctx->display, XkbUseCoreKbd, &indicators);
	g_rw_lock_reader_unlock(&ctx->lock);

	if ((indicators & 0x02) == 2) {
		g_print ("events running (numlock enabled). mouseclicks: %04d keypresses: %04d\n", ctx->mousebutton_count, ctx->keypress_count);
		add_mouse_events(ctx);
		add_key_events(ctx);
	} else {
		g_print ("events paused (numlock disabled).\n");
		remove_mouse_events(ctx);
		remove_key_events(ctx);
		g_rw_lock_writer_lock(&ctx->lock);
		ctx->mousebutton_count = 0;
		ctx->keypress_count = 0;
		g_rw_lock_writer_unlock(&ctx->lock);
	}
	return G_SOURCE_CONTINUE;
}

static gboolean sigquit_handler(gpointer user_data) {
	t_clicker *ctx = (t_clicker *)user_data;
	g_print ("Abort requested.\n");

	g_rw_lock_writer_lock(&ctx->lock);
	g_main_loop_quit(ctx->main);
	g_rw_lock_writer_unlock(&ctx->lock);

	return G_SOURCE_CONTINUE;
}

static int add_mouse_events(t_clicker *ctx) {
	g_rw_lock_writer_lock(&ctx->lock);
	if (ctx->mousebutton_event_id == 0 &&
	    ctx->mousebutton > 0 &&
	    ctx->mousebutton_interval_ms > 0) {
		g_print ("Starting mouse clicks...\n");
		ctx->mousebutton_event_id = g_timeout_add (ctx->mousebutton_interval_ms,
		                                           mouse_event,
		                                           ctx);
	}
	g_rw_lock_writer_unlock(&ctx->lock);
	return 0;
}

static int remove_mouse_events(t_clicker *ctx) {
	g_rw_lock_writer_lock(&ctx->lock);
	if (ctx->mousebutton_event_id > 0) {
		g_print ("Pausing mouse clicks...\n");
		g_source_remove(ctx->mousebutton_event_id);
		ctx->mousebutton_event_id = 0;
	}
	g_rw_lock_writer_unlock(&ctx->lock);
	return 0;
}

static int add_key_events(t_clicker *ctx) {
	g_rw_lock_writer_lock(&ctx->lock);
	if (ctx->keypress_event_id == 0 &&
	    ctx->keypress != NULL &&
	    ctx->keypress_interval_ms > 0) {
		g_print ("Starting key presses...\n");
		ctx->keypress_event_id = g_timeout_add (ctx->keypress_interval_ms,
		                                        keypress_event,
		                                        ctx);
	}
	g_rw_lock_writer_unlock(&ctx->lock);
	return 0;
}

static int remove_key_events(t_clicker *ctx) {
	g_rw_lock_writer_lock(&ctx->lock);
	if (ctx->keypress_event_id > 0) {
		g_print ("Pausing key presses...\n");
		g_source_remove(ctx->keypress_event_id);
		ctx->keypress_event_id = 0;
	}
	g_rw_lock_writer_unlock(&ctx->lock);
	return 0;
}

static int init_display(t_clicker *ctx) {
	g_rw_lock_writer_lock(&ctx->lock);
	ctx->display = XOpenDisplay(ctx->displayname);
	g_rw_lock_writer_unlock(&ctx->lock);
}

static int close_display(t_clicker *ctx) {
	g_rw_lock_writer_lock(&ctx->lock);
	XCloseDisplay(ctx->display);
	g_rw_lock_writer_unlock(&ctx->lock);
}

static void print_config(t_clicker *ctx) {
	g_print ("clicker configuration:\n");

	g_rw_lock_reader_lock(&ctx->lock);
	if (ctx->displayname == NULL) {
		g_print ("X11 display    : default\n");
	} else {
		g_print ("X11 display    : %s\n", ctx->displayname);
	}

	g_print ("initial delay  : %d (msecs)\n", ctx->initial_delay_ms);

	if (ctx->mousebutton == 0 || ctx->mousebutton_interval_ms == 0) {
		g_print ("mouse clicks   : disabled\n");
	} else {
		g_print ("mouse clicks   : button %d every %d msecs\n", ctx->mousebutton, ctx->mousebutton_interval_ms);
	}

	if (ctx->keypress == NULL || ctx->keypress_interval_ms == 0) {
		g_print ("keypresses     : disabled\n");
	} else {
		g_print ("keypresses     : key %s every %d msecs\n", ctx->keypress, ctx->keypress_interval_ms);
	}
	g_rw_lock_reader_unlock(&ctx->lock);
}

int start_events(t_clicker *ctx) {
	print_config(ctx);

	init_display(ctx);

	g_rw_lock_reader_lock(&ctx->lock);
	g_usleep(ctx->initial_delay_ms * 1000);
	g_rw_lock_reader_unlock(&ctx->lock);

	/* Configure main event loop */
	g_rw_lock_writer_lock(&ctx->lock);
	ctx->sighup_event_id = g_unix_signal_add(SIGHUP, sigquit_handler, ctx);
	ctx->sigint_event_id = g_unix_signal_add(SIGINT, sigquit_handler, ctx);
	ctx->sigterm_event_id = g_unix_signal_add(SIGTERM, sigquit_handler, ctx);
	/* g_timeout_add_seconds allows the process to go into deeper sleep states */
	ctx->pause_event_id = g_timeout_add_seconds (1, pause_event, ctx);
	/* g_timeout_add should be used for much shorter delays between polling */
	/* ctx->pause_event_id = g_timeout_add (50, pause_event, ctx); */
	ctx->main = g_main_loop_new(NULL, FALSE);
	g_rw_lock_writer_unlock(&ctx->lock);

	/* Start processing events */
	g_main_loop_run(ctx->main);

	g_print ("Cleaning up...\n");
	close_display(ctx);
}

