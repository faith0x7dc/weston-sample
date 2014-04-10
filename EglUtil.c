#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <wayland-client.h>
#include <wayland-egl.h>

#include "EglUtil.h"

static const char *vertShaderText =
	"uniform mat4 rotation;\n"
	"attribute vec4 pos;\n"
	"attribute vec4 color;\n"
	"varying vec4 v_color;\n"
	"void main() {\n"
	"  gl_Position = rotation * pos;\n"
	"  v_color = color;\n"
	"}\n";

static const char *fragShaderText =
	"precision mediump float;\n"
	"varying vec4 v_color;\n"
	"void main() {\n"
	"  gl_FragColor = v_color;\n"
	"}\n";

void
InitEgl(struct wl_display *display, struct EglInfo *eglInfo)
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

	eglInfo->eglDpy = eglGetDisplay(display);
	assert(eglInfo->eglDpy);

	ret = eglInitialize(eglInfo->eglDpy, &major, &minor);
	assert(ret == EGL_TRUE);
	ret = eglBindAPI(EGL_OPENGL_ES_API);
	assert(ret == EGL_TRUE);

	ret = eglChooseConfig(eglInfo->eglDpy, config_attribs, &(eglInfo->eglCfg), 1, &n);
	assert(ret && n == 1);

	eglInfo->eglCtx = eglCreateContext(eglInfo->eglDpy, eglInfo->eglCfg, EGL_NO_CONTEXT, context_attribs);
	assert(eglInfo->eglCtx);
}

void
DeinitEgl(struct EglInfo *eglInfo)
{
	eglTerminate(eglInfo->eglDpy);
	eglReleaseThread();
}

void
CreateSurface(struct EglInfo *eglInfo, struct wl_surface *surface, int width, int height)
{
	EGLBoolean ret;

	eglInfo->eglWindow = wl_egl_window_create(surface, width, height);
	eglInfo->eglSurface = eglCreateWindowSurface(eglInfo->eglDpy, eglInfo->eglCfg, eglInfo->eglWindow, NULL);

	ret = eglMakeCurrent(eglInfo->eglDpy, eglInfo->eglSurface, eglInfo->eglSurface, eglInfo->eglCtx);
	assert(ret == EGL_TRUE);
}

void
DestroySurface(struct EglInfo *eglInfo)
{
	eglMakeCurrent(eglInfo->eglDpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	eglDestroySurface(eglInfo->eglDpy, eglInfo->eglSurface);
	wl_egl_window_destroy(eglInfo->eglWindow);
}

static GLuint
CreateShader(const char *source, GLenum type)
{
	GLuint shader;
	GLint status;

	shader = glCreateShader(type);
	assert(shader != 0);

	glShaderSource(shader, 1, (const char **) &source, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		char log[1000];
		GLsizei len;
		glGetShaderInfoLog(shader, 1000, &len, log);
		fprintf(stderr, "Error: compiling %s: %*s\n", type == GL_VERTEX_SHADER ? "vertex" : "fragment", len, log);
		exit(1);
	}

	return shader;
}

void
InitGl(struct EglInfo *eglInfo)
{
	GLuint frag, vert;
	GLuint program;
	GLint status;

	frag = CreateShader(fragShaderText, GL_FRAGMENT_SHADER);
	vert = CreateShader(vertShaderText, GL_VERTEX_SHADER);

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
	
	eglInfo->pos = 0;
	eglInfo->col = 1;

	glBindAttribLocation(program, eglInfo->pos, "pos");
	glBindAttribLocation(program, eglInfo->col, "color");
	glLinkProgram(program);

	eglInfo->rotUniform = glGetUniformLocation(program, "rotation");
}

void Draw(struct EglInfo *eglInfo)
{
	static const GLfloat verts[3][2] = {
		{ -0.5, -0.5 },
		{  0.5, -0.5 },
		{  0,    0.5 }
	};
	static const GLfloat colors[3][3] = {
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ 0, 0, 1 }
	};
	GLfloat angle;
	GLfloat rotation[4][4] = {
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 }
	};

	glUniformMatrix4fv(eglInfo->rotUniform, 1, GL_FALSE, (GLfloat *) rotation);

	glClearColor(0.0, 0.0, 0.0, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glVertexAttribPointer(eglInfo->pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
	glVertexAttribPointer(eglInfo->col, 3, GL_FLOAT, GL_FALSE, 0, colors);
	glEnableVertexAttribArray(eglInfo->pos);
	glEnableVertexAttribArray(eglInfo->col);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableVertexAttribArray(eglInfo->pos);
	glDisableVertexAttribArray(eglInfo->col);
}

