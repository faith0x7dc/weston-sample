#ifndef WL_TOOLKIT_WINDOW_HPP
#define WL_TOOLKIT_WINDOW_HPP

#include <stdint.h>

struct wl_surface;
struct window;
struct widget;

namespace WLToolKit {

class Display;

class Window {
public:
	Window(Display* display, int width, int height);
	virtual ~Window();

	void Resize(int width, int height);

	Display* GetDisplay() { return m_display; }
	struct window* GetWindow() { return m_window; }
	struct widget* GetWidget() { return m_widget; }
	struct wl_surface* GetWlSurface();

	int GetWidth() { return m_width; }
	int GetHeight() { return m_height; }

	virtual void OnClick(uint32_t button, int x, int y) {}
	virtual void OnTouchDown(int x, int y) {}

protected:
	Display* m_display;
	struct window* m_window;
	struct widget* m_widget;

	int m_width, m_height;
}; // End-of-class Window

} // End-of-namespace WLToolKit

#endif /* WL_TOOLKIT_WINDOW_HPP */
