#include "fixq.h"
#include "dbg.h"
#include "nlocker.h"
net_err_t fixq_init(fixq_t *q, void **buf, int size, nlocker_type_t locker){
    q->size = size;
    q->in = q->out = q->cnt = 0;
    q->buf = buf;
    q->send_sem = SYS_SEM_INVALID; // 初始化发送信号量
    q->recv_sem = SYS_SEM_INVALID; // 初始化接收信号量
    net_err_t err = nlocker_init(&q->locker, locker);
    if(err < 0){
        dbg_error(DBG_QUEUE, "init locker failed\n");
        return err;
    }

    q->send_sem = sys_sem_create(size); // 初始化发送信号量，初始值为size
    if(q->send_sem == SYS_SEM_INVALID){
        dbg_error(DBG_QUEUE, "create send semaphore failed\n");
        err = NET_ERR_SYS;
        goto init_failed;
    }


    q->recv_sem = sys_sem_create(0); // 初始化发送信号量，初始值为size
    if(q->recv_sem == SYS_SEM_INVALID){
        dbg_error(DBG_QUEUE, "create send semaphore failed\n");
        err = NET_ERR_SYS;
        goto init_failed;
    }
    //q->buf = buf; // 设置缓冲区指针
    return NET_ERR_OK; // 返回成功

init_failed:
    if(q->recv_sem != SYS_SEM_INVALID) // 如果接收信号量有效
        sys_sem_free(q->recv_sem); // 释放接收信号量
    nlocker_destroy(&q->locker); // 销毁锁
    return err; // 返回系统错误
}


net_err_t fixq_send(fixq_t *q, void *msg, int tms){
    nlocker_lock(&q->locker); // 上锁
    if((tms < 0) && (q->cnt >= q->size)){
        nlocker_unlock(&q->locker); // 解锁
        return NET_ERR_FULL; // 返回内存不足错误
    }
    nlocker_unlock(&q->locker); // 解锁

    if(sys_sem_wait(q->send_sem, tms) < 0){ // 等待发送信号量，超时tms毫秒
        return NET_ERR_TMS; // 返回系统错误
    }

    nlocker_lock(&q->locker); // 上锁
    q->buf[q->in] = msg; // 将消息放入缓冲区
    if(q->in >= q->size){ // 如果到达缓冲区末尾
        q->in = 0; // 重置到缓冲区开头
    }
    q->cnt++; // 增加消息计数
    nlocker_unlock(&q->locker); // 解锁

    /*当前向线程里面去写，但是缓存都是满的，那么就会卡在等待信号量这里，假设有一个线程去读数据，
    读消息的线程也会进行上锁，但是上不了锁，这把锁已经被别人拿走了，此时这个线程在等待这个空闲单元
    ，读的线程也在等待这个锁，两个线程都在等待，那么就会造成死锁*/

    sys_sem_notify(q->recv_sem); // 通知接收信号量，表示有新消息可读
    return NET_ERR_OK; // 返回成功
}


void *fixq_recv(fixq_t *q, int tms){
    nlocker_lock(&q->locker); // 上锁
    if(!q->cnt && (tms < 0)){
        nlocker_unlock(&q->locker); // 解锁
        return (void *)0; // 返回空指针
    }

    nlocker_unlock(&q->locker); // 解锁

    if(sys_sem_wait(q->recv_sem, tms) < 0){ // 等待接收信号量，超时tms毫秒
        return (void *)0; // 返回空指针
    }
    nlocker_lock(&q->locker); // 上锁
    void *msg = q->buf[q->out]; // 从缓冲区取出消息
    if(q->out >= q->size){ // 如果到达缓冲区末尾
        q->out = 0; // 重置到缓冲区开头
    }
    q->cnt--; // 减少消息计数
    nlocker_unlock(&q->locker); // 解锁
    sys_sem_notify(q->send_sem); // 通知发送信号量，表示有空闲消息可写
    return msg;
    
}

void fixq_destroy(fixq_t *q){
    nlocker_destroy(&q->locker); // 销毁锁
    sys_sem_free(q->send_sem); // 释放发送信号量
    sys_sem_free(q->recv_sem); // 释放接收信号量
}

int  fixq_count(fixq_t *q){
    nlocker_lock(&q->locker); // 上锁
    int count = q->cnt; // 获取消息计数
    nlocker_unlock(&q->locker); // 解锁
    return count; // 返回消息计数
}