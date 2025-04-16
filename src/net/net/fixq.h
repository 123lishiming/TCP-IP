#ifndef __FIXQ_H__
#define __FIXQ_H__
#include "net_err.h"
#include "sys.h"
#include "nlocker.h"

/*
作用:
定义固定大小队列的接口和数据结构，用于消息的发送和接收。
关键功能:
定义固定大小队列结构体 fixq_t。
提供队列的初始化、发送、接收、销毁等接口：fixq_init、fixq_send、fixq_recv、fixq_destroy。
*/
typedef struct _fixq_t{
    int size;
    int in, out, cnt;
    void **buf;

    nlocker_t locker; // 锁
    sys_sem_t recv_sem; // 接收信号量
    sys_sem_t send_sem; // 发送信号量

}fixq_t; // 定义固定大小队列结构体

net_err_t fixq_init(fixq_t *q, void **buf, int size, nlocker_type_t locker);
net_err_t fixq_send(fixq_t *q, void *msg, int tms);
void *fixq_recv(fixq_t *q, int tms);
void fixq_destroy(fixq_t *q);
int  fixq_count(fixq_t *q);
#endif // __FIXQ_H__