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
#include <assert.h>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <cairo.h>

#include "egl_window.h"

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

struct surface *load_surface(char *path);
void release_surface(struct surface *s);

struct surface {
	cairo_surface_t *cairo_surface;
	unsigned char *data;
	int width, height, stride, format;
};

struct surface *surface;
GLuint program;
GLuint pos, texcoord, rotation_uniform, texture_uniform;

static void redraw_handler(void *arg);

static void redraw_handler(void *arg)
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

	glUniformMatrix4fv(rotation_uniform, 1, GL_FALSE, (GLfloat *) rotation);

	glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
	glVertexAttribPointer(texcoord, 2, GL_FLOAT, GL_FALSE, 0, texcoords);

	glEnableVertexAttribArray(pos);
	glEnableVertexAttribArray(texcoord);
	glEnableVertexAttribArray(GL_TEXTURE_2D);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(pos);
	glDisableVertexAttribArray(texcoord);
}

int
main(int argc, char **argv)
{
	struct display *display;
	struct egl_window *egl_window;

	display = display_create(&argc, argv);
	assert(display);

	egl_window = egl_window_create(display, 800, 480);
	assert(egl_window);

	surface = load_surface("test.png");

	egl_window_set_redraw_handler(egl_window, &redraw_handler, NULL);
	program = egl_window_set_shader(egl_window, vert_shader_text, frag_shader_text);

	pos = 0;
	texcoord = 1;

	glBindAttribLocation(program, pos, "pos");
	glBindAttribLocation(program, texcoord, "texcoord");
	glLinkProgram(program);

	rotation_uniform = glGetUniformLocation(program, "rotation");
	texture_uniform  = glGetUniformLocation(program, "texture");

	window_schedule_resize(egl_window->window, 800, 480);

	display_run(display);

	release_surface(surface);

	egl_window_destroy(egl_window);

	display_destroy(display);

	return 0;
}

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

void release_surface(struct surface *s)
{
	cairo_surface_destroy(s->cairo_surface);

	free(s);
}

