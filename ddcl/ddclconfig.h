#pragma once

#include "ddplatform.h"

#ifdef DDSYS_WIN
    #ifndef WIN32_LEAN_AND_MEAN
       #define WIN32_LEAN_AND_MEAN
    #endif

    #ifdef DDCL_EXPORTS
        #ifdef DDCL_CORE
            #define DDCLAPI    __declspec(dllexport)
        #else
            #define DDCLAPI    __declspec(dllimport)
        #endif
    #else
        #define DDCLAPI    extern
    #endif
#else
    #ifdef DDCL_EXPORTS
        #define DDCLAPI extern
     #else
        #define DDCLAPI
    #endif
#endif
