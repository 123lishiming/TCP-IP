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
    uint8_t payload[PKTBUF_BLK_SIZE]; //数据包的实际数据
}pktblk_t; //数据块结构


typedef struct _pktbuf_t{
    int total_size; //数据包的总大小
    nlist_t blk_list; //数据块链表
    nlist_node_t node; //链表节点

}pktbuf_t; //数据包结构

static inline pktblk_t * pktbuf_blk_next(pktblk_t *blk) {
    nlist_node_t * next = nlist_node_next(&blk->node);
    return nlist_entry(next, pktblk_t, node);
}

static inline pktblk_t *pktblk_blk_next(pktblk_t *blk) {
    nlist_node_t *next = nlist_node_next(&blk->node);
    return nlist_entry(next, pktblk_t, node);
}

net_err_t pktbuf_init(void);
pktbuf_t *pktbuf_alloc(int size);
void pktbuf_free(pktbuf_t *buf);

#endif