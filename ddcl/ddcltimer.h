#pragma once

#include "ddclconfig.h"
#include "ddcl.h"
#include "ddclthread.h"
#include "ddclservice.h"

DDCLAPI int
ddcl_timer_module_init(ddcl * conf);

DDCLAPI dduint64
ddcl_now ();

DDCLAPI dduint64
ddcl_systime ();

DDCLAPI void
ddcl_add_timeout (ddcl_Handle h, ddcl_Session session, dduint32 ms);
