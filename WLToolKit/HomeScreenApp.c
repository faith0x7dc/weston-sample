#include <stdio.h>

#include "HomeScreenFactory.h"
#include "window.h"

int
main(int argc, char** argv)
{
	struct display *display;
	HomeScreenHandle hs_handle;

	display = display_create(&argc, argv);

	hs_handle = HomeScreenCreate(display);

	display_run(display);

	HomeScreenDestroy(hs_handle);

	display_destroy(display);

	return 0;
}

