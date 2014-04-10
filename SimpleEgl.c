#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <signal.h>

#include <linux/input.h>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <wayland-cursor.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "EglUtil.h"

#include "window.h"

struct EglInfo eglInfo;

static struct wl_callback *g_callback = NULL;
static int g_width = 250;
static int g_height = 250;

static void
redraw(void *data, struct wl_callback *callback, uint32_t time);

static void
configure_callback(void *data, struct wl_callback *callback, uint32_t  time)
{
	wl_callback_destroy(callback);

	if (g_callback == NULL)
		redraw(data, NULL, time);
}

static struct wl_callback_listener configure_callback_listener = {
	configure_callback,
};

static void
create_surface(struct window *window)
{
	struct wl_callback *callback;
	struct wl_surface *surface;

	surface = window_get_wl_surface(window);

	CreateSurface(&eglInfo, surface, g_width, g_height);

	callback = wl_display_sync(display_get_display(window_get_display(window)));
	wl_callback_add_listener(callback, &configure_callback_listener, window);
}

static const struct wl_callback_listener frame_listener;

static void
redraw(void *data, struct wl_callback *callback, uint32_t time)
{
	struct window *window = (struct window *)data;

	assert(g_callback == callback);
	g_callback = NULL;

	if (callback)
		wl_callback_destroy(callback);

	glViewport(0, 0, g_width, g_height);

	Draw(&eglInfo);

	g_callback = wl_surface_frame(window_get_wl_surface(window));
	wl_callback_add_listener(g_callback, &frame_listener, window);

	eglSwapBuffers(eglInfo.eglDpy, eglInfo.eglSurface);
}

static const struct wl_callback_listener frame_listener = {
	redraw
};

int
main(int argc, char **argv)
{
	struct display *display;
	struct window *window;
	struct widget *widget;

	display = display_create(&argc, argv);
	assert(display);

	window = window_create(display);
	window_set_user_data(window, window);

	widget = window_add_widget(window, window);

	InitEgl(display_get_display(display), &eglInfo);
	create_surface(window);
	InitGl(&eglInfo);

	window_schedule_resize(window, 250, 250);

	display_run(display);

	DestroySurface(&eglInfo);

	widget_destroy(widget);
	window_destroy(window);
	display_destroy(display);

	return 0;
}
