#include "ether.h"
#include "dbg.h"
#include "netif.h"

// 打开
 net_err_t ether_open(struct _netif_t *netif)
 {
    return NET_ERR_OK;

 }

 // 关闭
void ether_close(struct _netif_t *netif)
{

}

// 输入数据包
net_err_t ether_in(struct _netif_t *netif, pktbuf_t * buf)
{
    return NET_ERR_OK;
}

// 输出数据包
net_err_t ether_out(struct _netif_t *netif,ipaddr_t *dest, pktbuf_t *buf)
{
    return NET_ERR_OK;
}





// 以太网接口初始化
net_err_t ether_init(void)
{
    static const  link_layer_t link_layer = {
    .type  = NETIF_TYPE_ETHER,
    .open  = ether_open,
    .close = ether_close,
    .in    = ether_in,
    .out   = ether_out,
    };
    dbg_info(DBG_ETHER, "ether init doing");
    net_err_t  err = netif_register_layer(NETIF_TYPE_ETHER, &link_layer);
    if(err < 0){
        dbg_error(DBG_ETHER, "register err");
        return err;
    }
    dbg_info(DBG_ETHER, "link_layer init done");
    return NET_ERR_OK;
}