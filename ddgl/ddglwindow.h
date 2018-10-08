#pragma once

#include "ddgl.h"
#include "ddclstorage.h"

#define WINDOW_EXTEND 256

typedef void(*ddgl_WindowFreeFn)(void * w);
typedef void(*ddgl_WindowSwapBufferFn)(void * w);
typedef void(*ddgl_WindowMakeCurrentFn)(void * w);
typedef void(*ddgl_WindowClearFn)(void * w);

typedef struct tag_ddgl_Window{
    ddcl_Handle h;
    unsigned x;
    unsigned y;
    unsigned width;
    unsigned height;
    char title[256];
    char extend[WINDOW_EXTEND];

    ddgl_WindowFreeFn free;
    ddgl_WindowSwapBufferFn swap_buffer;
    ddgl_WindowMakeCurrentFn make_current;
    ddgl_WindowClearFn clear;
}ddgl_Window;


#ifdef DDSYS_WIN
#define DDGL_USE_WIN32_WINDOW
#endif

#ifdef DDSYS_LINUX
#define DDGL_USE_X11_WINDOW
#endif

DDGLAPI int
ddgl_init_window_module (ddgl * conf);

DDGLAPI void
ddgl_exit_window_module ();

DDGLAPI ddgl_Window *
ddgl_new_window (const char * title, unsigned width, unsigned height);

DDGLAPI int
ddgl_poll_window_event();

