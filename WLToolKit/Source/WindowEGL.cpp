#include "Common.hpp"
#include "Display.hpp"
#include "WindowEGL.hpp"

namespace WLToolKit {

class WindowEGLImpl {
public:
	WindowEGLImpl(WindowEGL* window);
	virtual ~WindowEGLImpl();

	void OnRedraw(struct wl_callback* callback, uint32_t time);

	static void _RedrawHandler(void* data, struct wl_callback* callback, uint32_t time);
	static void _ConfigureHandler(void* data, struct wl_callback* callback, uint32_t time);

protected:
	bool InitEGL();
	void DeinitEGL();

	bool InitGL();

	bool CreateSurface();
	void DestroySurface();

	GLuint CreateShader(const char* source, GLenum type);

public:
	struct wl_egl_window* m_native;
	EGLSurface m_eglSurface;
	struct wl_callback* m_callback;

	struct {
		EGLDisplay	dpy;
		EGLContext	ctx;
		EGLConfig	cfg;
	} m_egl;

	struct {
		GLuint attributePosition;
		GLuint attributeTexCoord;
		GLuint uniformRotation;
		GLuint uniformTexture;
	} m_gl;

protected:
	WindowEGL *m_window;
};

static const wl_callback_listener frameListener = {
	&WindowEGLImpl::_RedrawHandler
};
static const wl_callback_listener configureListener  = {
	&WindowEGLImpl::_ConfigureHandler
};

WindowEGL::WindowEGL(Display* display, int width, int height)
: Window(display, width, height)
{
	m_pImpl = new WindowEGLImpl(this);

	Resize(width, height);
}

WindowEGL::~WindowEGL()
{
	delete m_pImpl;
}


GLuint
WindowEGL::GetVertexAttribute()
{
	return m_pImpl->m_gl.attributePosition;
}

GLuint
WindowEGL::GetTexCoordAttribute()
{
	return m_pImpl->m_gl.attributeTexCoord;
}

GLuint
WindowEGL::GetRotationUniform()
{
	return m_pImpl->m_gl.uniformRotation;
}

GLuint
WindowEGL::GetTextureUniform()
{
	return m_pImpl->m_gl.uniformTexture;
}

WindowEGLImpl::WindowEGLImpl(WindowEGL* window)
: m_window(window), m_callback(NULL)
{
	assert(m_window);

	bool ret;

	ret = InitEGL();
	assert(ret);

	ret = CreateSurface();
	assert(ret);

	ret = InitGL();
	assert(ret);
}

WindowEGLImpl::~WindowEGLImpl()
{
	DestroySurface();
	DeinitEGL();
}

void
WindowEGLImpl::OnRedraw(struct wl_callback* callback, uint32_t time)
{
	assert(m_callback == callback);
	m_callback = NULL;

	if (callback)
		wl_callback_destroy(callback);

	glViewport(0, 0, m_window->GetWidth(), m_window->GetHeight());

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	m_window->Render();

	m_callback = wl_surface_frame(m_window->GetWlSurface());
	wl_callback_add_listener(m_callback, &frameListener, this);

	eglSwapBuffers(m_egl.dpy, m_eglSurface);
}

void
WindowEGLImpl::_RedrawHandler(void* data, struct wl_callback* callback, uint32_t time)
{
	assert(data);

	WindowEGLImpl *pImpl = (WindowEGLImpl*)data;

	pImpl->OnRedraw(callback, time);
}

void
WindowEGLImpl::_ConfigureHandler(void* data, struct wl_callback* callback, uint32_t time)
{
	assert(data);

	wl_callback_destroy(callback);

	WindowEGLImpl* pImpl = (WindowEGLImpl*)data;

	if (pImpl->m_callback == NULL)
		_RedrawHandler(data, NULL, time);
}

bool
WindowEGLImpl::InitEGL()
{
	static const EGLint ctx_attr[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	static const EGLint cfg_attr[] = {
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

	m_egl.dpy = eglGetDisplay(m_window->GetDisplay()->GetWlDisplay());
	if (!m_egl.dpy)
		return false;

	ret = eglInitialize(m_egl.dpy, &major, &minor);
	if (ret != EGL_TRUE)
		return false;	

	ret = eglBindAPI(EGL_OPENGL_ES_API);
	if (ret != EGL_TRUE)
		return false;

	ret = eglChooseConfig(m_egl.dpy, cfg_attr, &(m_egl.cfg), 1, &n);
	if (!ret)
		return false;
	if (n != 1)
		return false;

	m_egl.ctx = eglCreateContext(m_egl.dpy, m_egl.cfg, EGL_NO_CONTEXT, ctx_attr);
	if (!m_egl.ctx)
		return false;

	return true;
}

void
WindowEGLImpl::DeinitEGL()
{
	eglTerminate(m_egl.dpy);
	eglReleaseThread();
}

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

bool
WindowEGLImpl::InitGL()
{
	GLuint frag, vert;
	GLuint program;
	GLint status;

	frag = CreateShader(frag_shader_text, GL_FRAGMENT_SHADER);
	vert = CreateShader(vert_shader_text, GL_VERTEX_SHADER);

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
		return false;
	}

	glUseProgram(program);

	m_gl.attributePosition = 0;
	m_gl.attributeTexCoord = 1;

	glBindAttribLocation(program, m_gl.attributePosition, "pos");
	glBindAttribLocation(program, m_gl.attributeTexCoord, "texcoord");
	glLinkProgram(program);

	m_gl.uniformRotation = glGetUniformLocation(program, "rotation");
	m_gl.uniformTexture  = glGetUniformLocation(program, "texture");

	return true;
}

bool
WindowEGLImpl::CreateSurface()
{
	struct wl_callback *callback;
	EGLBoolean ret;

	m_native = wl_egl_window_create(m_window->GetWlSurface(), m_window->GetWidth(), m_window->GetHeight());
	m_eglSurface = eglCreateWindowSurface(m_egl.dpy, m_egl.cfg, m_native, NULL);

	ret = eglMakeCurrent(m_egl.dpy, m_eglSurface, m_eglSurface, m_egl.ctx);
	if (ret != EGL_TRUE)
		return false;

	callback = wl_display_sync(m_window->GetDisplay()->GetWlDisplay());
	wl_callback_add_listener(callback, &configureListener, this);

	return true;
}

void
WindowEGLImpl::DestroySurface()
{
	eglMakeCurrent(m_egl.dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	eglDestroySurface(m_egl.dpy, m_eglSurface);
	wl_egl_window_destroy(m_native);

	if (m_callback)
		wl_callback_destroy(m_callback);
}

GLuint
WindowEGLImpl::CreateShader(const char *source, GLenum type)
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
		fprintf(stderr, "Error: compiling %s: %*s\n", ((type == GL_VERTEX_SHADER) ? "vertex" : "fragment"), len, log);
		exit(1);
	}

	return shader;
}

} // End-of-namespace WLToolKit

