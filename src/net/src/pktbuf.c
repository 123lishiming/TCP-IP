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
static inline int total_blk_remain(pktbuf_t *buf)
{
    return buf->total_size -buf->pos; // 计算数据包剩余大小
}


static inline int curr_remain_size(pktbuf_t *buf){
    pktblk_t *blk = buf->curr_blk; // 获取当前数据块
    if(!blk){
        return 0; // 如果当前数据块为空，返回0
    }
    return(int)(buf->curr_blk->data +  blk ->size - buf->offset_blk); // 计算数据块的大小
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
    nlocker_lock(&locker); // 锁定
    pktblk_t *block = mblock_alloc(&block_list, -1); // 从内存块管理器中分配数据块
    nlocker_unlock(&locker); // 解锁
    if(block ){
        block->size = 0;
        block->data = (uint8_t *)0;
        nodelist_init(&block->node);

    }
    return block;

}

static void pktblock_free(pktblk_t *blk){
    nlocker_lock(&locker); // 锁定
    mblock_free(&block_list, blk); // 释放数据块内存
    nlocker_unlock(&locker); // 解锁
}
// Ensure this is the only definition of pktblock_free_list
static void pktblock_free_list(pktblk_t *first_blk){
    // 释放数据块链表
    while(first_blk){
        pktblk_t *next_blk = pktblk_blk_next(first_blk); // 获取下一个数据块
        pktblock_free(first_blk); // 释放数据块
        first_blk = next_blk; // 更新当前数据块指针
    }
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
                //失败，要回收释放整个链
                pktblock_free_list(first_block);
            }
            /*如果分配到一半可以先进行回收*/
            return  (pktblk_t *)0;
        }
        int cur_size = 0;
        // 头插法和尾插法
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


// 
void pktbuf_inc_ref(pktbuf_t *buf){
    nlocker_lock(&locker); // 锁定
    buf->ref++;
    nlocker_unlock(&locker); // 解锁
}
// 分配数据包
pktbuf_t *pktbuf_alloc(int size)
{
    nlocker_lock(&locker); // 锁定
    pktbuf_t *buf = mblock_alloc(&pktbuf_list, -1); // 从内存块管理器中分配数据包
    nlocker_unlock(&locker); // 解锁
    if(!buf){
        dbg_error(DBG_BUF, "no buffer");
        return (pktbuf_t *)0;
    }
    buf->total_size = 0;
    buf->ref = 1; // 引用计数
    nlist_init(&buf->blk_list); // 初始化数据包链表
    nodelist_init(&buf->node);
    if(size){
        pktblk_t *block = pktblock_alloc_list(size, 1); // 分配数据块,0是尾插法，1是头插法
        if(!block){
            nlocker_lock(&locker); // 锁定
            mblock_free(&pktbuf_list, buf); // 释放数据包内存
            nlocker_unlock(&locker); // 解锁
            return (pktbuf_t *)0;
        }
        pktbuf_insert_blk_list(buf, block, 1); // 插入数据块到数据包链表
    }
    display_check_buf(buf);
    return buf; // 返回数据包指针

}




void pktbuf_free(pktbuf_t *buf)
{
    nlocker_lock(&locker);
    if(--buf->ref == 0){
    pktblock_free_list(pktblk_first_blk(buf)); // 释放数据包链表中的数据块
    mblock_free(&pktbuf_list, buf); // 释放数据包内存
    nlocker_unlock(&locker);
    }
    
}



net_err_t pktbuf_add_header(pktbuf_t *buf, int size, int cont)
{
    dbg_assert(buf->ref !=0, "buf ref is 0\n"); // 断言数据包引用计数不为0
    pktblk_t *block = pktblk_first_blk(buf); // 获取数据包链表中的第一个数据块  
    int resv_size = (int)(block->data -block->payload); // 计算数据块的前面大小
    if(size <= resv_size){
        block->size += size; // 更新数据块大小
        block->data -= size;  // 更新数据块数据指针
        buf->total_size += size; // 更新数据块总大小
        display_check_buf(buf); // 检查数据包
        return NET_ERR_OK; // 返回成功
    }
    // 添加连续包头和非连续包头 
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
        block->data = block->payload; // 设置数据块数据指针
        block->size += resv_size; // 更新数据块大小
        buf->total_size += resv_size; // 更新数据包总大小
        size -= resv_size; // 更新剩余大小
        block = pktblock_alloc_list(size, 1); // 分配数据块
        if(!block){
            dbg_error(DBG_BUF, "no buffer (size %d)", size); // 打印错误信息
            return NET_ERR_NONE; // 返回内存不足错误
        }
    }
    pktbuf_insert_blk_list(buf, block, 0); // 插入数据块到数据包链表
    display_check_buf(buf); // 检查数据包
    return NET_ERR_OK; // 返回成功
}


net_err_t pktbuf_remove_header(pktbuf_t *buf, int size)
{
    dbg_assert(buf->ref !=0, "buf ref is 0\n"); // 断言数据包引用计数不为0
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

net_err_t pktbuf_resize(pktbuf_t *buf, int size){
    dbg_assert(buf->ref !=0, "buf ref is 0\n"); // 断言数据包引用计数不为0
    if(size == buf->total_size){
        return NET_ERR_OK; // 如果大小相等，直接返回成功
    }

    if(buf->total_size == 0){
        pktblk_t *block = pktblock_alloc_list(size, 0);
        if(!block){
            dbg_error(DBG_BUF, "no buffer (size %d)", size); // 打印错误信息
            return NET_ERR_NONE; // 返回内存不足错误
        }
        pktbuf_insert_blk_list(buf, block, 1); // 插入数据块到数据包链表
    }else if(size == 0){
        pktblock_free_list(pktblk_first_blk(buf)); // 释放数据包链表中的数据块
        buf->total_size = 0; // 更新数据包总大小
        nlist_init(&buf->blk_list); // 初始化数据包链表
    }
    else if(size > buf->total_size){
        pktblk_t *tail_blk = pktblk_last_blk(buf); // 获取数据包链表中的最后一个数据块
        int inc_size = size - buf->total_size; // 计算增加的大小
        int remain_size = curr_blk_tail_free(tail_blk); // 计算数据块的尾部大小
        if(inc_size <= remain_size){
            tail_blk->size += inc_size; // 更新数据块大小
            buf->total_size += inc_size; // 更新数据包总大小
        }else{
            pktblk_t *new_block = pktblock_alloc_list(inc_size, 0); // 分配数据块
            if(!new_block){
                dbg_error(DBG_BUF, "no buffer (size %d)", inc_size); // 打印错误信息
                return NET_ERR_NONE; // 返回内存不足错误
            }
            tail_blk->size += remain_size; // 更新数据块大小
            buf->total_size += remain_size; // 更新数据包总大小
            pktbuf_insert_blk_list(buf, new_block, 1); // 插入数据块到数据包链表
        }
    }else{
        int total_size = 0;
        pktblk_t *tail_blk;
        for(tail_blk = pktblk_first_blk(buf); tail_blk; tail_blk = pktblk_blk_next(tail_blk)){
            total_size += tail_blk->size; // 计算数据块的总大小
            if(total_size >= size){
                break; // 如果总大小大于等于目标大小，跳出循环
            }
        }
        if(tail_blk == (pktblk_t *)0){
            return NET_ERR_SIZE; // 返回参数错误
        }
        int total_size_blk = total_size; // 记录数据块的总大小

        pktblk_t *curr_blk = pktbuf_blk_next(tail_blk); // 获取下一个数据块
        while(curr_blk){
            pktblk_t *next_blk = pktblk_blk_next(curr_blk); // 获取下一个数据块
            total_size_blk += curr_blk->size; // 计算数据块的总大小
            nlist_remove(&buf->blk_list, &curr_blk->node); // 从数据包链表中移除当前数据块
            pktblock_free(curr_blk); // 释放数据块
            curr_blk = next_blk; // 更新当前数据块指针
        }
       
        tail_blk->size -=  (total_size - size); // 更新数据块大小
        buf->total_size = size; // 更新数据包总大小


    }
    display_check_buf(buf); // 检查数据包
    return NET_ERR_OK; // 返回成功

}


net_err_t pktbuf_join(pktbuf_t *dest, pktbuf_t *src)
{
    dbg_assert(dest->ref !=0, "dest ref is 0\n"); // 断言数据包引用计数不为0
    dbg_assert(src->ref !=0, "src ref is 0\n"); // 断言数据包引用计数不为0
    pktblk_t *first_blk;
    while((first_blk = pktblk_first_blk(src))){
        nlist_remove_first(&src->blk_list); // 从源数据包链表中移除第一个数据块
        pktbuf_insert_blk_list(dest, first_blk, 1); // 插入数据块到目标数据包链表
    }
    pktbuf_free(src); // 释放源数据包内存
    display_check_buf(dest); // 检查目标数据包
    return NET_ERR_OK; // 返回成功
}


net_err_t pktbuf_set_cont(pktbuf_t *buf, int size){
    dbg_assert(buf->ref !=0, "buf ref is 0\n"); // 断言数据包引用计数不为0
    if(size > buf->total_size){
        dbg_error(DBG_BUF, "size %d > total size %d", size, buf->total_size); // 打印错误信息
        return NET_ERR_SIZE; // 返回参数错误
    }

    if(size > PKTBUF_BLK_SIZE){
        dbg_error(DBG_BUF, "size %d > PKTBUF_BLK_SIZE %d", size, PKTBUF_BLK_SIZE); // 打印错误信息
        return NET_ERR_SIZE; // 返回参数错误
    }

    pktblk_t *first_blk = pktblk_first_blk(buf); // 获取数据包链表中的第一个数据块

    if(size <= first_blk->size){
        display_check_buf(buf); // 检查数据包
        return NET_ERR_OK; // 如果大小小于等于第一个数据块，直接返回成功
    }
    
    uint8_t *dest = first_blk->payload;  // 设置目标数据块指针
    for(int i = 0; i < first_blk->size; i++){
        *dest++ = first_blk->data[i]; // 将数据块中的数据复制到目标数据块中
    }
    first_blk->data = first_blk->payload; // 设置数据块数据指针

    int remain_size = size -first_blk->size; // 计算剩余大小
    pktblk_t *curr_blk = pktbuf_blk_next(first_blk); // 获取下一个数据块
    while(remain_size && curr_blk){
        int curr_size = (curr_blk->size > remain_size) ? remain_size : curr_blk->size; // 计算当前数据块的大小
        plat_memcpy(dest, curr_blk->data, curr_size); // 将数据块中的数据复制到目标数据块中
        dest += curr_size; // 更新目标数据块指针
        curr_blk->data += curr_size; // 更新数据块数据指针
        curr_blk->size -= curr_size; // 更新数据块大小
        first_blk->size += curr_size; // 更新第一个数据块大小
        remain_size -= curr_size; // 更新剩余大小
        if(curr_blk->size == 0){
            pktblk_t *next_blk = pktblk_blk_next(curr_blk); // 获取下一个数据块
            nlist_remove(&buf->blk_list, &curr_blk->node); // 从数据包链表中移除当前数据块
            pktblock_free(curr_blk); // 释放数据块
            curr_blk = next_blk; // 更新当前数据块指针
        }
        
    }
    display_check_buf(buf); // 检查数据包
    return NET_ERR_OK; // 返回成功
   


}

void pktbuf_reset_acc(pktbuf_t *buf)
{
    dbg_assert(buf->ref !=0, "buf ref is 0\n"); // 断言数据包引用计数不为0
    if(buf){
        buf->pos = 0;
        buf->curr_blk = pktblk_first_blk(buf); // 获取数据包链表中的第一个数据块
        buf->offset_blk =buf->curr_blk? buf->curr_blk->data: (uint8_t *)0; // 设置数据块数据指针
    }
}

static void move_forward(pktbuf_t *buf, int size)
{
    pktblk_t *curr = buf->curr_blk; // 获取当前数据块
    buf->pos += size; // 更新数据包偏移量
    buf->offset_blk += size; // 更新数据块数据指针 
    if(buf->offset_blk >= curr->data + curr->size){
        buf->curr_blk = pktblk_blk_next(curr); // 获取下一个数据块
        if(buf->curr_blk){
            buf->offset_blk = buf->curr_blk->data; // 设置数据块数据指针
        }else{
            buf->offset_blk = (uint8_t *)0; // 如果没有下一个数据块，设置数据块数据指针为空
        }
    }
}
net_err_t pktbuf_write(pktbuf_t *buf, uint8_t *src, int size)
{
    dbg_assert(buf->ref !=0, "buf ref is 0\n"); // 断言数据包引用计数不为0
   if(!src || !size){
        return NET_ERR_PARAM; // 返回参数错误
   }
   int remain_size = total_blk_remain(buf); // 计算剩余大小
   if(remain_size < size){
        dbg_error(DBG_BUF, "no buffer for write (%d)", size); // 打印错误信息
        return NET_ERR_SIZE; // 返回参数错误
   }
   while(size){
    int blk_size = curr_remain_size(buf); // 计算数据块的大小
    int copy_size = (size > blk_size) ? blk_size : size; // 计算复制的大小
    plat_memcpy(buf->offset_blk, src, copy_size); // 将数据复制到数据块中
    src += copy_size; // 更新源数据指针
    size -= copy_size; // 更新剩余大小
    move_forward(buf, copy_size); // 更新数据包偏移量和数据块数据指针
   }
   return NET_ERR_OK; // 返回成功
}



net_err_t  pktbuf_read(pktbuf_t *buf, uint8_t *dest, int size){
    dbg_assert(buf->ref !=0, "buf ref is 0\n"); // 断言数据包引用计数不为0
    if(!dest || !size){
        return NET_ERR_PARAM; // 返回参数错误
   }
   int remain_size = total_blk_remain(buf); // 计算剩余大小
   if(remain_size < size){
        dbg_error(DBG_BUF, "no buffer for write (%d)", size); // 打印错误信息
        return NET_ERR_SIZE; // 返回参数错误
   }
   while(size > 0){
    int blk_size = curr_remain_size(buf); // 计算数据块的大小
    int copy_size = (size > blk_size) ? blk_size : size; // 计算复制的大小
    plat_memcpy(dest, buf->offset_blk, copy_size); // 将数据复制到数据块中
    dest += copy_size; // 更新源数据指针
    size -= copy_size; // 更新剩余大小
    move_forward(buf, copy_size); // 更新数据包偏移量和数据块数据指针
   }
   return NET_ERR_OK; // 返回成功
}


net_err_t  pktbuf_seek(pktbuf_t *buf, int offset)
{
    dbg_assert(buf->ref !=0, "buf ref is 0\n"); // 断言数据包引用计数不为0
    if(buf->pos == offset){
        return NET_ERR_OK; // 如果偏移量相等，直接返回成功
    }
    if(offset >= buf->total_size || offset < 0){
        dbg_error(DBG_BUF, "seek out of range (%d)", offset); // 打印错误信息
        return NET_ERR_SIZE; // 返回参数错误
    }
    int move_bytes;
    if(offset < buf->pos){
        buf->curr_blk = pktblk_first_blk(buf); // 获取数据包链表中的第一个数据块
        buf->offset_blk = buf->curr_blk->data; // 设置数据块数据指针
        buf->pos = 0; // 更新数据包偏移量
        move_bytes = offset; // 设置移动的字节数
    }else{
        buf->curr_blk = pktblk_first_blk(buf); // 获取数据包链表中的第一个数据块
        buf->offset_blk = buf->curr_blk->data; // 设置数据块数据指针
        buf->pos = 0; // 更新数据包偏移量
        move_bytes = offset - buf->pos; // 计算移动的字节数
    }

    while(move_bytes){
        int remain_size = curr_remain_size(buf); // 计算数据块的大小
        int curr_move = (move_bytes > remain_size) ? remain_size : move_bytes; // 计算当前移动的字节数
        move_forward(buf, curr_move); // 更新数据包偏移量和数据块数据指针
        move_bytes -= curr_move; // 更新剩余移动的字节数

    }

    return NET_ERR_OK; // 返回成功
}


net_err_t  pktbuf_copy(pktbuf_t *dest, pktbuf_t *src, int size)
{
    dbg_assert(dest->ref !=0, "dest ref is 0\n"); // 断言数据包引用计数不为0
    dbg_assert(src->ref !=0, "dest ref is 0\n"); // 断言数据包引用计数不为0
    if(total_blk_remain(dest) < size || total_blk_remain(src) < size){
        dbg_error(DBG_BUF, "no buffer for copy (%d)", size); // 打印错误信息
        return NET_ERR_SIZE; // 返回参数错误
    }
    while(size){
        int dest_remain = curr_remain_size(dest); // 计算目标数据块的大小
        int src_remain = curr_remain_size(src); // 计算源数据块的大小
        int copy_size = (dest_remain > src_remain) ? src_remain : dest_remain; // 计算复制的大小

        copy_size = (copy_size > size) ? size : copy_size; // 计算实际复制的大小
        plat_memcpy(dest->offset_blk, src->offset_blk, copy_size); // 将数据复制到目标数据块中
        move_forward(dest, copy_size); // 更新目标数据包偏移量和数据块数据指针
        move_forward(src, copy_size); // 更新源数据包偏移量和数据块数据指针
        size -= copy_size; // 更新剩余大小
    }
    return NET_ERR_OK; // 返回成功
}

net_err_t  pktbuf_fill(pktbuf_t *buf, uint8_t v, int size){
    if(!size){
        return NET_ERR_PARAM; // 返回参数错误
   }
   int remain_size = total_blk_remain(buf); // 计算剩余大小
   if(remain_size < size){
        dbg_error(DBG_BUF, "no buffer for write (%d)", size); // 打印错误信息
        return NET_ERR_SIZE; // 返回参数错误
   }
   while(size){
    int blk_size = curr_remain_size(buf); // 计算数据块的大小
    int curr_fill = (size > blk_size) ? blk_size : size; // 计算复制的大小
    plat_memset(buf->offset_blk, v, curr_fill); // 将数据复制到数据块中
    // plat_memcpy(buf->offset_blk, src, copy_size); // 将数据复制到数据块中
 
    size -= curr_fill; // 更新剩余大小
    move_forward(buf, curr_fill); // 更新数据包偏移量和数据块数据指针
   }
   return NET_ERR_OK; // 返回成功
}