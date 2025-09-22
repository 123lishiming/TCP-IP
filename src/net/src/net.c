#include "net.h"
#include <stdio.h>
#include "exmsg.h"
#include "net_plat.h"
#include "pktbuf.h"
#include "dbg.h"
#include "netif.h"
// 网络协议栈初始化
net_err_t net_init(void) {
    dbg_info(DBG_INIT, "Initializing network stack...\n");
    net_plat_init(); // 初始化平台相关
    exmsg_init(); // 初始化消息队列
    pktbuf_init(); // 初始化数据包缓冲区
    netif_init(); // 初始化网络接口
    loop_init(); // 初始化环回接口
    
    return NET_ERR_OK;
}

// 启动网络协议栈
net_err_t net_start(void) {
    dbg_info(DBG_INIT, "Starting network stack...\n");
    exmsg_start(); // 启动消息队列
    return NET_ERR_OK;
}