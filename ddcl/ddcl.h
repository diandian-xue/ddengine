#pragma once


#include "ddclconfig.h"

typedef struct tag_ddcl{
    dduint32 worker; //工作线程数量
    dduint32 socket; // socket 线程数量
    dduint32 timer_ms; // timer毫米间隔
}ddcl;

DDCLAPI int
ddcl_init(ddcl * conf);

DDCLAPI void
ddcl_default(ddcl * conf);
