#ifndef __PKTBUF_H__
#define __PKTBUF_H__
#include "nlist.h"
#include "net_err.h"
#include "net_cfg.h"
#include "stdint.h"


typedef  struct _pktblk_t {
    nlist_node_t node; // 链表节点
    int size;         //数据包的大小
    uint8_t *data;   //数据块中的指针
    uint8_t payload[PKTBUF_BLK_SIZE]; //数据包的负载
}pktblk_t; //数据块结构


typedef struct _pktbuf_t{
    int total_size; //数据包的总大小
    nlist_t blk_list; //数据块链表
    nlist_node_t node; //链表节点

    int pos; //数据包的偏移量
    pktblk_t *curr_blk; //当前数据块指针
    uint8_t  *offset_blk;
}pktbuf_t; //数据包结构






static inline pktblk_t * pktbuf_blk_next(pktblk_t *blk) {
    nlist_node_t * next = nlist_node_next(&blk->node);
    return nlist_entry(next, pktblk_t, node);
}

static inline pktblk_t *pktblk_blk_next(pktblk_t *blk) {
    nlist_node_t *next = nlist_node_next(&blk->node);
    return nlist_entry(next, pktblk_t, node);
}


static inline pktblk_t *pktblk_first_blk (pktbuf_t *buf) {
    nlist_node_t *first = nlist_first(&buf->blk_list);
    return nlist_entry(first, pktblk_t, node);
} 

static inline pktblk_t *pktblk_last_blk (pktbuf_t *buf) {
    nlist_node_t *last = nlist_last(&buf->blk_list);
    return nlist_entry(last, pktblk_t, node);
} 

static inline int pktbuf_total(pktbuf_t *buf) {
    return buf->total_size; // 返回数据包的总大小
}



net_err_t pktbuf_init(void);
pktbuf_t *pktbuf_alloc(int size);
void pktbuf_free(pktbuf_t *buf);

net_err_t pktbuf_add_header(pktbuf_t *buf, int size, int cont);
net_err_t pktbuf_remove_header(pktbuf_t *buf, int size);
net_err_t pktbuf_resize(pktbuf_t *buf, int size);
net_err_t pktbuf_join(pktbuf_t *dest, pktbuf_t *src);
net_err_t pktbuf_set_cont(pktbuf_t *buf, int size);
void pktbuf_reset_acc(pktbuf_t *buf);
net_err_t  pktbuf_write(pktbuf_t *buf, uint8_t *src, int size);
net_err_t  pktbuf_read(pktbuf_t *buf, uint8_t *dest, int size);
#endif