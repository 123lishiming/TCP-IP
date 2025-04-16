#ifndef __MBLOCK_H__
#define __MBLOCK_H__

#include "nlist.h"
#include "nlocker.h"

/*
功能:
提供内存块的分配和释放功能，支持多线程安全。
关键功能:
定义内存块管理器结构体 mblock_t。
提供内存块的初始化、分配、释放、销毁等接口：mblock_init、mblock_alloc、mblock_free、mblock_destroy。
*/
typedef struct _mblock_t {
    nlist_t free_list;      // 空闲链表
    void *start;            // 内存块起始地址
    nlocker_t locker;       // 锁
    sys_sem_t alloc_sem;    // 信号量
} mblock_t;                 // 注意这里需要以分号结束

net_err_t mblock_init(mblock_t *mblock, void *mem, int blk_size, int cn, nlocker_type_t type);
void *mblock_alloc(mblock_t *mblock, int ms);
int mblock_free_cnt(mblock_t *mblock);
void mblock_free(mblock_t *mblock, void *mem);
void mblock_destroy(mblock_t *mblock);
#endif // __MBLOCK_H__