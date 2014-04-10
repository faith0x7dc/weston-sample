#include <stdio.h>
#include <assert.h>

#include <wayland-client.h>
#include "window.h"

#include "CppSample/CppSample.h"

int main(int argc, char *argv[])
{
	struct display *display;
	CppSampleHandle handle;

	display = display_create(&argc, argv);
	assert(display);

	handle = CreateCppSample(display);

	display_run(display);

	DestroyCppSample(handle);

	return 0;
}

