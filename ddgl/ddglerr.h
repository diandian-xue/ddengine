#pragma once

#include "ddgl.h"

enum ddgl_ERR{
    DDGL_OK = 0,

    DDGL_ERR_END,
};

DDGLAPI const char *
ddgl_err(int err);
