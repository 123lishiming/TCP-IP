#include "net.h"
#include <stdio.h>
#include "exmsg.h"
#include "net_plat.h"
#include "pktbuf.h"
// 网络协议栈初始化
net_err_t net_init(void) {
    printf("Initializing network stack...\n");
    net_plat_init(); // 初始化平台相关
    exmsg_init(); // 初始化消息队列
    pktbuf_init(); // 初始化数据包缓冲区
    return NET_ERR_OK;
}

// 启动网络协议栈
net_err_t net_start(void) {
    printf("Starting network stack...\n");
    exmsg_start(); // 启动消息队列
    return NET_ERR_OK;
}