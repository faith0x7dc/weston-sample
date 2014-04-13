#ifndef WL_TOOLKIT_DISPLAY_HPP
#define WL_TOOLKIT_DISPLAY_HPP

struct display;
struct wl_display;

namespace WLToolKit {

class Display {
public:
	Display(struct display* display);
	Display(int* argc, char** argv);
	virtual ~Display();

	void Run();

	struct display* GetDisplay() { return m_display; }
	struct wl_display* GetWlDisplay();

protected:
	struct display *m_display;
	bool m_isOwner;
}; // End-of-class Display

} // End-of-namespace WLToolKit

#endif /* WL_TOOLKIT_DISPLAY_HPP */
