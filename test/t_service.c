#include "ddclservice.h"
#include "ddcltimer.h"
#include "ddclthread.h"

#include <stdio.h>
#include <assert.h>
#include "ddcllog.h"

static void
svr_msg_call_resp(ddcl_Msg * msg) {
    if (msg->session % 100000 == 0) {
        ddcl_sleepms(100);
        ddcl_log(msg->self, "svr_msg_call_resp %ld  %s", msg->session, msg->data);
    }
}

static void
main_svr_msg_cb(ddcl_Msg * msg) {
}

#define SVR_COUNT 10
ddcl_Handle t_service(){
    printf("t_service: start\n");
    fflush(stdout);
    ddcl_Handle svrs[SVR_COUNT];
    for (int i = 0; i < SVR_COUNT; i++) {
        svrs[i] = ddcl_new_service(svr_msg_call_resp, svrs);
    }
    ddcl_Handle main_svr = ddcl_new_service_not_worker(main_svr_msg_cb, svrs);

    int index = 0;
    int count = 0;
    dduint64 t_start = ddcl_now();
    for (int i = 0; i < 5000000; i++) {
        int ret = ddcl_send_b(svrs[index], main_svr, DDCL_PTYPE_SEND,
            DDCL_CMD_TEXT, i, "lua test", strlen("lua test") + 1);
        assert(ret == 0);
        index++;
        if (index >= SVR_COUNT) {
            index = 0;
        }
    }
    dduint64 t_end = ddcl_now();

    ddcl_sleepms(1000);
    printf("t_service: %lld\n", t_end - t_start);
    fflush(stdout);
    return main_svr;
}
