#ifndef __NET_H__
#define __NET_H__

#include "net_err.h"


/*
作用:
提供网络协议栈的初始化和启动接口。
关键功能:
net_init: 初始化网络协议栈。
net_start: 启动网络协议栈。


*/
// Network protocol stack initialization
net_err_t net_init(void);

// Start the network protocol stack
net_err_t net_start(void);

#endif // __NET_H__