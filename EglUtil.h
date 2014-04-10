#ifndef EGL_UTIL_H
#define EGL_UTIL_H

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

struct display;
struct wl_egl_window;

struct EglInfo{
	struct wl_egl_window *eglWindow;
	EGLSurface eglSurface;

	EGLDisplay eglDpy;
	EGLContext eglCtx;
	EGLConfig eglCfg;

	GLuint rotUniform;
	GLuint pos;
	GLuint col;
};

#if 0
extern void InitEgl(struct wl_display *display, EGLDisplay *eglDpy, EGLContext *eglCtx, EGLConfig *eglCfg);
extern void DeinitEgl(EGLDisplay *eglDpy);

extern void CreateSurface(EGLDisplay *eglDpy, EGLConfig *eglCfg, EGLContext *eglCtx, struct wl_surface **surface, struct wl_egl_window **eglWindow, EGLSurface *eglSurface, int width, int height);
#endif

extern void InitEgl(struct wl_display *display, struct EglInfo *eglInfo);
extern void DeinitEgl(struct EglInfo *eglInfo);

extern void CreateSurface(struct EglInfo *eglInfo, struct wl_surface *surface, int width, int height);
extern void DestroySurface(struct EglInfo *eglInfo);

extern void InitGl(struct EglInfo *eglInfo);

extern void Draw(struct EglInfo *eglInfo);

#endif /* EGL_UTIL_H */
