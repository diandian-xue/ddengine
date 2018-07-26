#define DDCL_CORE

#include "ddcl.h"

#include "ddclfile.h"
#include "ddclmalloc.h"
#include "ddclservice.h"
#include "ddcltimer.h"
#include "ddclsocket.h"
#include "ddcllog.h"

#define CKRETURN(X) { int r = X(conf); if(r != 0) return r; }

DDCLAPI int
ddcl_init(ddcl * conf){
    CKRETURN(ddcl_malloc_module_init);
    CKRETURN(ddcl_file_module_init);
    CKRETURN(ddcl_init_service_module);
    CKRETURN(ddcl_init_log_module);
    CKRETURN(ddcl_timer_module_init);
    CKRETURN(ddcl_socket_module_init);
    return 0;
}

DDCLAPI void
ddcl_default(ddcl * conf){
    conf->worker = 1;
    conf->socket = 1;
    conf->timer_ms = 1;
}
