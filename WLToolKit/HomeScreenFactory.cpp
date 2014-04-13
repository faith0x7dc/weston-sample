#include "HomeScreenFactory.h"
#include "HomeScreen.hpp"

HomeScreenHandle
HomeScreenCreate(struct display *display)
{
	return (HomeScreenHandle)(new HomeScreen(display));
}

void
HomeScreenDestroy(HomeScreenHandle handle)
{
	HomeScreen *hs = (HomeScreen*)handle;

	delete hs;
}

