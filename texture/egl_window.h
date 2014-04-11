#ifndef EGL_WINDOW_H
#define EGL_WINDOW_H

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

#include "window.h"

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

typedef void (*egl_window_redraw_handler)(void *);

struct egl_window {
	struct window *window;	/* toytoolkit's window */
	struct widget *widget;	/* toytoolkit's widget */

	void *priv;				/* egl_window's private data */
};

extern struct egl_window *egl_window_create(struct display *display, int width, int height);
extern void egl_window_destroy(struct egl_window *self);

extern void egl_window_set_redraw_handler(struct egl_window *self, egl_window_redraw_handler handler, void *arg);
extern GLuint egl_window_set_shader(struct egl_window *self, const char *vertex, const char *fragment);

#endif /* EGL_WINDOW_H */
