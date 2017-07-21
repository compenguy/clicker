#include "clicker.h"

static t_clicker ctx = {
	.lock = {},
	.displayname = NULL,
	.initial_delay_ms = 250,
	.mousebutton = 1,
	.mousebutton_interval_ms = 50,
	.mousebutton_count = 0,
	.keypress = NULL,
	.keypress_interval_ms = 1000,
	.keypress_count = 0,
	.display = NULL,
	.main = NULL,
	.mousebutton_event_id = 0,
	.keypress_event_id = 0,
	.pause_event_id = 0,
	.sigint_event_id = 0,
	.sighup_event_id = 0,
	.sigterm_event_id = 0
};

static GOptionEntry entries[] = 
{
	{ "x11-display", 'x', 0, G_OPTION_ARG_STRING, &ctx.displayname, "The X11 display to send the input to. Default: DISPLAY env var", "NAME" },
	{ "delay", 'd', 0, G_OPTION_ARG_INT, &ctx.initial_delay_ms, "Delay in msecs before sending any input events. Default: 250", "N" },
	{ "mouseclick", 'm', 0, G_OPTION_ARG_INT, &ctx.mousebutton, "Click mouse button N at regular intervals. Default: 1", "N" },
	{ "mouseclick-interval", 'c', 0, G_OPTION_ARG_INT, &ctx.mousebutton_interval_ms, "Delay in msecs between mouse button clicks. Default: 50, 0: disable", "N" },
	{ "keypress", 'k', 0, G_OPTION_ARG_STRING, &ctx.keypress, "Press key with the X11 name of KEY at regular intervals. Default: none", "KEY" },
	{ "keypress-interval", 'p', 0, G_OPTION_ARG_INT, &ctx.keypress_interval_ms, "Delay in msecs between key presses. Default: 1000, 0: disable", "N" },
	{ NULL }
};

int main(int argc, char *argv[]) {
	GError *error = NULL;
	GOptionContext *context;

	context = g_option_context_new ("- sends input signals to currently active window in an X session");
	g_option_context_add_main_entries (context, entries, NULL);
	if (!g_option_context_parse (context, &argc, &argv, &error))
	{
		g_print ("option parsing failed: %s\n", error->message);
		return 1;
	}
	else
	{
		return start_events(&ctx);
	}

	g_rw_lock_writer_lock(&ctx.lock);
	g_free(ctx.displayname);
	g_free(ctx.keypress);
	g_rw_lock_writer_unlock(&ctx.lock);

	return 0;
}
