#ifndef __LIST_H__
#define __LIST_H__



/*
功能:
定义双向链表的接口和数据结构。
关键功能:
定义链表节点结构体 nlist_node_t 和链表结构体 nlist_t。
提供链表的初始化、插入、删除、遍历等接口：nlist_init、nlist_insert_first、nlist_remove_first、nlist_for_each。

*/
typedef struct _nlist_node_t {
    struct  _nlist_node_t *next;  // 下一个节点指针
    struct  _nlist_node_t *prev;  // 上一个节点指针
}nlist_node_t;

static inline void nodelist_init(nlist_node_t *node) {
    node->next = node->prev = (nlist_node_t *)0;
}

static inline nlist_node_t * nlist_node_next(nlist_node_t *node){
    return node->next;
}

static inline nlist_node_t * nlist_node_prev(nlist_node_t *node){
    return node->prev;
}



static inline void nlist_node_set_next(nlist_node_t *node, nlist_node_t *next){
    node->next = next;
}
typedef struct _nlist_t {
    nlist_node_t *first;
    nlist_node_t *last;
    int count;
}nlist_t;



void nlist_init(nlist_t *list);

static inline int nlist_is_empty(nlist_t *list){
    return list->count == 0;
}

static inline int nlist_count (nlist_t *list){
    return list->count;
}
static inline nlist_node_t * nlist_first(nlist_t *list){
    return list->first;
}

static inline nlist_node_t * nlist_last(nlist_t *list){
    return list->last;
}


//往链表头部添加节点
void nlist_insert_first(nlist_t *list, nlist_node_t *node);

//遍历链表
#define nlist_for_each(node, list)      for (node = (list)->first; node; node = node->next)

#define noffset_in_parent(parent_type, node_name) \
    ((char*)&(((parent_type *)0)->node_name))

#define noffset_to_parent(node, parent_type, node_name) \
    ((parent_type *)((char *)(node) - noffset_in_parent(parent_type, node_name)))

#define nlist_entry(node, parent_type, node_name) \
    ((node) ? noffset_to_parent((node), parent_type, node_name) : (parent_type*)0)

nlist_node_t *nlist_remove(nlist_t *list, nlist_node_t *node);

static inline nlist_node_t *nlist_remove_first(nlist_t *list){
    nlist_node_t *first = nlist_first(list);
    if(first){
        nlist_remove(list, first);
    }
    return first;
}


//尾部插入节点
void nlist_insert_last(nlist_t *list, nlist_node_t *node);
static inline nlist_node_t *nlist_remove_last(nlist_t *list){
    nlist_node_t *last = nlist_last(list);
    if(last){
        nlist_remove(list, last);
    }
    return last;
}
void nlist_insert_after(nlist_t *list, nlist_node_t *pre, nlist_node_t *node);

#endif // __LIST_H__