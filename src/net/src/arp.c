#include "arp.h"
#include "mblock.h"
#include "sys_plat.h"
#include "tools.h"
#include "protocol.h"
static arp_entry_t cache_tbl[ARP_CACHE_SIZE];
static mblock_t cache_block;
static nlist_t cache_list;

/**
 * @brief 
 * 
 * @return net_err_t 
 */
static net_err_t cache_init(void)
{
    nlist_init(&cache_list);
    plat_memset(cache_tbl, 0, ARP_CACHE_SIZE * sizeof(arp_entry_t));
    net_err_t err = mblock_init(&cache_block, cache_tbl, sizeof(arp_entry_t), ARP_CACHE_SIZE, NLOCKER_NONE);
    if(err < 0){
        return err;
    }
    return NET_ERR_OK;

}


net_err_t arp_init(void)
{
    net_err_t err = cache_init();
    if(err < 0){
        dbg_info(DBG_ARP, "arp cache init failed\n");
        return err;
    }
    return NET_ERR_OK;
}




/**
 * @brief 发送arp请求
 * 
 * @param netif  网卡
 * @param dest  目标IP地址
 * @return net_err_t  错误码
 * @note 实现arp请求包的构建和发送
 * @warning  注意网络字节序的转换
 */
net_err_t arp_make_request(netif_t *netif, const ipaddr_t *dest)
{
    pktbuf_t *buf = pktbuf_alloc(sizeof(arp_pkt_t));
    if(buf ==(pktbuf_t *)0){
        dbg_error(DBG_ARP, "alloc failed\n");
    }

    pktbuf_set_cont(buf, sizeof(arp_pkt_t));
    arp_pkt_t *arp_packet = (arp_pkt_t *)pktbuf_data(buf);

    arp_packet->htype     = x_htons(ARP_HW_ETHER);
    arp_packet->ptype     = x_htons(NET_PROTOCOL_IPv4);
    arp_packet->hwlen     = ETHER_HWA_SIZE;
    arp_packet->plen      = IPV4_ADDR_SIZE;
    arp_packet->opcode    = x_htons(ARP_REQUEST);
    plat_memcpy(arp_packet->sender_hwaddr, netif->hwaddr.addr, ETHER_HWA_SIZE);
    ipaddr_to_buf(&netif->ipaddr,arp_packet->sender_paddr);
    plat_memset(arp_packet->target_hwaddr, 0, ETHER_HWA_SIZE);
    // plat_memcpy(arp_packet->target_hwaddr, netif->hwaddr.addr, ETHER_HWA_SIZE);
    ipaddr_to_buf(dest,arp_packet->target_paddr);

    net_err_t err = ether_raw_out(netif,NET_PROTOCOL_ARP,ether_board_cast_addr(), buf);

    // 如果下层出错，由下层去释放
    if(err < 0){
        pktbuf_free(buf);
    }
    return err;

}