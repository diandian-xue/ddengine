#pragma once

#include "ddclconfig.h"
#include "ddcl.h"
#include "ddclservice.h"

DDCLAPI int
ddcl_init_log_module(ddcl * conf);

DDCLAPI void
ddcl_set_log_service(ddcl_Handle h);

DDCLAPI void
ddcl_log(ddcl_Handle from, const char * msg, ...);