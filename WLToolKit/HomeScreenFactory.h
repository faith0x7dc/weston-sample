#ifndef HOME_SCREEN_FACTORY_H
#define HOME_SCREEN_FACTORY_H

#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

struct display;

typedef void* HomeScreenHandle;

extern HomeScreenHandle HomeScreenCreate(struct display *display);
extern void HomeScreenDestroy(HomeScreenHandle handle);

#if defined(__cplusplus)
}
#endif /* defined(__cplusplus) */

#endif /* HOME_SCREEN_FACTORY_H */
