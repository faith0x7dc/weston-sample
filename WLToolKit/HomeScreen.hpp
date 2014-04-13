#ifndef HOME_SCREEN_HPP
#define HOME_SCREEN_HPP

#define WINDOW_WIDTH	800
#define WINDOW_HEIGHT	480

struct display;
struct HomeScreenImpl;

class HomeScreen {
public:
	HomeScreen(struct display* display);
	virtual ~HomeScreen();

protected:
	struct HomeScreenImpl* m_pImpl;
};

#endif /* HOME_SCREEN_HPP */
