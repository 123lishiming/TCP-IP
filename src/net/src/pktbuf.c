#include "pktbuf.h"
#include "mblock.h"
#include "nlocker.h"
#include "dbg.h"
#include "sys_plat.h"
static nlocker_t locker; // 锁类型
static pktblk_t block_buffer[PKTBUF_BLK_CNT]; // 数据块缓冲区
static mblock_t block_list; // 数据块内存块管理器   
static pktbuf_t pktbuf_buffer[PKTBUF_BUF_CNT]; // 数据包缓冲区
static mblock_t pktbuf_list; // 数据包内存块管理器

static inline int curr_blk_tail_free(pktblk_t *blk){
   return (int)((blk->payload +PKTBUF_BLK_SIZE)-(blk->data + blk->size)); // 计算数据块的尾部大小
}


#if DBG_DISP_ENABLED(DBG_BUF)
static void display_check_buf(pktbuf_t *buf){
   if(!buf){
    dbg_error(DBG_BUF, "buf is null\n");
    return;
   }
   plat_printf("check buf %p:size %d\n", buf, buf->total_size); // 打印数据包指针和大小
   //遍历数据块链表，打印每个数据块的信息
   pktblk_t *curr;
   int index = 0, totol_size = 0;
   for(curr = pktblk_first_blk(buf); curr; curr = pktbuf_blk_next(curr)){
        plat_printf("%d: ", index++); 

        if(curr->data < curr->payload || curr->data >= curr->payload + PKTBUF_BLK_SIZE){
            dbg_error(DBG_BUF, "bad data pointer"); // 打印错误信息
        }

        int pre_size = (int)(curr->data - curr->payload); // 计算数据块的前面大小
        plat_printf("pre_size:%d b, ", pre_size); // 打印前面大小

        int used_size = curr->size; // 获取数据块的大小
        plat_printf("used:%d b, ", curr->size); // 打印数据块大小

        int free_size = curr_blk_tail_free(curr); // 计算数据块的尾部大小
        plat_printf("free:%d b, \n", free_size); // 打印尾部大小

        int blk_total = pre_size + used_size + free_size; // 计算数据块的总大小
        if(blk_total != PKTBUF_BLK_SIZE){
            dbg_error(DBG_BUF, "bad block size:%d != %d", blk_total, PKTBUF_BLK_SIZE); // 打印错误信息
        }
        totol_size += used_size; // 累加数据块的大小
    }
    if(totol_size != buf->total_size){
        dbg_error(DBG_BUF, "bad total size:%d != %d", totol_size, buf->total_size); // 打印错误信息
    }
    plat_printf("check buf done\n");
   
}
#else
#define display_check_buf(buf)
#endif

net_err_t pktbuf_init(void){
    dbg_info(DBG_BUF, "init pktbuf\n");
    nlocker_init(&locker, NLOCKER_THREAD); // 初始化锁
    mblock_init(&block_list, block_buffer, sizeof(pktblk_t), PKTBUF_BLK_CNT, NLOCKER_THREAD); // 初始化数据块内存块管理器
    mblock_init(&pktbuf_list, pktbuf_buffer, sizeof(pktbuf_t), PKTBUF_BUF_CNT, NLOCKER_THREAD); // 初始化数据包内存管理器
    dbg_info(DBG_BUF, "init done\n");
    return NET_ERR_OK;
}


// 分配数据块
static pktblk_t *pktblock_alloc(){
    pktblk_t *block = mblock_alloc(&block_list, -1); // 从内存块管理器中分配数据块
    if(block ){
        block->size = 0;
        block->data = (uint8_t *)0;
        nodelist_init(&block->node);

    }
    return block;

}
/*
size 是内存块中要分配出去的大小
*/
static pktblk_t *pktblock_alloc_list(int size, int add_front){
    pktblk_t *first_block = (pktblk_t *)0; // 记录第一个数据块
    pktblk_t *pre_block = (pktblk_t *)0;
    while(size){
        pktblk_t * new_block = pktblock_alloc(); // 分配数据块
        if(!new_block){
            dbg_error(DBG_BUF,"no buffer for alloc(%d)d", size);
            if (first_block) {
                // 失败，要回收释放整个链
                //pktblock_free_list(first_block);
            }
            /*如果分配到一半可以先进行回收*/
            return  (pktblk_t *)0;
        }
        int cur_size = 0;
        if(add_front){
            cur_size = size > PKTBUF_BLK_SIZE ? PKTBUF_BLK_SIZE : size; // 计算当前数据块的大小
            new_block->size = cur_size; // 设置数据块大小
            new_block->data = new_block->payload+PKTBUF_BLK_SIZE-cur_size; // 设置数据块数据指针
            if(first_block){
                nlist_node_set_next(&new_block->node, &first_block->node); // 设置下一个数据块的节点
            }
            first_block = new_block;
            


        }else{
            if(!first_block){
                first_block = new_block; // 记录第一个数据块
            }

            cur_size = size > PKTBUF_BLK_SIZE ? PKTBUF_BLK_SIZE : size; // 计算当前数据块的大小
            new_block ->size = cur_size; // 设置数据块大小
            new_block->data = new_block->payload; // 设置数据块数据指针
            if(pre_block){
                nlist_node_set_next(&pre_block->node, &new_block->node); // 设置前一个数据块的下一个节点
            }
        }
        size -= cur_size;
        pre_block = new_block; // 更新前一个数据块指针
        
    }
    return first_block;

}






static void pktbuf_insert_blk_list(pktbuf_t *buf, pktblk_t *first_blk, int add_front){
    //将链表的头部插入另外一块链表的尾部
    if(add_front){
        while(first_blk){
            pktblk_t *next_blk = pktbuf_blk_next(first_blk); // 获取下一个数据块
            nlist_insert_last(&buf->blk_list, &first_blk->node); // 将数据块插入到数据包链表的头部
            buf->total_size += first_blk->size; // 更新数据包总大小 
            first_blk = next_blk; // 更新当前数据块指针
        }
    }else{
        pktblk_t *pre = (pktblk_t *)0; //上一次插入的位置，第一次插入链表的前面，第二次插入到后面
        while(first_blk){
            pktblk_t *next_blk = pktblk_blk_next(first_blk); // 获取下一个数据块
            if(pre){
                nlist_insert_after(&buf->blk_list, &pre->node, &first_blk->node); // 将数据块插入到数据包链表的后面
            }else{
                nlist_insert_first(&buf->blk_list, &first_blk->node); // 将数据块插入到数据包链表的头部
            }
            buf->total_size += first_blk->size;
            pre = first_blk; // 更新前一个数据块指针
            first_blk = next_blk;
        }
    }
}
// 分配数据包
pktbuf_t *pktbuf_alloc(int size)
{
    pktbuf_t *buf = mblock_alloc(&pktbuf_list, -1); // 从内存块管理器中分配数据包
    if(!buf){
        dbg_error(DBG_BUF, "no buffer");
        return (pktbuf_t *)0;
    }
    buf->total_size = 0;
    nlist_init(&buf->blk_list); // 初始化数据包链表
    nodelist_init(&buf->node);
    if(size){
        pktblk_t *block = pktblock_alloc_list(size, 1); // 分配数据块,0是尾插法，1是头插法
        if(!block){
            mblock_free(&pktbuf_list, buf); // 释放数据包内存
            return (pktbuf_t *)0;
        }
        pktbuf_insert_blk_list(buf, block, 1); // 插入数据块到数据包链表
    }
    display_check_buf(buf);
    return buf; // 返回数据包指针

}


static void pktblock_free(pktblk_t *blk){
    mblock_free(&block_list, blk); // 释放数据块内存
}
static void  pktblock_free_list(pktblk_t *first_blk){
    // 释放数据块链表
    while(first_blk){
        pktblk_t *next_blk = pktblk_blk_next(first_blk); // 获取下一个数据块
        pktblock_free(first_blk); // 释放数据块
        first_blk = next_blk; // 更新当前数据块指针
    }
}


void pktbuf_free(pktbuf_t *buf)
{
    pktblock_free_list(pktblk_first_blk(buf)); // 释放数据包链表中的数据块
    mblock_free(&pktbuf_list, buf); // 释放数据包内存
}



net_err_t pktbuf_add_header(pktbuf_t *buf, int size, int cont)
{
    pktblk_t *block = pktblk_first_blk(buf); // 获取数据包链表中的第一个数据块  
    int resv_size = (int)(block->data -block->payload); // 计算数据块的前面大小
    if(size <= resv_size){
        block->size += size; // 更新数据块大小
        block->data -= size;  // 更新数据块数据指针
        buf->total_size += size; // 更新数据块总大小
        display_check_buf(buf); // 检查数据包
        return NET_ERR_OK; // 返回成功
    }

    if(cont){
        if(size > PKTBUF_BLK_SIZE){
            dbg_error(DBG_BUF, "set cont, size too big: %d > %d", size, PKTBUF_BLK_SIZE); // 打印错误信息
            return NET_ERR_SIZE; // 返回参数错误
        }
        block = pktblock_alloc_list(size, 1); // 分配数据块
        if(!block){
            dbg_error(DBG_BUF, "no buffer (size %d)", size); // 打印错误信息
            return NET_ERR_NONE; // 返回内存不足错误
        }
    }else{

    }
    pktbuf_insert_blk_list(buf, block, 0); // 插入数据块到数据包链表
    display_check_buf(buf); // 检查数据包
    return NET_ERR_OK; // 返回成功
}


net_err_t pktbuf_remove_header(pktbuf_t *buf, int size)
{
    pktblk_t *block = pktblk_first_blk(buf); // 获取数据包链表中的第一个数据块
    while(size){
        pktblk_t *next_blk = pktbuf_blk_next(block); // 获取下一个数据块
        if(size < block->size){
            block->data += size; // 更新数据块数据指针
            block->size -= size; // 更新数据块大小
            buf->total_size -= size; // 更新数据包总大小
            break;
        }
        int curr_size = block->size; // 获取当前数据块大小
        nlist_remove_first(&buf->blk_list); // 从数据包链表中移除第一个数据块
        pktblock_free(block); // 释放数据块

        size -= curr_size; // 更新剩余大小
        buf->total_size -= curr_size; // 更新数据包总大小
        block = next_blk; // 更新当前数据块指针
    }
    display_check_buf(buf); // 检查数据包
    return NET_ERR_OK; // 返回成功
}