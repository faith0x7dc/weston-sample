#include "Common.hpp"
#include "Display.hpp"

namespace WLToolKit {

Display::Display(struct display* display)
: m_display(display), m_isOwner(false)
{
	assert(m_display);
}

Display::Display(int* argc, char** argv)
: m_isOwner(true)
{
	m_display = display_create(argc, argv);
	assert(m_display);
}

Display::~Display()
{
	if (m_isOwner)
		display_destroy(m_display);
}

void
Display::Run()
{
	if (m_isOwner)
		display_run(m_display);
}

struct wl_display*
Display::GetWlDisplay()
{
	return display_get_display(m_display);
}

} // End-of-namespace WLToolKit

