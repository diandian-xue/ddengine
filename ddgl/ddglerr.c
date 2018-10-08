#define DDGL_CORE


#include "ddglerr.h"

static const struct{
    int e;
    const char * str;
}_err_strs[] = {
    { DDGL_OK, "ok" },

    { DDGL_ERR_END, "Unknow error" },

    { -1, NULL },
};

DDGLAPI const char *
ddgl_err(int err){
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
