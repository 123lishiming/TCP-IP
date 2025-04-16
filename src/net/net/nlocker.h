#ifndef __NLOCKER_H__
#define __NLOCKER_H__
#include "sys.h"
#include "net_err.h"

/*
功能:
定义锁的接口和数据结构，用于多线程同步。
关键功能:
定义锁类型枚举 nlocker_type_t。
提供锁的初始化、销毁、加锁、解锁等接口：nlocker_init、nlocker_destroy、nlocker_lock、nlocker_unlock。

*/
typedef enum _nlocker_type_t {
    NLOCKER_NONE, // 无锁
    NLOCKER_THREAD, //  线程锁
} nlocker_type_t;

typedef struct _nlocker_t {
    nlocker_type_t type; // 锁类型
    union 
    {
        sys_mutex_t mutex; // 互斥锁
        //... // 其他锁类型
    };
}nlocker_t;

net_err_t nlocker_init(nlocker_t *locker, nlocker_type_t type);
void nlocker_destroy(nlocker_t *locker);
void nlocker_lock(nlocker_t *locker);
void nlocker_unlock(nlocker_t *locker);
#endif
