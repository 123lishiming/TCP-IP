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


/**
 * @brief  //无回报的ip请求
 * 
 * @param netif  网卡
 * @return net_err_t  错误码
 * @note 实现arp请求包的构建和发送
 */
net_err_t arp_make_gratuitious(netif_t *netif)
{
    dbg_error(DBG_ARP, "send an gratuitious arp...");

    return arp_make_request(netif, &netif->ipaddr);
}



/**
 * @brief 
 * 包的合法性判断
 * @param arp_packet 
 * @param size 
 * @param netif 
 * @return net_err_t 
 */

static net_err_t is_pkt_ok(arp_pkt_t *arp_packet, int size, netif_t *netif)
{
    // sizeof(arp_packet) < size
    if(sizeof(arp_packet) < size){
        dbg_error(DBG_ARP, "packet size error\n");
        return NET_ERR_SIZE;
    }

    // 详细检查，符合IPv4,大小端转换
    if((x_ntohs(arp_packet->htype) != ARP_HW_ETHER)   
    || (x_ntohs(arp_packet->hwlen) != ETHER_HWA_SIZE)
    || (x_htons(arp_packet->ptype) != NET_PROTOCOL_IPv4)
    || (arp_packet->plen           != IPV4_ADDR_SIZE))
    {
        dbg_warning(DBG_ARP, "packet incorrect\n");
        return NET_ERR_UN_SUPPOT;
    }

    uint16_t opcode = x_ntohs(arp_packet->opcode);
    if((opcode != ARP_REPLY) && (opcode != ARP_REQUEST))
    {
        dbg_warning(DBG_ARP, "unknown opcode\n");
        return NET_ERR_UN_SUPPOT;
    }
    return NET_ERR_OK;

}




/**
 * @brief  接收arp包
 *
 * @param netif 
 * @param buf 
 * @return net_err_t 
 * @note
 * 
 */

net_err_t arp_in(netif_t *netif, pktbuf_t *buf)
{
    dbg_info(DBG_ARP, "arp in\n");
    net_err_t err = pktbuf_set_cont(buf, sizeof(arp_pkt_t));
    if(err < 0){
        dbg_error(DBG_ARP,"set_cont failed\n");
        return err;
    }
    arp_pkt_t *arp_packet = (arp_pkt_t *)pktbuf_data(buf);
    if(is_pkt_ok(arp_packet, buf->total_size, netif) != NET_ERR_OK)
    {
        return err;
    }
    pktbuf_free(buf);
    return NET_ERR_OK;
}