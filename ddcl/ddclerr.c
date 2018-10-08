#define DDCL_CORE

#include "ddclerr.h"

static const struct {
    int e;
    const char * str;
}_err_strs[] = {
    { DDCL_OK , "ok" },
    { DDCL_FILE_PATH_ERR, "File path is error" },
    { DDCL_FILE_NOT_EXIST, "File is not exist" },
    { DDCL_FILE_MUST_BE_DIR, "File path must be directory" },
    { DDCL_FILE_CANNOT_BE_DIR, "File path can not be directory" },
    { DDCL_SERVICE_UNKNOW_HANDLE, "Unknow service handle" },
    { DDCL_SERVICE_THREAD_IS_REGISTERED, "Thread has been registered in service framework" },
    { DDCLSOCKET_INVALID_HANDLE, "Unknow socket handle" },
    { DDCL_ERR_END, "Unknow error" },

    { -1, NULL},
};


DDCLAPI const char *
ddcl_err(int err){
    int i = 0;
    for(;;){
        int e = _err_strs[i].e;
        if(e != -1 && err == e){
            return _err_strs[i].str;
        }else{
            return _err_strs[i - 1].str;
        }
        i ++;
    }
}

