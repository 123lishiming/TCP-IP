#ifndef  __NETIF_H__
#define __NETIF_H__

#include "nlist.h"
#include "fixq.h"
#include "net_cfg.h"
#include "net_err.h"
#include "ipaddr.h"
#include "pktbuf.h"
// ipv4:32bits 192.169.245.1
typedef struct _netif_hwaddr_t {
    uint8_t addr[NETIF_HWADDR_SIZE]; // 硬件地址
    uint8_t len; // 硬件地址长度
}netif_hwaddr_t;



typedef enum _netif_type_t {
    NETIF_TYPE_NONE = 0,
    NETIF_TYPE_ETHER,
    NETIF_TYPE_LOOP,

    NET_TYPE_SIZE,
}netif_type_t; 


struct _netif_t; // 前向声明


// 网络接口操作函数指针结构体
typedef struct _netif_ops_t{
    net_err_t (*open) (struct _netif_t *netif, void *data); //给底层的网络驱动,打开
    void (*close) (struct _netif_t *netif); // 关闭网络接口
    net_err_t (*xmit) (struct _netif_t *netif); //发送数据
}netif_ops_t;




// 网络接口名称最大长度
typedef struct _netif_t {
    char name[NETIF_NAME_MAX]; // 网络接口名称
    netif_hwaddr_t hwaddr; // 硬件地址
    ipaddr_t ipaddr; // IP地址
    ipaddr_t netmask; // 子网掩码
    ipaddr_t gateway; // 网关地址
    netif_type_t type; // 网络接口类型
    int mtu; // 最大传输单元

    enum{
        NETIF_CLOSED,
        NETIF_OPENED,
        NETIF_ACTIVE,
    }state;

    const netif_ops_t *ops; // 网络接口操作函数指针
    void *ops_data; // 网络接口操作函数参数


    nlist_node_t node; // 链表节点
    fixq_t in_q;
    void *in_q_buf[NETIF_INO_SIZE]; // 输入队列缓冲区
    fixq_t out_q;
    void *out_q_buf[NETIF_OUT_SIZE]; // 输出队列缓冲区

}netif_t;

net_err_t netif_init(void);
netif_t *netif_open(const char *dev_name, const netif_ops_t *ops, void * ops_data);
net_err_t netif_set_addr(netif_t *netif, ipaddr_t *ip, ipaddr_t *mask, ipaddr_t *gateway);
net_err_t netif_set_hwaddr(netif_t *netif, const char *hwaddr, int len);
net_err_t netif_set_active(netif_t *netif);
net_err_t netif_set_deactive(netif_t *netif);
net_err_t netif_close(netif_t *netif);
void netif_set_default(netif_t *netif);
net_err_t netif_put_in(netif_t *netif, pktbuf_t *pktbuf, int tmo);
pktbuf_t *netif_get_in(netif_t *netif, int tmo);
net_err_t netif_put_out(netif_t *netif, pktbuf_t *pktbuf, int tmo);
pktbuf_t *netif_get_out(netif_t *netif, int tmo);

net_err_t netif_out(netif_t *netif, ipaddr_t *ipaddr, pktbuf_t *buf); // 发送数据包到指定网络接口

#endif // ! __NETIF_H__