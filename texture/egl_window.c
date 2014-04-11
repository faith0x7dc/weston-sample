/*
 * Copyright © 2011 Benjamin Franzke
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

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

#include "egl_window.h"

struct egl_window_priv {
	struct egl_window *egl_window;

	struct display *display;
	int width, height;

	struct wl_egl_window *native;
	EGLSurface egl_surface;
	struct wl_callback *callback;

	struct {
		EGLDisplay dpy;
		EGLContext ctx;
		EGLConfig conf;
	} egl;

#if 0
	struct {
		GLuint rotation_uniform;
		GLuint texture_uniform;
		GLuint pos;
		GLuint texcoord;
	} gl;
#endif

	egl_window_redraw_handler redraw_handler;
	void *redraw_handler_arg;
};

#if 0
static const char *vert_shader_text =
	"uniform mat4 rotation;\n"
	"attribute vec4 pos;\n"
	"attribute vec2 texcoord;\n"
	"varying vec2 v_texcoord;\n"
	"void main() {\n"
	"  gl_Position = rotation * pos;\n"
	"  v_texcoord = texcoord;\n"
	"}\n";

static const char *frag_shader_text =
	"precision mediump float;\n"
	"varying vec2 v_texcoord;\n"
	"uniform sampler2D texture;\n"
	"void main() {\n"
	"  gl_FragColor = texture2D(texture, v_texcoord);\n"
	"}\n";
#endif

static void init_egl(struct egl_window_priv *priv);
static void fini_egl(struct egl_window_priv *priv);

static void create_surface(struct egl_window_priv *priv);
static void destroy_surface(struct egl_window_priv *priv);

static void init_gl(struct egl_window_priv *priv);

static void redraw(void *data, struct wl_callback *callback, uint32_t time);
static void configure_callback(void *data, struct wl_callback *callback, uint32_t  time);

static GLuint create_shader(const char *source, GLenum shader_type);

static const struct wl_callback_listener frame_listener = {
	redraw
};

static struct wl_callback_listener configure_callback_listener = {
	configure_callback,
};

struct egl_window *
egl_window_create(struct display *display, int width, int height)
{
	struct egl_window *self;
	struct egl_window_priv *priv;

	self = (struct egl_window *)malloc(sizeof *self);
	memset(self, 0, sizeof *self);

	priv = (struct egl_window_priv *)malloc(sizeof *priv);
	memset(priv, 0, sizeof *priv);

	self->priv = priv;
	priv->egl_window = self;
	priv->display = display;
	priv->width = width;
	priv->height = height;

	self->window = window_create(priv->display);
	window_set_user_data(self->window, priv);

	self->widget = window_add_widget(self->window, priv);

	init_egl(priv);
	create_surface(priv);
	init_gl(priv);

	return self;
}

void
egl_window_destroy(struct egl_window *self)
{
	if (self) {
		if (self->priv) {
			struct egl_window_priv *priv = (struct egl_window_priv *)self->priv;

			destroy_surface(priv);
			fini_egl(priv);

			free(priv);
		}

		widget_destroy(self->widget);
		window_destroy(self->window);

		free(self);
	}
}

void
egl_window_set_redraw_handler(struct egl_window *self, egl_window_redraw_handler handler, void *arg)
{
	struct egl_window_priv *priv;

	if (!self)
		return;

	priv = (struct egl_window_priv *)self->priv;
	priv->redraw_handler = handler;
	priv->redraw_handler_arg = arg;
}

GLuint
egl_window_set_shader(struct egl_window *self, const char *vertex, const char *fragment)
{
	GLuint frag, vert;
	GLuint program;
	GLint status;

	frag = create_shader(fragment, GL_FRAGMENT_SHADER);
	vert = create_shader(vertex, GL_VERTEX_SHADER);

	program = glCreateProgram();
	glAttachShader(program, frag);
	glAttachShader(program, vert);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status) {
		char log[1000];
		GLsizei len;
		glGetProgramInfoLog(program, 1000, &len, log);
		fprintf(stderr, "Error: linking:\n%*s\n", len, log);
		exit(1);
	}

	glUseProgram(program);

	return program;
}


static void
init_egl(struct egl_window_priv *priv)
{
	static const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_ALPHA_SIZE, 1,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};

	EGLint major, minor, n;
	EGLBoolean ret;

	priv->egl.dpy = eglGetDisplay(display_get_display(priv->display));
	assert(priv->egl.dpy);

	ret = eglInitialize(priv->egl.dpy, &major, &minor);
	assert(ret == EGL_TRUE);
	ret = eglBindAPI(EGL_OPENGL_ES_API);
	assert(ret == EGL_TRUE);

	ret = eglChooseConfig(priv->egl.dpy, config_attribs, &priv->egl.conf, 1, &n);
	assert(ret && n == 1);

	priv->egl.ctx = eglCreateContext(priv->egl.dpy, priv->egl.conf, EGL_NO_CONTEXT, context_attribs);
	assert(priv->egl.ctx);
}

static void
fini_egl(struct egl_window_priv *priv)
{
	eglTerminate(priv->egl.dpy);
	eglReleaseThread();
}

static void
init_gl(struct egl_window_priv *priv)
{
#if 0
	GLuint frag, vert;
	GLuint program;
	GLint status;

	frag = create_shader(frag_shader_text, GL_FRAGMENT_SHADER);
	vert = create_shader(vert_shader_text, GL_VERTEX_SHADER);

	program = glCreateProgram();
	glAttachShader(program, frag);
	glAttachShader(program, vert);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status) {
		char log[1000];
		GLsizei len;
		glGetProgramInfoLog(program, 1000, &len, log);
		fprintf(stderr, "Error: linking:\n%*s\n", len, log);
		exit(1);
	}

	glUseProgram(program);
#endif
#if 0
	priv->gl.pos = 0;
	priv->gl.texcoord = 1;

	glBindAttribLocation(program, priv->gl.pos, "pos");
	glBindAttribLocation(program, priv->gl.texcoord, "texcoord");
	glLinkProgram(program);

	priv->gl.rotation_uniform = glGetUniformLocation(program, "rotation");
	priv->gl.texture_uniform  = glGetUniformLocation(program, "texture");
#endif
}

static void
create_surface(struct egl_window_priv *priv)
{
	struct wl_callback *callback;
	EGLBoolean ret;

	priv->native = wl_egl_window_create(window_get_wl_surface(priv->egl_window->window), priv->width, priv->height);
	priv->egl_surface = eglCreateWindowSurface(priv->egl.dpy, priv->egl.conf, priv->native, NULL);

	ret = eglMakeCurrent(priv->egl.dpy, priv->egl_surface, priv->egl_surface, priv->egl.ctx);
	assert(ret == EGL_TRUE);

	callback = wl_display_sync(display_get_display(priv->display));
	wl_callback_add_listener(callback, &configure_callback_listener, priv);
}

static void
destroy_surface(struct egl_window_priv *priv)
{
	/* Required, otherwise segfault in egl_dri2.c: dri2_make_current()
	 * on eglReleaseThread(). */
	eglMakeCurrent(priv->egl.dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	eglDestroySurface(priv->egl.dpy, priv->egl_surface);
	wl_egl_window_destroy(priv->native);

	if (priv->callback)
		wl_callback_destroy(priv->callback);
}

static const struct wl_callback_listener frame_listener;

#if 0

struct surface {
	cairo_surface_t *cairo_surface;
	unsigned char *data;
	int width, height, stride, format;
};

struct surface *load_surface(char *path);
struct surface *load_surface(char *path)
{
	struct surface *s;
	int x, y;
	unsigned char *ptr;

	s = (struct surface *)malloc(sizeof *s);
	memset(s, 0, sizeof *s);

	s->cairo_surface = cairo_image_surface_create_from_png("test.png");
	s->width = cairo_image_surface_get_width(s->cairo_surface);
	s->height = cairo_image_surface_get_height(s->cairo_surface);
	s->stride = cairo_image_surface_get_stride(s->cairo_surface);
	s->data = (unsigned char *)malloc(s->stride * s->height);

	ptr = cairo_image_surface_get_data(s->cairo_surface);

	/*
	 * cairoで取得したデータの並びはBGRAになっているが、
	 * OpenGL ES 2.0がRGBAしかサポートしていないので変換する
	 */
	/* cairoでsurfaceのデータを取得すると、bottom-upになるので、テクスチャ座標を上下逆にする */
	for (y = 0; y < s->height; y++) {
		for (x = 0; x < s->width; x++) {
			int idx_d = y * s->width + x;
			int idx_s = (s->height - y - 1) * s->width + x;

			s->data[idx_d * 4 + 0] = ptr[idx_s * 4 + 2];
			s->data[idx_d * 4 + 1] = ptr[idx_s * 4 + 1];
			s->data[idx_d * 4 + 2] = ptr[idx_s * 4 + 0];
			s->data[idx_d * 4 + 3] = ptr[idx_s * 4 + 3];
		}
	}

	return s;
}

void release_surface(struct surface *s);
void release_surface(struct surface *s)
{
	cairo_surface_destroy(s->cairo_surface);

	free(s);
}
#endif

#if 0
static void render(struct egl_window_priv *priv);
static void
render(struct egl_window_priv *priv)
{
	static const GLfloat verts[] = {
		-1.0f, -1.0f,		/* left, top */
		-1.0f,  1.0f,		/* left, bottom */
		 1.0f, -1.0f,		/* right, top */
		 1.0f,  1.0f,		/* right, bottom */
	};

	static const GLfloat texcoords[] = {
		0.0f, 0.0f,			/* left, top */
		0.0f, 1.0f,			/* left, bottom */
		1.0f, 0.0f,			/* right top */
		1.0f, 1.0f,			/* right bottom */
	};
	GLfloat rotation[] = {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	GLuint texture;
	struct surface *surface;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	surface = load_surface("test.png");
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->width, surface->height,
		0, GL_RGBA, GL_UNSIGNED_BYTE, surface->data);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glUniformMatrix4fv(priv->gl.rotation_uniform, 1, GL_FALSE, (GLfloat *) rotation);

	glVertexAttribPointer(priv->gl.pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
	glVertexAttribPointer(priv->gl.texcoord, 2, GL_FLOAT, GL_FALSE, 0, texcoords);

	glEnableVertexAttribArray(priv->gl.pos);
	glEnableVertexAttribArray(priv->gl.texcoord);
	glEnableVertexAttribArray(GL_TEXTURE_2D);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(priv->gl.pos);
	glDisableVertexAttribArray(priv->gl.texcoord);
}
#endif

static void
redraw(void *data, struct wl_callback *callback, uint32_t time)
{
	struct egl_window_priv *priv = (struct egl_window_priv *)data;

	assert(priv->callback == callback);
	priv->callback = NULL;

	if (callback)
		wl_callback_destroy(callback);

	glViewport(0, 0, priv->width, priv->height);

	glClearColor(0.0, 0.0, 0.0, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

#if 1
	if (priv->redraw_handler)
		priv->redraw_handler(priv->redraw_handler_arg);
#else
	render(priv);
#endif

	priv->callback = wl_surface_frame(window_get_wl_surface(priv->egl_window->window));
	wl_callback_add_listener(priv->callback, &frame_listener, priv);

	eglSwapBuffers(priv->egl.dpy, priv->egl_surface);
}

static void
configure_callback(void *data, struct wl_callback *callback, uint32_t  time)
{
	struct egl_window_priv *priv = (struct egl_window_priv *)data;

	wl_callback_destroy(callback);

	if (priv->callback == NULL)
		redraw(data, NULL, time);
}

static GLuint
create_shader(const char *source, GLenum shader_type)
{
	GLuint shader;
	GLint status;

	shader = glCreateShader(shader_type);
	assert(shader != 0);

	glShaderSource(shader, 1, (const char **) &source, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		char log[1000];
		GLsizei len;
		glGetShaderInfoLog(shader, 1000, &len, log);
		fprintf(stderr, "Error: compiling %s: %*s\n",
			shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment",
			len, log);
		exit(1);
	}

	return shader;
}

