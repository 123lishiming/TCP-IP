#ifndef __ARP_H__
#define __ARP_H__

#include "ipaddr.h"
#include "ether.h"
#include "pktbuf.h"
#include "netif.h"
#include "net_err.h"

#define ARP_HW_ETHER 1   // 以太网硬件类型
#define ARP_REQUEST  1   // ARP请求操作码
#define ARP_REPLY    2   // ARP应答操作码


#pragma pack(1)
typedef struct _arp_pkt_t{
    uint16_t htype;  //硬件类型
    uint16_t ptype; //协议类型
    uint8_t  hwlen; //硬件地址长度
    uint8_t  plen;
    uint16_t opcode; //操作码
    uint8_t sender_hwaddr[ETHER_HWA_SIZE]; //发送者硬件地址
    uint8_t sender_paddr[IPV4_ADDR_SIZE];  //发送者协议地址
    uint8_t target_hwaddr[ETHER_HWA_SIZE]; //目标硬件地址
    uint8_t target_paddr[IPV4_ADDR_SIZE];  //目标协议地址
}arp_pkt_t;
#pragma pack()



typedef struct _arp_entry_t{
    uint8_t paddr[IPV4_ADDR_SIZE];  //协议地址
    uint8_t hwaddr[ETHER_HWA_SIZE]; //硬件地址
    nlist_node_t node;   //链表节点
    nlist_t buf_list;  //等待该ARP请求的包链表
    enum{
        NET_ARP_FREE,
        NET_ARP_WAIT,
        NET_ARP_RESOLVED,
    }state;
    netif_t *netif;  //得到数据包,应该通过这块网卡发送出去
}arp_entry_t;


net_err_t arp_init(void);  // arp初始化
net_err_t arp_make_request(netif_t *netif, const ipaddr_t *dest);  //arp发送请求
net_err_t arp_make_gratuitious(netif_t *netif);                    //发送免费arp请求
net_err_t arp_in(netif_t *netif, pktbuf_t *buf);                   // arp输入处理

#endif 
