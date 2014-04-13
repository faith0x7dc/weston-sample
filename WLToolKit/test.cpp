#include <stdio.h>

#include "Source/WLToolKit.hpp"

extern "C" {
#include <cairo.h>
}

using namespace WLToolKit;

class MyWindow : public WindowEGL {
public:
	MyWindow(Display *display, int width, int height);
	virtual ~MyWindow();

	virtual void Render();

protected:
	Texture *m_texture;

	float m_scale;
};

int
main(int argc, char** argv)
{
	Display *display = new Display(&argc, argv);
	Window *window = new MyWindow(display, 400, 400);

	display->Run();

	delete window;
	delete display;

	return 0;
}

MyWindow::MyWindow(Display* display, int width, int height)
: WindowEGL(display, width, height)
{
	m_texture = new Texture("icon.png");

	m_scale = 1.0f;
}

MyWindow::~MyWindow()
{
	delete m_texture;
}

void
MyWindow::Render()
{
	int x = GetWidth() / 2 - m_texture->GetWidth() / 2;
	int y = GetHeight() / 2 - m_texture->GetHeight() / 2;

#if 1
	m_scale += 0.01;

	float scale = m_scale;

	if (scale >= 2.0f)
		scale = 2.0f;
	else if (scale <= 1.0f)
		scale = 1.0f;

	if (m_scale >= 3.0f)
		m_scale = 0.0f;

	m_texture->Draw(this, x, y, scale);

#else

	m_texture->Draw(this, x, y, 1.0f);
#endif
}

