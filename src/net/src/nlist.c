#include "nlist.h"

void nlist_init(nlist_t *list){
    list->first = list->last = (nlist_node_t *)0;
    list->count = 0;
}

void nlist_insert_first(nlist_t *list, nlist_node_t *node){
    node->prev = (nlist_node_t *)0;
    node->next = list->first;  //list->first == 0;
    if(nlist_is_empty(list)){
        list->last = list->first = node;
    }else{
        list->first->prev = node;
        list->first = node;
    }
    list->count++;

}


nlist_node_t *nlist_remove(nlist_t *list, nlist_node_t *node){
    if(node == list->first){
        list->first = node->next;
    }
    if(node == list->last){
        list->last = node->prev;
    }
    if(node->prev){
        node->prev->next = node->next;
    }
    if(node->next){
        node->next->prev = node->prev;
    }

    node->prev = node->next = (nlist_node_t *)0;
    list->count--;
    return node;

}

void nlist_insert_last(nlist_t *list, nlist_node_t *node){
    node->next = (nlist_node_t *)0;
    node->prev = list->last;  //list->last == 0;
    if(nlist_is_empty(list)){
        list->first = list->last = node;
    }else{
        list->last->next = node;
        list->last = node;
    }
    list->count++;
}

/**
 * @brief 在链表中某个节点之后插入一个新节点
 * 
 * @param list 指向链表的指针
 * @param pre 指向链表中目标节点的指针，新节点将插入到该节点之后
 * @param node 指向要插入的新节点的指针
 * 
 * @note 如果链表为空或 `pre` 为 NULL，则直接将新节点插入到链表中（调用 `nlist_insert`）。
 */
void nlist_insert_after(nlist_t *list, nlist_node_t *pre, nlist_node_t *node) {
    // 如果链表为空或 pre 节点为 NULL，则直接插入节点
    if (nlist_is_empty(list) || !pre) {
        nlist_insert_first(list, node); // 调用链表插入函数
        return;
    }

    // 设置新节点的 next 指针为 pre 节点的 next 指针
    node->next = pre->next;

    // 设置新节点的 prev 指针为 pre 节点
    node->prev = pre;

    // 如果 pre 节点的 next 不为空，则更新 pre->next 的前驱指针为新节点
    if (pre->next) {
        pre->next->prev = node;
    }

    // 更新 pre 节点的 next 指针为新节点
    pre->next = node;

    if(list->last == pre) {
        list->last = node; // 更新链表的最后一个节点
    }
    list->count++; // 更新链表节点计数
}