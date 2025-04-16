#include "mblock.h"
#include "nlist.h"
#include "dbg.h"

/**
 * @brief 初始化内存块管理器
 * 
 * @param mblock 指向内存块管理器的指针
 * @param mem 内存块的起始地址
 * @param blk_size 每个内存块的大小
 * @param cnt 内存块的数量
 * @param locker 锁的类型（如无锁、线程锁等）
 * @return net_err_t 返回初始化结果，成功返回 NET_ERR_OK，失败返回错误码
 * 
 * 核心操作：mem表示数组最开始的位置，将其转换为一个链表节点，插入到一个空闲链表中。
 * 然后不断遍历，就可以通过free_list来管理内存块的分配和释放。
 */
net_err_t mblock_init(mblock_t *mblock, void *mem, int blk_size, int cnt, nlocker_type_t locker) {
    // 将内存块的起始地址转换为字节指针，方便后续的指针运算
    uint8_t *buf = (uint8_t *)mem;

    // 初始化空闲链表，用于管理所有空闲的内存块
    nlist_init(&mblock->free_list);

    // 遍历所有内存块，将它们初始化并加入空闲链表
    for (int i = 0; i < cnt; i++, buf += blk_size) {
        // 将当前内存块的地址转换为链表节点类型
        nlist_node_t *block = (nlist_node_t *)(buf);

        // 初始化链表节点（将其前后指针置为 NULL）
        nodelist_init(block);

        // 将节点插入到空闲链表的尾部
        nlist_insert_last(&mblock->free_list, block);
    }

    // 初始化锁，根据传入的锁类型决定是否启用锁
    nlocker_init(&mblock->locker, locker);

    // 如果锁类型不是 NLOCKER_NONE，则需要创建信号量
    if (locker != NLOCKER_NONE) {
        // 创建信号量，初始值为内存块的数量
        mblock->alloc_sem = sys_sem_create(cnt);

        // 如果信号量创建失败，记录错误日志并销毁已初始化的锁
        if (mblock->alloc_sem == SYS_SEM_INVALID) {
            dbg_error(DBG_MBLOCK, "mblock_init: create semaphore failed\n");
            nlocker_destroy(&mblock->locker); // 销毁锁
            return NET_ERR_SYS; // 返回系统错误
        }
    }
    mblock->start = mem; // 设置内存块的起始地址

    // 初始化成功，返回 NET_ERR_OK
    return NET_ERR_OK;
}


void *mblock_alloc(mblock_t *mblock, int ms){
   if((ms < 0) || (mblock->locker.type == NLOCKER_NONE)){
    nlocker_lock(&mblock->locker); // 上锁，防止其他线程同时访问
    int count = nlist_count(&mblock->free_list); // 获取空闲内存块的数量
    if(count == 0){
        nlocker_unlock(&mblock->locker); // 解锁，允许其他线程访问
        return (void*)0; // 如果没有空闲内存块，返回 NULL
    }else{
        nlist_node_t *block = nlist_remove_first(&mblock->free_list); // 从空闲链表中移除第一个节点
        nlocker_unlock(&mblock->locker); // 解锁，允许其他线程访问
        return block; // 返回分配的内存块地址
    }
   }else{
    if(sys_sem_wait(mblock->alloc_sem, ms) < 0){ // 等待信号量，ms为超时时间
        return (void*)0; // 如果超时，返回 NULL
    }else{
        nlocker_lock(&mblock->locker); // 上锁，防止其他线程同时访问
        nlist_node_t *block = nlist_remove_first(&mblock->free_list); // 从空闲链表中移除第一个节点
        nlocker_unlock(&mblock->locker); // 解锁，允许其他线程访问
        return block; // 返回分配的内存块地址
    }
   }
}
int mblock_free_cnt(mblock_t *mblock){
    nlocker_lock(&mblock->locker); // 上锁，防止其他线程同时访问
    int count = nlist_count(&mblock->free_list); // 获取空闲内存块的数量
    nlocker_unlock(&mblock->locker); // 解锁，允许其他线程访问
    return count; // 返回空闲内存块的数量
}

void mblock_free(mblock_t *mblock, void *mem){
    nlocker_lock(&mblock->locker); // 上锁，防止其他线程同时访问
    nlist_insert_last(&mblock->free_list, (nlist_node_t *)mem); // 将释放的内存块插入到空闲链表的尾部
    nlocker_unlock(&mblock->locker); // 解锁，允许其他线程访问
    if(mblock->alloc_sem != SYS_SEM_INVALID){ // 如果信号量有效，通知等待的线程
        sys_sem_notify(mblock->alloc_sem); // 通知信号量
    }
}
void mblock_destroy(mblock_t *mblock){
    if(mblock->locker.type != NLOCKER_NONE){
        sys_sem_free(mblock->alloc_sem); // 释放信号量
        nlocker_destroy(&mblock->locker); // 销毁锁
    }
    mblock->start = (void*)0; // 清空内存块的起始地址
}