#ifndef WL_TOOLKIT_COMMON_HPP
#define WL_TOOLKIT_COMMON_HPP

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

/** wayland */
#include <wayland-client.h>
#include <wayland-egl.h>

/** OpenGL ES 2.0 */
#include <GLES2/gl2.h>

/** EGL */
#include <EGL/egl.h>
#include <EGL/eglext.h>

/** cairo */
#include <cairo.h>

/** wayland toytoolkit */
#include "window.h"
}

#endif /* WL_TOOLKIT_COMMON_HPP */
