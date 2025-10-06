#include "timer.h"
#include "dbg.h"
#include "net_cfg.h"
#include "sys_plat.h"
static nlist_t timer_list;

// 定时器显示
#if DBG_DISP_ENABLED(DBG_TIMER)
static display_timer_list(void)
{
    plat_printf("-----------------timer list-------------\n");
    nlist_node_t *node;
    int index = 0;
    nlist_for_each(node,&timer_list){
        net_timer_t *timer = nlist_entry(node, net_timer_t, node);
        plat_printf("%d: %s, period=%d, curr: %d ms, reload : %d ms\n", 
            index++, timer->name, 
            timer->flags & NET_TIMER_RELOAD ? 1 :0,
            timer->curr, timer->reload 
        ); 
    }
    plat_printf("-----------------timer list end-------------\n");

}

#else
#define display_timer_list()

#endif // DEBUG

// 定时器初始化
net_err_t net_timer_init(void)
{
    dbg_info(DBG_TIMER, "timer init");
    nlist_init(&timer_list);

    dbg_info(DBG_TIMER, "done");
    return NET_ERR_OK;
}

// 定时器根据递增插入
static void insert_timer(net_timer_t *insert)
{
    nlist_node_t *node;
    nlist_for_each(node, &timer_list){
        net_timer_t *curr = nlist_entry(node, net_timer_t, node);
        // 计算剩余时间
        if(insert->curr > curr->curr){
            insert->curr -= curr->curr;
        }
        // 如果相等,插入到后面
        else if(insert->curr == curr->curr)
        {
            insert->curr = 0;
            nlist_insert_after(&timer_list, node, &insert->node);
            return;
        }
        // 如果小于,插入到前面
        else
        {
            curr->curr -= insert->curr;
            nlist_node_t *pre = nlist_node_prev(&curr->node);
            if(pre){
                nlist_insert_after(&timer_list,pre,&insert->node);
            }else{
                nlist_insert_first(&timer_list,&insert->node);
            }
            return;
        }
    }
    nlist_insert_last(&timer_list, &insert->node);
}

// 定时器增加
net_err_t net_timer_add(net_timer_t *timer, const char *name,timer_proc_t proc,
    void *arg,int ms,int flags)
{
    dbg_info(DBG_TIMER,"insert time: %s", name);
    plat_strncpy(timer->name, name, TIMER_NAME_SIZE);
    timer->name[TIMER_NAME_SIZE-1] = '\0';
    timer->reload = ms;
    timer->curr = ms;
    timer->proc = proc;
    timer->arg  = arg;
    timer->flags = flags;

    insert_timer(timer);
    display_timer_list();
    return NET_ERR_OK;

}

// 定时器删除函数
net_err_t net_timer_remove(net_timer_t *timer)
{
    dbg_info(DBG_TIMER,"remove timer: %s", timer->name);
    nlist_node_t * node;
    nlist_for_each(node, &timer_list){
           net_timer_t *curr = nlist_entry(node, net_timer_t, node);
           if(curr != timer){
            continue;
           }
           nlist_node_t *next = nlist_node_next(&timer->node);
            if(next){
                net_timer_t * next_timer = nlist_entry(next, net_timer_t, node);
                next_timer->curr += timer->curr;
            }
            nlist_remove(&timer_list, &timer->node);
            break;
    }
    display_timer_list();
 
}

// 定时器扫描,diff_ms是两次扫描定时器的间隔
net_err_t net_timer_check_two(int diff_ms)
{
    // 超时定时器放到列表里面
    nlist_t wait_list;
    nlist_init(&wait_list);
    nlist_node_t *node = nlist_first(&timer_list);
    while(node){
        nlist_node_t *next = nlist_node_next(node);
        net_timer_t *timer = nlist_entry(node, net_timer_t, node);
        if(timer->curr > diff_ms){ // 100 - 20 = 80
            timer->curr -= diff_ms;
            break;
        }
        diff_ms -= timer->curr;
        // timer_cur <= diff_ms
        timer->curr  = 0;
        nlist_remove(&timer_list,&timer->node);
        nlist_insert_last(&wait_list, &timer->node);
        node = next;
        // 200ms
    }
    while((node = nlist_remove_first(&wait_list)) != (nlist_node_t*)0)
    {
        net_timer_t *timer = nlist_entry(node, net_timer_t, node);
        timer->proc(timer, timer->arg);
        if(timer->flags & NET_TIMER_RELOAD) {
            timer->curr = timer->reload;
            insert_timer(timer);
        }
    }
    display_timer_list();
    return NET_ERR_OK;

}



int net_timer_first_tmo(void)
{
    nlist_node_t *node = nlist_first(&timer_list);
    if(node){
        net_timer_t *timer = nlist_entry(node, net_timer_t, node);
        return timer->curr;
    }
    return 0;
}