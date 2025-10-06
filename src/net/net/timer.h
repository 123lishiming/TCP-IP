#ifndef __TIMER_H__
#define __TIMER_H__
#include "net_cfg.h"
#include "nlist.h"
#include "net_err.h"

#define NET_TIMER_RELOAD  (1 << 0)

struct _net_timer_t;
typedef void (*timer_proc_t)(struct _net_timer_t *timer, void *arg);
typedef struct _net_timer_t{
    char name[TIMER_NAME_SIZE];
    int flags;
    int curr;    //计数器，单位 ms
    int reload;   //定时器重载

    timer_proc_t proc;  //定时器函数指针
    void *arg;

    nlist_node_t node;
}net_timer_t;

net_err_t net_timer_init(void);
net_err_t net_timer_add(net_timer_t *timer, const char *name,timer_proc_t proc,
    void *arg,int ms,int flags);
net_err_t net_timer_remove(net_timer_t *timer);
net_err_t net_timer_check_two(int diff_ms);
int net_timer_first_tmo(void);
#endif