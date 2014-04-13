#include <stdio.h>
#include <sys/time.h>

#include "Source/WLToolKit.hpp"
#include "HomeScreen.hpp"

#define NUM_ICONS	4

using namespace WLToolKit;

class Background;
class Icon;

class MyWindow : public WindowEGL {
public:
	MyWindow(Display *display, int width, int height);
	virtual ~MyWindow();

	virtual void Render();

	virtual void OnClick(uint32_t button, int x, int y);
	virtual void OnTouchDown(int x, int y);

protected:
	Display *m_display;

	Background *m_bg;
	Icon *m_icons[NUM_ICONS];
};

/* ------------------------------------
	Background
-------------------------------------*/

class Background {
public:
	Background(MyWindow *window)
	: m_window(window) {
		m_texture = new Texture("bg.png");
	}

	virtual ~Background() {
		delete m_texture;
	}

	virtual void Draw() {
		m_texture->Draw(m_window, 0, 0);
	}

protected:
	MyWindow *m_window;
	Texture *m_texture;
};

/* ------------------------------------
	Animation
-------------------------------------*/

class Animation {
public:
	Animation(int x, int y, int width, int height, float initScale, float lastScale)
	: m_initX(x), m_initY(y),
	  m_initWidth(width), m_initHeight(height),
	  m_initScale(initScale), m_lastScale(lastScale) {
	}

	virtual ~Animation() {
	}

	virtual void Start(int duration) {
		m_duration = duration;
		m_time = GetTime();
	}

	virtual void Update() {
		unsigned int new_time = GetTime();

		float ratio = (float)m_duration / (new_time - m_time);

		m_scale = m_initScale + (m_lastScale - m_initScale) * ratio;
	}

	float GetScale() {
		return m_scale;
	}

protected:
	unsigned int GetTime() {
		struct timeval tv;
		gettimeofday(&tv, NULL);

		return (tv.tv_sec / 1000) + (tv.tv_usec * 1000);
	}

protected:
	int m_initX, m_initY;
	int m_initWidth, m_initHeight;
	float m_initScale, m_lastScale;

	int m_duration;
	unsigned int m_time;

	int m_x, m_y;
	int m_width, m_height;
	float m_scale;
};

/* ------------------------------------
	Icon
-------------------------------------*/

class Icon {
public:
	Icon(MyWindow *window, const char *iconPath, int x, int y)
	: m_window(window), m_x(x), m_y(y), m_bSelected(false) {
		m_texture = new Texture(iconPath);
		m_width = m_texture->GetWidth();
		m_height = m_texture->GetHeight();
	}

	virtual ~Icon() {
		delete m_texture;
	}

	virtual void Draw() {
		if (m_bSelected) {
			m_texture->Draw(m_window, m_x, m_y, 1.3);
		}
		else
			m_texture->Draw(m_window, m_x, m_y);
	}

	virtual void OnClick(int x, int y) {
		if (((m_x <= x) && (x <= (m_x + m_width))) && 
			((m_y <= y) && (y <= (m_y + m_height)))) {
#if 1
			fprintf(stderr, "Icon: Clicked\n");
#endif

			m_bSelected = true;
		} else {
			m_bSelected = false;
		}
	}

protected:
	MyWindow *m_window;
	int m_x, m_y;
	int m_width, m_height;
	bool m_bSelected;

	Texture *m_texture;
};

/* ------------------------------------
	HomeScreen
-------------------------------------*/

struct HomeScreenImpl {
	Display *display;
	MyWindow *window;
};

HomeScreen::HomeScreen(struct display* display)
{
	m_pImpl = new HomeScreenImpl;

	m_pImpl->display = new Display(display);
	m_pImpl->window = new MyWindow(m_pImpl->display, WINDOW_WIDTH, WINDOW_HEIGHT);
}

HomeScreen::~HomeScreen()
{
	delete m_pImpl->window;
	delete m_pImpl->display;
}

/* ------------------------------------
	MyWindow
-------------------------------------*/

MyWindow::MyWindow(Display* display, int width, int height)
: WindowEGL(display, width, height)
{
	m_bg = new Background(this);

	m_icons[0] = new Icon(this, "icon.png", 60, 180);
	m_icons[1] = new Icon(this, "icon.png", 240, 180);
	m_icons[2] = new Icon(this, "icon.png", 420, 180);
	m_icons[3] = new Icon(this, "icon.png", 610, 180);
}

MyWindow::~MyWindow()
{
	for (int i = 0; i < NUM_ICONS; i++) {
		delete m_icons[i];
	}
	delete m_bg;
}

void
MyWindow::Render()
{
	m_bg->Draw();

	for (int i = 0; i < NUM_ICONS; i++)
		m_icons[i]->Draw();
}

void
MyWindow::OnClick(uint32_t button, int x, int y)
{
#if 1
	fprintf(stderr, "HomeScreen: OnClick: (%f, %f)\n", x, y);
#endif

	for (int i = 0; i < NUM_ICONS; i++)
		m_icons[i]->OnClick(x, y);
}

void
MyWindow::OnTouchDown(int x, int y)
{
#if 1
	fprintf(stderr, "HomeScreen: OnTouchDown: (%f, %f)\n", x, y);
#endif

	for (int i = 0; i < NUM_ICONS; i++)
		m_icons[i]->OnClick(x, y);
}

