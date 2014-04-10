
#include "CppSample.h"
#include "CCppSample.hpp"

CppSampleHandle
CreateCppSample(struct display* display)
{
	return (CppSampleHandle)(new CCppSample(display));
}

void
DestroyCppSample(CppSampleHandle handle)
{
	if (!handle)
		return;

	CCppSample* app = (CCppSample*)handle;

	delete app;
}

