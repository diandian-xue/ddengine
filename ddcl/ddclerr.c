#define DDCL_CORE

#include "ddclerr.h"


static const char * _ERRER_STRS[] = {
    "OK",
    "File path is error",
    "File is not exist",
    "File path must be directory",
    "File path can not be directory",

    "Unknow service handle",
    "Thread has been registered in service framework",

    "Unknow socket handle",


    "unknow error", // DDCL_ERR_END
};


DDCLAPI const char *
ddcl_err(int err){
    if (err < 0 || err > DDCL_ERR_END)
        return _ERRER_STRS[DDCL_ERR_END];
    else
        return _ERRER_STRS[err];
}
