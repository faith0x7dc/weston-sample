#ifndef CCPP_SAMPLE_HPP
#define CCPP_SAMPLE_HPP

#include <stdint.h>

extern "C" {
#include <wayland-client.h>
#include "window.h"
}

class CCppSample {
public:
	CCppSample(struct display* display);
	virtual ~CCppSample();

	struct display* GetDisplay();
	struct window* GetWindow();
	struct widget* GetWidget();

	struct wl_display* GetWlDisplay();
	struct wl_surface* GetWlSurface();

	void OnRedraw();
	void OnResize(int width, int height);
	void OnMouseDown(struct input* input, uint32_t time, uint32_t button);
	void OnTouchDown(struct input* input, uint32_t serial, uint32_t time, int32_t id, float x, float y);

	static void _RedrawHandler(struct widget* widget, void* data);
	static void _ResizeHandler(struct widget* widget, int width, int height, void* data);
	static void _ButtonHandler(struct widget* widget, struct input* input, uint32_t time, uint32_t button, enum wl_pointer_button_state state, void* data);
	static void _TouchHandler(struct widget* widget, struct input* input, uint32_t serial, uint32_t time, int32_t id, float x, float y, void* data);

protected:
	struct display* m_display;
	struct window* m_window;
	struct widget* m_widget;
};

#endif /* CCPP_SAMPLE_HPP */
