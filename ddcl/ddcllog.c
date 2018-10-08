#define DDCL_CORE


#include "ddcllog.h"
#include "ddcltimer.h"
#include "ddclmalloc.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LOG_MESSAGE_SIZE 256

static ddcl_Handle _log_svr;

static void
default_log_service_cb(ddcl_Msg * msg){
    //printf("[:%08x] ", msg->from);
    printf("%s\n", (const char *)msg->data);
    fflush(stdout);
}

DDCLAPI int
ddcl_init_log_module(ddcl * conf){
    _log_svr = ddcl_new_service(default_log_service_cb, NULL);
    return 0;
}

DDCLAPI void
ddcl_set_log_service(ddcl_Handle h){

}

DDCLAPI void
ddcl_log(ddcl_Handle from, const char * msg, ...){
    char tmp[LOG_MESSAGE_SIZE];
    char *data = NULL;

    va_list ap;
    va_start(ap,msg);
    int len = vsnprintf(tmp, LOG_MESSAGE_SIZE, msg, ap);
    va_end(ap);
    if (len >=0 && len < LOG_MESSAGE_SIZE) {
        len = (int)strlen(tmp);
        data = ddcl_malloc(len + 1);
        memcpy(data, tmp, len + 1);
    } else {
        int max_size = LOG_MESSAGE_SIZE;
        for (;;) {
            max_size *= 2;
            data = ddcl_malloc(max_size);
            va_start(ap, msg);
            len = vsnprintf(data, max_size, msg, ap);
            va_end(ap);
            if (len < max_size) {
                break;
            }
            ddcl_free(data);
        }
    }
    if (len < 0) {
        ddcl_free(data);
        perror("vsnprintf error :");
        return;
    }

    ddcl_send(_log_svr, from,
        DDCL_PTYPE_SEND, DDCL_CMD_LOG, 0, data, len + 1);
}
