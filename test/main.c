
#include <stdio.h>

#include "ddcl.h"
#include "ddclservice.h"

extern void t_file();
extern void t_map();
extern void t_storage();
extern void t_thread();
extern ddcl_Handle t_service();
extern void t_socket(ddcl_Handle svr);

int main(){
    ddcl conf;
    ddcl_default(&conf);
    conf.worker = 2;
    int ret = ddcl_init(&conf);
    if(ret){
        printf("ddcl_init failed %d\n", ret);
        getc(stdin);
        return 1;
    }

    t_file();
    t_map();

    ddcl_Handle svr = t_service();
    //t_socket(svr);

    fflush(stdout);
    ddcl_start(svr);
    getc(stdin);
    return 0;
}
