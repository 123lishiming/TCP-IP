#ifndef __NET_CFG_H__
#define __NET_CFG_H__
#include "dbg.h"
#define DBG_MBLOCK DBG_LEVEL_INFO
#define DBG_QUEUE  DBG_LEVEL_INFO
#define DBG_MSG    DBG_LEVEL_INFO
#define DBG_BUF    DBG_LEVEL_INFO
#define DBG_INIT   DBG_LEVEL_INFO
#define DBG_PLAT   DBG_LEVEL_INFO
#define DBG_NETIF  DBG_LEVEL_INFO
#define DBG_ETHER  DBG_LEVEL_INFO
#define DGB_TOOLS  DBG_LEVEL_INFO
#define DBG_TIMER  DBG_LEVEL_NONE
#define DBG_ARP    DBG_LEVEL_INFO

#define NET_ENDIAN_LITTLE  1 // 大小端模式

#define EXMSG_MSG_CNT  10 // 消息数量
#define EXMSG_LOCKER NLOCKER_THREAD // 消息锁类型


#define PKTBUF_BLK_SIZE  128 // 数据块大小
#define PKTBUF_BLK_CNT   100 // 数据块数量
#define PKTBUF_BUF_CNT   100



#define NETIF_NAME_MAX  10 // 网络接口名称最大长度
#define NETIF_HWADDR_SIZE  10 // 硬件地址长度
#define NETIF_INO_SIZE  50 // 输入队列大小
#define NETIF_OUT_SIZE  50 // 输出队列大小

#define NETIF_QUTO_SIZE  50 // 网络接口队列大小
#define NETITF_DEV_CNT   20 // 网络接口设备数量
#define ETHER_DATA_MINSIZE 46 // 以太网数据最小长度


#define TIMER_NAME_SIZE 32 //定时器名称大小
#define ARP_CACHE_SIZE  50 //ARP的大小

#endif