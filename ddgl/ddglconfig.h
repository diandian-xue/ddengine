#pragma once

#include "ddplatform.h"

#ifdef DDSYS_WIN
    #ifdef DDGL_EXPORTS
        #ifdef DDGL_CORE
            #define DDGLAPI     __declspec(dllexport)
        #else
            #define DDGLAPI     __declspec(dllimport)
        #endif
    #else
        #define DDGLAPI     extern
    #endif
#else
    #ifdef DDCL_EXPORTS
        #define DDGLAPI extern
     #else
        #define DDGLAPI
    #endif
#endif
