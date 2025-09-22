#include "exmsg.h"
#include "sys_plat.h"
#include "dbg.h"
#include "fixq.h"
#include "mblock.h"
static  void *msg_tbl[EXMSG_MSG_CNT];
static fixq_t msg_queue; // 消息队列

static exmsg_t msg_buffer[EXMSG_MSG_CNT]; // 消息缓冲区
static mblock_t msg_mblock; // 消息内存块

/**
 * @brief 初始化消息模块
 * 
 * @return net_err_t 返回初始化结果，成功返回 NET_ERR_OK，失败返回错误码
 */
net_err_t exmsg_init(void) {
    // 打印调试信息，表示消息模块初始化开始
    dbg_info(DBG_MSG, "exmsg_init\n");

    // 初始化消息队列
    // 参数说明：
    // - &msg_queue: 指向消息队列的指针
    // - msg_tbl: 消息队列的缓冲区，用于存储消息指针
    // - EXMSG_MSG_CNT: 消息队列的最大容量
    // - EXMSG_LOCKER: 消息队列的锁类型（如线程锁）
    net_err_t err = fixq_init(&msg_queue, msg_tbl, EXMSG_MSG_CNT, EXMSG_LOCKER);
    if (err < 0) {
        // 如果消息队列初始化失败，打印错误信息并返回错误码
        dbg_error(DBG_MSG, "exmsg_init: fixq_init failed\n");
        return err;
    }

    // 初始化消息内存块
    // 参数说明：
    // - &msg_mblock: 指向消息内存块管理器的指针
    // - msg_buffer: 消息缓冲区，用于存储实际的消息结构体
    // - sizeof(exmsg_t): 每个消息结构体的大小
    // - EXMSG_MSG_CNT: 消息内存块的最大数量
    // - EXMSG_LOCKER: 消息内存块的锁类型（如线程锁）
    err = mblock_init(&msg_mblock, msg_buffer, sizeof(exmsg_t), EXMSG_MSG_CNT, EXMSG_LOCKER);
    if (err < 0) {
        // 如果消息内存块初始化失败，打印错误信息并返回错误码
        dbg_error(DBG_MSG, "exmsg_init: mblock_init failed\n");
        return err;
    }

    // 打印调试信息，表示消息模块初始化完成
    dbg_info(DBG_MSG, "init_done\n");

    // 返回成功状态
    return NET_ERR_OK;
}
// 消息处理线程入口函数
static void exmsg_thread_entry(void *arg)
{
    dbg_info(DBG_MSG, "exmsg is runing.....\n");
    while(1){
        exmsg_t *msg = (exmsg_t *)fixq_recv(&msg_queue, 0); // 从消息队列中接收消息
        plat_printf("recv a msg type: %d, id: %d\n", msg->type,msg->id);
        mblock_free(&msg_mblock, msg); // 释放消息内存块
        sys_sleep(1);
    }
}
//线程启动函数
net_err_t  exmsg_start(void)
{
    sys_thread_t thread = sys_thread_create(exmsg_thread_entry, (void *)0);
    if(thread == SYS_THREAD_INVALID) {
        return NET_ERR_SYS;
    }
    return NET_ERR_OK;

}

// 构造数据包到达的消息，然后发给工作线程
net_err_t exmsg_netif_in(netif_t *netif){
    exmsg_t *msg = mblock_alloc(&msg_mblock, -1); // 从内存块中分配一个消息结构体
    if(msg == NULL){
        dbg_warning(DBG_MSG, "no free msg\n");
        return NET_ERR_MEM;
    }
    static int id = 0;
    msg->type = NET_EXMSG_NETIF_IN; // 设置消息类型
    msg->id = id++; // 设置消息ID

    net_err_t err = fixq_send(&msg_queue, msg, -1); // 将消息发送到消息队列中
    if(err < 0){
        dbg_warning(DBG_MSG, "send msg failed\n");
        mblock_free(&msg_mblock, msg); // 释放消息内存块
        return err;
    }

    return err;
}

