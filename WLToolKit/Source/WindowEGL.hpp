#ifndef WL_TOOLKIT_WINDOW_EGL_HPP
#define WL_TOOLKIT_WINDOW_EGL_HPP

extern "C" {
#include <GLES2/gl2.h>
}

#include "Window.hpp"

namespace WLToolKit {

class Display;
class WindowEGLImpl;

class WindowEGL : public Window {
public:
	WindowEGL(Display* display, int width, int height);
	virtual ~WindowEGL();

	virtual void Render() {}

	GLuint GetVertexAttribute();
	GLuint GetTexCoordAttribute();
	GLuint GetRotationUniform();
	GLuint GetTextureUniform();

protected:
	Display* m_display;
	WindowEGLImpl* m_pImpl;
}; // End-of-class WindowEGL

} // End-of-namespace WLToolKit

#endif /* WL_TOOLKIT_WINDOW_EGL_HPP */
