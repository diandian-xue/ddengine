#pragma once

#include "ddclconfig.h"
#include "ddcl.h"
#include "ddclthread.h"
#include "ddclstorage.h"
#include "ddclmsg.h"

#include <stdbool.h>


/* 启动服务框架
 */
DDCLAPI int
ddcl_init_service_module (ddcl * conf);

DDCLAPI ddcl_Service
ddcl_new_service (ddcl_MsgCB cb, void * ud);

DDCLAPI ddcl_Service
ddcl_new_service_not_worker (ddcl_MsgCB cb, void * ud);

DDCLAPI int
ddcl_exit_service (ddcl_Service h);

DDCLAPI int
ddcl_send (ddcl_Service to, ddcl_Service self, int ptype, 
    int cmd, ddcl_Session session, void * data, size_t sz);

DDCLAPI int
ddcl_send_b (ddcl_Service to, ddcl_Service self, int ptype, 
    int cmd, ddcl_Session session, void * data, size_t sz);

DDCLAPI int
ddcl_send_raw (ddcl_Service to, ddcl_Service self, int ptype, 
    int cmd, ddcl_Session session, void * data, size_t sz);

DDCLAPI int
ddcl_call (ddcl_Service to, ddcl_Service self, int ptype, 
    int cmd, ddcl_Session * session, void * data, size_t sz);

DDCLAPI int
ddcl_call_b (ddcl_Service to, ddcl_Service self, int ptype, 
    int cmd, ddcl_Session * session, void * data, size_t sz);

DDCLAPI int
ddcl_call_raw (ddcl_Service to, ddcl_Service self, int ptype, 
    int cmd, ddcl_Session * session, void * data, size_t sz);

DDCLAPI int
ddcl_set_service_ud (ddcl_Service h, void * ud);

DDCLAPI void *
ddcl_get_service_ud (ddcl_Service h);

DDCLAPI int
ddcl_start (ddcl_Service h);

DDCLAPI int
ddcl_dispatch (ddcl_Service h);

DDCLAPI int
ddcl_timeout (ddcl_Service h, ddcl_Session * session, dduint32 ms);

DDCLAPI void
ddcl_signal_monitor();

DDCLAPI int
ddcl_new_session(ddcl_Service h, ddcl_Session * session);
