#include "Common.hpp"
#include "Display.hpp"
#include "Window.hpp"

namespace WLToolKit {

static void _ButtonHandler(struct widget *widget, struct input *input, uint32_t time, uint32_t button, enum wl_pointer_button_state state, void *data);
static void _TouchDownHandler(struct widget *widget, struct input *input, uint32_t serial, uint32_t time, int32_t id, float x, float y, void *data);

Window::Window(Display *display, int width, int height)
: m_display(display), m_width(width), m_height(height)
{
	assert(m_display);

	m_window = window_create(m_display->GetDisplay());
	window_set_user_data(m_window, this);

	m_widget = window_add_widget(m_window, this);
	widget_set_button_handler(m_widget, &_ButtonHandler);
	widget_set_touch_down_handler(m_widget, &_TouchDownHandler);
}

Window::~Window()
{
	window_destroy(m_window);
}

void
Window::Resize(int width, int height)
{
	m_width = width;
	m_height = height;

	window_schedule_resize(m_window, m_width, m_height);
}

struct wl_surface*
Window::GetWlSurface()
{
	return window_get_wl_surface(m_window);
}

static void
_ButtonHandler(struct widget *widget, struct input *input, uint32_t time, uint32_t button, enum wl_pointer_button_state state, void *data)
{
	if (data) {
		Window* self = (Window*)data;

		if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
			int x, y;
			input_get_position(input, &x, &y);

			self->OnClick(button, x, y);
		}
	}
}

static void
_TouchDownHandler(struct widget *widget, struct input *input, uint32_t serial, uint32_t time, int32_t id, float x, float y, void *data)
{
	if (data) {
		Window* self = (Window*)data;

		self->OnTouchDown((int)x, (int)y);
	}
}

} // End-of-namespace WLToolKit

