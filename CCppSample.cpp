#include <stdio.h>

#include "CppSample.h"
#include "CCppSample.hpp"

CCppSample::CCppSample(struct display* display)
: m_display(display)
{
	m_window = window_create(display);
	window_set_user_data(m_window, this);

	m_widget = window_add_widget(m_window, this);
	widget_set_redraw_handler(m_widget, &_RedrawHandler);
	widget_set_resize_handler(m_widget, &_ResizeHandler);
	widget_set_button_handler(m_widget, &_ButtonHandler);
	widget_set_touch_down_handler(m_widget, &_TouchHandler);

	window_schedule_resize(m_window, WINDOW_WIDTH, WINDOW_HEIGHT);
}

CCppSample::~CCppSample()
{
	widget_destroy(m_widget);
	window_destroy(m_window);
}

struct display*
CCppSample::GetDisplay()
{
	return m_display;
}

struct window*
CCppSample::GetWindow()
{
	return m_window;
}

struct widget*
CCppSample::GetWidget()
{
	return m_widget;
}

struct wl_display*
CCppSample::GetWlDisplay()
{
	return display_get_display(GetDisplay());
}

struct wl_surface*
CCppSample::GetWlSurface()
{
	return widget_get_wl_surface(GetWidget());
}

void
CCppSample::OnRedraw()
{
	cairo_t *cr = widget_cairo_create(m_widget);

	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba(cr, 0.0, 0.0, 0.2, 1.0);
	cairo_paint(cr);

	cairo_destroy(cr);
}

void
CCppSample::OnResize(int width, int height)
{
	widget_set_size(m_widget, width, height);
}

void
CCppSample::OnMouseDown(struct input *input, uint32_t time, uint32_t button)
{
}

void
CCppSample::OnTouchDown(struct input *input, uint32_t serial, uint32_t time,
	int32_t id, float x, float y)
{
}

void
CCppSample::_RedrawHandler(struct widget *widget, void *data)
{
	if (!data)
		return;

	CCppSample* self = (CCppSample*)data;
	self->OnRedraw();
}

void
CCppSample::_ResizeHandler(struct widget *widget, int width, int height, void *data)
{
	if (!data)
		return;

	CCppSample* self = (CCppSample*)data;
	self->OnResize(width, height);
}

void
CCppSample::_ButtonHandler(struct widget *widget, struct input *input,
	uint32_t time, uint32_t button, enum wl_pointer_button_state state, void *data)
{
	if (!data)
		return;

	CCppSample* self = (CCppSample*)data;
	if (state == WL_POINTER_BUTTON_STATE_RELEASED)
		self->OnMouseDown(input, time, button);
}

void
CCppSample::_TouchHandler(struct widget *widget, struct input *input,
	uint32_t serial, uint32_t time, int32_t id, float x, float y, void *data)
{
	if (!data)
		return;

	CCppSample* self = (CCppSample*)data;
	self->OnTouchDown(input, serial, time, id, x, y);
}

