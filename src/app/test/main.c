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
#include "netif.h"
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
    buf = pktbuf_alloc(2000);
    for(int i = 0; i <16; i++){
        pktbuf_add_header(buf, 33, 1);
    }
    for(int i = 0; i <16; i++){
        pktbuf_remove_header(buf, 33);
    }

    for(int i = 0; i <16; i++){
        pktbuf_add_header(buf, 33, 0);
    }
    for(int i = 0; i <16; i++){
        pktbuf_remove_header(buf, 33);
    }

    pktbuf_free(buf);
    buf = pktbuf_alloc(8);
    pktbuf_resize(buf, 16);
    pktbuf_resize(buf, 32);
    pktbuf_resize(buf, 288);
    pktbuf_resize(buf, 1024);
    pktbuf_resize(buf, 288);
    pktbuf_resize(buf, 32);
    pktbuf_resize(buf, 0);
    pktbuf_free(buf);

    buf = pktbuf_alloc(689);
    pktbuf_t *sbuf = pktbuf_alloc(892);
    pktbuf_join(buf, sbuf);
    pktbuf_free(buf);

    buf = pktbuf_alloc(32);
    pktbuf_join(buf, pktbuf_alloc(4));
    pktbuf_join(buf, pktbuf_alloc(16));
    pktbuf_join(buf, pktbuf_alloc(54));
    pktbuf_join(buf, pktbuf_alloc(32));
    pktbuf_join(buf, pktbuf_alloc(38));
    pktbuf_join(buf, pktbuf_alloc(512));


    pktbuf_reset_acc(buf);
    static uint16_t temp[1000];
    for(int i = 0; i < 1000; i++){
        temp[i] = i;
    }
    pktbuf_write(buf, (uint16_t *)temp, pktbuf_total(buf));

    static uint16_t read_temp[1000];
    plat_memset(read_temp, 0, sizeof(read_temp)); 
    pktbuf_reset_acc(buf);
    pktbuf_read(buf, (uint16_t *)read_temp, pktbuf_total(buf));
    if(plat_memcmp(temp, read_temp, pktbuf_total(buf)) != 0){
        plat_printf("read data error\n");
        return;
    }
    plat_memset(read_temp, 0, sizeof(read_temp));
    pktbuf_seek(buf, 18*2);
    pktbuf_read(buf, (uint16_t *)read_temp, 56);
    if(plat_memcmp(temp + 18, read_temp, 56) != 0){
        plat_printf("read data error\n");
        return;
    }
    plat_memset(read_temp, 0, sizeof(read_temp));
    pktbuf_seek(buf, 85*2);
    pktbuf_read(buf, (uint16_t *)read_temp, 256);
    if(plat_memcmp(temp + 85, read_temp, 256) != 0){
        plat_printf("read data error\n");
        return;
    }


    pktbuf_t *dest = pktbuf_alloc(1024);
    pktbuf_seek(dest, 600);
    pktbuf_seek(buf, 200);
    pktbuf_copy(dest, buf, 122);

    plat_memset(read_temp, 0, sizeof(read_temp));
    pktbuf_seek(dest, 600);
    pktbuf_read(dest, (uint16_t *)read_temp, 122);
    if(plat_memcmp(temp + 100, read_temp, 122) != 0){
        plat_printf("read data error\n");
        return;
    }
    pktbuf_seek(dest, 0);
    pktbuf_fill(dest, 53, pktbuf_total(dest));
    pktbuf_seek(dest, 0);
    pktbuf_read(dest, (uint16_t *)read_temp, pktbuf_total(dest));
    char *temp_ptr = (char *)read_temp;
    for(int i = 0; i <pktbuf_total(dest); i++){
        if(temp_ptr[i] != 53){
            plat_printf("read data error\n");
            return;
        }
    }


    pktbuf_free(dest);
    pktbuf_free(buf);





}




void basic_test(void){
    nlist_test();
    mblock_test();
    pktbuf_test();
    netif_init();
    loop_init();
    

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
