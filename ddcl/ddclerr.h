#pragma once


#include "ddclconfig.h"

enum ddcl_ERR{
    DDCL_OK = 0,
    DDCL_FILE_PATH_ERR,
    DDCL_FILE_NOT_EXIST,
    DDCL_FILE_MUST_BE_DIR,
    DDCL_FILE_CANNOT_BE_DIR,

    DDCL_SERVICE_UNKNOW_HANDLE,
    DDCL_SERVICE_THREAD_IS_REGISTERED,

    DDCLSOCKET_INVALID_HANDLE,

    DDCL_ERR_END,
};

DDCLAPI const char *
ddcl_err(int err);

