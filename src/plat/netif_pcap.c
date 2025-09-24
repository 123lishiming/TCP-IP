#include "netif_pcap.h"
#include "exmsg.h"
#include "sys_plat.h"
#include "pcap.h"
//创建两个线程，一个用于接收数据，一个用于发送数据
static void netif_pcap_recv_thread(void *arg)
{
    printf("pcap recv thread running\n");
    while(1) {
        sys_sleep(1);
        exmsg_netif_in((netif_t *)0);
    }

}


static void netif_pcap_send_thread(void *arg)
{
    printf("pcap send thread running\n");
    while(1) {
        // 线程执行的代码
        sys_sleep(1000);  // 延时1秒
    }

}


static net_err_t netif_pcap_open(struct _netif_t *netif, void *data) {
    pcap_data_t *dev_data = (pcap_data_t *)data;
    pcap_t * pcap = pcap_device_open(dev_data->ip, dev_data->hwaddr);
    if(pcap == (pcap_t *)0){
        dbg_error(DBG_NETIF,"pcap open failed! name:%s\n",netif->name);
        return NET_ERR_IO;
    }
    netif->type = NETIF_TYPE_ETHER;
    netif->mtu  = 1500;
    netif->ops_data = pcap;
    netif_set_hwaddr(netif, dev_data->hwaddr, 6);


    sys_thread_create(netif_pcap_recv_thread, netif);
    sys_thread_create(netif_pcap_send_thread, netif);
    return NET_ERR_OK;
}

static void netif_pcap_close(struct _netif_t *netif) {
    pcap_t *pcap = (pcap_t *)netif->ops_data;
    pcap_close(pcap);
    // 关闭操作
}

static net_err_t netif_pcap_xmit(struct _netif_t *netif) {
    return NET_ERR_OK;
}

 const netif_ops_t netdev_ops = {
    .open = netif_pcap_open,
    .close = netif_pcap_close,
    .xmit = netif_pcap_xmit,
};