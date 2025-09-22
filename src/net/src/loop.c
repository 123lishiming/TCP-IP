#include "loop.h"
#include "netif.h"
#include "ipaddr.h"
#include "exmsg.h"
static net_err_t loop_open(struct _netif_t *netif, void *data) {
    netif->type = NETIF_TYPE_LOOP;
    return NET_ERR_OK;
}

static void loop_close(struct _netif_t *netif) {
    // 关闭操作
}

static net_err_t loop_xmit(struct _netif_t *netif) {
    pktbuf_t *pktbuf = netif_get_out(netif, -1);
    if(pktbuf){
        net_err_t err = netif_put_in(netif, pktbuf, -1);
        if(err < 0){
            pktbuf_free(pktbuf); // 释放数据包
            return err; // 返回错误
        }d

       
    }
    return NET_ERR_OK;
}

static const netif_ops_t loop_ops = {
    .open = loop_open,
    .close = loop_close,
    .xmit = loop_xmit
};

net_err_t loop_init(void) {
    dbg_info(DBG_INIT, "loop_init\n");

    netif_t *netif = netif_open("loop", &loop_ops, (void*)0);
    if (!netif) {
        dbg_error(DBG_INIT, "open loop err");
        return NET_ERR_NONE;
    }
    // 172.0.0.1 每一个是4个字节，点分十进制，ipv4/ipv6
    ipaddr_t ip, mask;
    ipaddr_from_str(&ip, "127.0.0.1");
    ipaddr_from_str(&mask, "255.0.0.0");


    netif_set_addr(netif, &ip, &mask, (ipaddr_t *)0);
    netif_set_active(netif);
    pktbuf_t *pktbuf = pktbuf_alloc(100);
    netif_out(netif, (ipaddr_t *)0, pktbuf); // 发送数据包
    dbg_info(DBG_INIT, "init done\n");
    return NET_ERR_OK;
}