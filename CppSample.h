#ifndef CPP_SAMPLE_H
#define CPP_SAMPLE_H

#define WINDOW_WIDTH	800
#define WINDOW_HEIGHT	480

struct display;

typedef void* CppSampleHandle;

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

CppSampleHandle CreateCppSample(struct display* display);
void DestroyCppSample(CppSampleHandle handle);

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* CCPP_SAMPLE_HPP */
