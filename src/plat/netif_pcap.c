#include "netif_pcap.h"
#include "exmsg.h"
#include "sys_plat.h"
//创建两个线程，一个用于接收数据，一个用于发送数据
static void netif_pcap_recv_thread(void *arg)
{
    printf("pcap recv thread running\n");
    while(1) {
        sys_sleep(1);
        exmsg_netif_in();
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

net_err_t netif_pcap_open(void)
{
    sys_thread_create(netif_pcap_recv_thread, (void *)0);
    sys_thread_create(netif_pcap_send_thread, (void *)0);

}