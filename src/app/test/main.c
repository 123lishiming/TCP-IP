/**
 * @file main.c
 * @brief 测试主程序，完成一些简单的测试主程序
 * @note 该源码配套相应的视频课程，请见源码仓库下面的README.md
 */

#include <string.h>
#include "sys_plat.h"
#include "echo/tcp_echo_client.h"
#include "echo/tcp_echo_server.h"
#include "net.h"
#include "netif_pcap.h"
#include "dbg.h"
#include "nlist.h"
#include "mblock.h"
#include "pktbuf.h"
static int count;
static sys_sem_t sem;
static sys_mutex_t mutex;
static char buffer[100]; //实现环形缓冲区
int write_index, read_index;
static sys_sem_t read_sem;
static sys_sem_t write_sem;



/*
    * @brief 线程入口函数
    * @param arg 传入的参数
*/
void  thread1_entry (void *arg)
{
    for(int i = 0; i < 2 * sizeof(buffer); i++){
        sys_sem_wait(read_sem, 0);  // 等待读信号量，超时1秒
        char data = buffer[read_index++];
        if(read_index >= sizeof(buffer)){
            read_index = 0;
        }
        sys_sem_notify(write_sem);  // 通知写信号量,因为有一个数据被读走了
        plat_printf("thread 1:read data: %d\n", (int)data);
        sys_sleep(100);  // 延时1秒
    }
    printf("thread1 count: %d\n", count);
    while(1){
        plat_printf("this is thread1: %s\n", (char *)arg);
        sys_sleep(1000);  // 延时1秒
        sys_sem_notify(sem);  // 通知信号量
        sys_sleep(1000);  // 延时1秒
    }

}

/*
    * @brief 线程入口函数
    * @param arg 传入的参数
*/
void  thread2_entry (void *arg)
{
    sys_sleep(100);  // 延时1秒
    for(int i = 0; i < 2 * sizeof(buffer); i++){
        sys_sem_wait(write_sem, 0);  // 等待写信号量，超时1秒
        buffer[write_index++] = i;
        if(write_index >= sizeof(buffer)){
            write_index = 0;
        }

        plat_printf("thread 2: write data: %d\n", i);
        sys_sem_notify(read_sem);  // 通知读信号量
    }
    printf("thread2 count: %d\n", count);
    while(1){
        sys_sem_wait(sem, 0);  // 等待信号量，超时1秒
        plat_printf("this is thread2: %s\n", (char *)arg);
    }

}

/**
 * @brief 测试入口
 */


 /*
 网卡驱动初始化
 */
typedef struct _tnode_t {
    int id;
    nlist_node_t node;
}tnode_t;


void nlist_test(){
    #define NODE_CNT 4
    tnode_t node[NODE_CNT];
    nlist_t list;

    nlist_init(&list);
    for(int i = 0; i < NODE_CNT; i++){
        node[i].id = i;
        nlist_insert_first(&list, &node[i].node);
    }
    plat_printf("insert first\n");
    nlist_node_t  *p;
    nlist_for_each(p, &list){
        tnode_t *tnode = nlist_entry(p, tnode_t, node);
        plat_printf("id: %d\n", tnode->id);
    }

    plat_printf("remove first\n");
    for(int i = 0; i < NODE_CNT; i++){
        p = nlist_remove_first(&list);
        plat_printf("id: %d\n", nlist_entry(p, tnode_t, node)->id);
    }

    for(int i = 0; i < NODE_CNT; i++){
        node[i].id = i;
        nlist_insert_last(&list, &node[i].node);
    }
    plat_printf("insert last\n");
    nlist_for_each(p, &list){
        tnode_t *tnode = nlist_entry(p, tnode_t, node);
        plat_printf("id: %d\n", tnode->id);
    }

    plat_printf("remove last\n");
    for(int i = 0; i < NODE_CNT; i++){
        p = nlist_remove_last(&list);
        plat_printf("id: %d\n", nlist_entry(p, tnode_t, node)->id);
    }
    

    plat_printf("insert after\n");
    for(int i = 0; i < NODE_CNT; i++){
        node[i].id = i;
        nlist_insert_after(&list,nlist_first(&list), &node[i].node);
    }
    nlist_for_each(p, &list){
        tnode_t *tnode = nlist_entry(p, tnode_t, node);
        plat_printf("id: %d\n", tnode->id);
    }
   

}


void mblock_test(){
    mblock_t blist;
    static uint8_t buffer[100][10];  // 10个100字节的内存块
    mblock_init(&blist, buffer, 100, 10, NLOCKER_THREAD);
    void *temp[10];
    for(int i = 0; i < 10; i++){
        temp[i] = mblock_alloc(&blist, -1);
        plat_printf("block :%p, free_count: %d\n", temp[i], mblock_free_cnt(&blist));
    }
    for(int i = 0; i < 10; i++){
        mblock_free(&blist, temp[i]);
        plat_printf("free_count: %d\n",mblock_free_cnt(&blist));
    }

    mblock_destroy(&blist);
}

void pktbuf_test(){
    pktbuf_init();
    pktbuf_t *buf = pktbuf_alloc(2000);
    pktbuf_free(buf);
}




void basic_test(void){
    nlist_test();
    mblock_test();
    pktbuf_test();
}

 net_err_t netdev_init(void)
 {
    netif_pcap_open();
    return NET_ERR_OK;
 }

#define DBG_TEST DBG_LEVEL_INFO
int main (int argc, char *argv[]) {
    dbg_info(DBG_TEST, "info");
    dbg_warning(DBG_TEST, "warning");
    dbg_error(DBG_TEST, "error");

    dbg_assert(1 == 1, "failed");
    basic_test();

     /*协议栈的初始化*/
    net_init();  // 初始化网络协议栈
    net_start(); // 启动网络协议栈
    netdev_init(); // 初始化网卡驱动
    while(1){
        sys_sleep(1000);  // 延时1秒
    }

}
