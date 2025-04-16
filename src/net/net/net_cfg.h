#ifndef __NET_CFG_H__
#define __NET_CFG_H__
#include "dbg.h"
#define DBG_MBLOCK DBG_LEVEL_INFO
#define DBG_QUEUE  DBG_LEVEL_INFO
#define DBG_MSG    DBG_LEVEL_INFO
#define DBG_BUF    DBG_LEVEL_INFO
#define EXMSG_MSG_CNT  10 // 消息数量
#define EXMSG_LOCKER NLOCKER_THREAD // 消息锁类型


#define PKTBUF_BLK_SIZE  128 // 数据块大小
#define PKTBUF_BLK_CNT   100 // 数据块数量
#define PKTBUF_BUF_CNT   100
#endif