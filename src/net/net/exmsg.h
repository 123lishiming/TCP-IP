#ifndef __EXMSG_H__
#define __EXMSG_H__

#include "net_err.h"
#include "fixq.h"
#include "nlist.h"
#include "netif.h"
/*
作用:
定义消息模块的接口和数据结构，用于网络协议栈中消息的传递和处理。
关键功能:
定义消息类型 exmsg_t，包括链表节点和消息类型字段。
提供消息模块的初始化和启动接口：exmsg_init、exmsg_start。
提供接收网络接口消息的接口：exmsg_netif_in

*/
typedef struct _exmsg_t {
    nlist_node_t node; // 链表节点
    enum{
        NET_EXMSG_NETIF_IN,
    }type;

    int id;
}exmsg_t; // 消息类型
// 初始化消息模块
net_err_t exmsg_init(void);

// 启动消息模块
net_err_t exmsg_start(void);

net_err_t exmsg_netif_in(netif_t *netif); // 接收网络接口消息

#endif // __EXMSG_H__