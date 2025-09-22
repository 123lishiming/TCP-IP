#include "netif.h"
#include "mblock.h"
#include "dbg.h"
#include "net_cfg.h"
#include "pktbuf.h"
#include "nlist.h"
#include "exmsg.h"
static netif_t netif_buffer[NETITF_DEV_CNT]; // 网络接口缓冲区
static mblock_t netif_mblock; // 网络接口内存块
static nlist_t netif_list; // 网络接口链表
static netif_t *netif_default; // 默认网络接口
#if DBG_DISP_ENABLED(DBG_NETIF)
void display_netif_list(void){
    plat_printf("netif list:\n");
    nlist_node_t *node; 
    nlist_for_each(node, &netif_list){
        netif_t *netif = nlist_entry(node, netif_t, node); // 获取网络接口

        plat_printf("%s:", netif->name); // 打印网络接口名称
        switch(netif->state){
        case NETIF_CLOSED:
            plat_printf("  %s  ", "closed");
            break;
        
        case NETIF_OPENED:
            plat_printf("  %s  ", "opened");
            break;
        
         
        case NETIF_ACTIVE:
            plat_printf("  %s  ", "active");
            break;
        default:
             break;
    }

    switch (netif->type) {

    case NETIF_TYPE_ETHER:
        plat_printf("  %s  ", "ether");
        break;
    case NETIF_TYPE_LOOP:
        plat_printf("  %s  ", "loop");
        break;
    default:
        break;
    }
    plat_printf("mtu:%d \n", netif->mtu); // 打印最大传输单元 
    dbg_dump_hwaddr("hwaddr: ", netif->hwaddr.addr, netif->hwaddr.len);
    dbg_dump_ip("ipaddr:", &netif->ipaddr); // 打印IP地址
    dbg_dump_ip("netmask:", &netif->netmask); // 打印子网掩码
    dbg_dump_ip("gateway:", &netif->gateway); // 打印网关地址
    plat_printf("\n");}
}
#else
#define display_netif_list()
#endif
net_err_t netif_init(void)
{
    dbg_info(DBG_NETIF, "netif_init\n");
    nlist_init(&netif_list); // 初始化网络接口链表
    mblock_init(&netif_mblock, netif_buffer, sizeof(netif_t), NETITF_DEV_CNT, NLOCKER_NONE); // 初始化网络接口内存块
    netif_default = (netif_t *)0; // 设置默认网络接口为NULL

    dbg_info(DBG_NETIF, "init done");
    return NET_ERR_OK;

}


netif_t *netif_open(const char *dev_name,  const netif_ops_t *ops, void * ops_data)
{
    netif_t *netif = (netif_t *)mblock_alloc(&netif_mblock, -1); // 分配网络接口内存块
    if (netif == NULL) {
        dbg_error(DBG_NETIF, "mblock alloc failed\n");
        return (netif_t *)0;
    }
    //进行一些初始化
    ipaddr_set_any(&netif->ipaddr); // 设置网卡的ip地址
    ipaddr_set_any(&netif->netmask); // 设置网卡的子网掩码
    ipaddr_set_any(&netif->gateway); // 设置网卡的网关地址
   
    plat_strncpy(netif->name, dev_name, NETIF_NAME_MAX); // 设置网卡名称
    netif->name[NETIF_NAME_MAX - 1] = '\0'; // 确保名称以'\0'结尾
  

    plat_memset(&netif->hwaddr, 0 ,sizeof(netif_hwaddr_t)); // 设置网卡的硬件地址
    netif->type = NETIF_TYPE_NONE; // 设置网卡类型,因为一开始就不知道网卡的类型
    netif->mtu = 0; // 设置网卡的最大传输单元
    nodelist_init(&netif->node); // 初始化网卡节点
    net_err_t err = fixq_init(&netif->in_q, netif->in_q_buf, NETIF_INO_SIZE, NLOCKER_THREAD);
    if(err < 0){
        dbg_error(DBG_NETIF, "netif in_q init failed\n");
        return (netif_t *)0;            
    }
   
    err = fixq_init(&netif->out_q, netif->out_q_buf, NETIF_OUT_SIZE, NLOCKER_THREAD);
    if(err < 0){
        dbg_error(DBG_NETIF, "netif out_q init failed\n");
        fixq_destroy(&netif->in_q); // 销毁输入队列
        return (netif_t *)0;            
    }

    netif->ops = ops; // 设置网卡操作函数
    netif->ops_data = ops_data; // 设置网卡操作函数参数
    err = ops->open(netif, ops_data); // 调用操作函数打开网卡
     netif->state = NETIF_OPENED; // 设置网卡状态为关闭
    if(err < 0){
        dbg_error(DBG_NETIF, "netif ops open err");
        goto free_return; // 打开网卡失败
    }
    if(netif->type == NETIF_TYPE_NONE){
        dbg_error(DBG_NETIF, "netif type is none\n");
        goto free_return; // 网卡类型为NONE
    }
    nlist_insert_last(&netif_list, &netif->node); // 将网卡添加到链表中
    display_netif_list(); // 显示网卡列表
    return netif;
free_return:
    if(netif->state == NETIF_OPENED){
        netif->ops->close(netif); // 关闭网卡
    }
    fixq_destroy(&netif->in_q); // 销毁输入队列
    fixq_destroy(&netif->out_q); // 销毁输出队列
    mblock_free(&netif_mblock, netif); // 释放网络接口内存块
    return (netif_t *)0; // 返回NULL

} 

net_err_t netif_set_addr(netif_t *netif, ipaddr_t *ip, ipaddr_t *mask, ipaddr_t *gateway)
{
    ipaddr_copy(&netif->ipaddr, ip ? ip : ipaddr_get_any()); // 设置网卡的ip地址
    ipaddr_copy(&netif->netmask, mask ? mask : ipaddr_get_any()); // 设置网卡的ip地址
    ipaddr_copy(&netif->gateway, gateway ? gateway : ipaddr_get_any()); // 设置网卡的ip地址
    return NET_ERR_OK; // 返回成功
}
net_err_t netif_set_hwaddr(netif_t *netif, const char *hwaddr, int len)
{
    plat_memcpy(netif->hwaddr.addr, hwaddr, len); // 设置网卡的硬件地址
    netif->hwaddr.len = len; // 设置网卡的硬件地址长度
    return NET_ERR_OK; // 返回成功

}


net_err_t netif_set_active(netif_t *netif)
{
    if(netif->state != NETIF_OPENED){
        dbg_error(DBG_NETIF, "netif state is not opened\n");
        return NET_ERR_STATA; // 返回参数错误
    }
    if(!netif_default && (netif->type != NETIF_TYPE_LOOP)){
        netif_set_default(netif); // 设置默认网络接口
    }
    netif->state = NETIF_ACTIVE; // 设置网卡状态为活动
    display_netif_list(); // 显示网卡列表
    return NET_ERR_OK;  //  返回成功
}

net_err_t netif_set_deactive(netif_t *netif)
{
     if(netif->state != NETIF_ACTIVE){
        dbg_error(DBG_NETIF, "netif state is not opened\n");
        return NET_ERR_STATA; // 返回参数错误
    }
    pktbuf_t *pktbuf;
    while((pktbuf = fixq_recv(&netif->in_q, -1)) != (pktbuf_t *)0){ // 从输入队列中接收数据包
        pktbuf_free(pktbuf); // 释放数据包
    }
    while((pktbuf = fixq_recv(&netif->out_q, -1)) != (pktbuf_t *)0){ // 从输入队列中接收数据包
        pktbuf_free(pktbuf); // 释放数据包
    }
    if(netif_default == netif){
        netif_default = (netif_t *)0; // 设置默认网络接口为NULL
    }
    netif->state = NETIF_OPENED; // 设置网卡状态为活动
    display_netif_list(); // 显示网卡列表
    return NET_ERR_OK;  //  返回成功


}



net_err_t netif_close(netif_t *netif)
{
    if(netif->state == NETIF_ACTIVE){
        dbg_error(DBG_NETIF, "netif is active\n");
        return NET_ERR_STATA;
    }
    netif->ops->close(netif);
    netif->state = NETIF_CLOSED; // 设置网卡状态为关闭
    nlist_remove(&netif_list, &netif->node); // 从链表中删除网卡
    mblock_free(&netif_mblock, netif); // 释放网络接口内存块
    display_netif_list();   // 显示网卡列表
    return NET_ERR_OK; // 返回成功
}
void netif_set_default(netif_t *netif)
{
    netif_default = netif; // 设置默认网络接口
}

// 发送数据包到输入队列
net_err_t netif_put_in(netif_t *netif, pktbuf_t *pktbuf, int tmo){
    net_err_t err = fixq_send(&netif->in_q, pktbuf, tmo); // 将数据包发送到输入队列
    if(err < 0){
        dbg_warning(DBG_NETIF, "netif in_q full \n");
        return NET_ERR_FULL; // 返回队列已满
    }
    exmsg_netif_in(netif); // 发送网络接口输入消息
    return NET_ERR_OK; // 返回成功
}
// 从输入队列中接收数据包
pktbuf_t *netif_get_in(netif_t *netif, int tmo){
    pktbuf_t *buf = fixq_recv(&netif->in_q, tmo); // 从输入队列中接收数据包
    if(buf){
        pktbuf_reset_acc(buf);  // 重置数据包的接收状态
        return buf; // 返回数据包
    }
    dbg_info(DBG_NETIF, "netif inq_q emptry\n");
    return (pktbuf_t *)0; // 返回NULL
}

// 发送数据包到输出队列
net_err_t netif_put_out(netif_t *netif, pktbuf_t *pktbuf, int tmo){
    net_err_t err = fixq_send(&netif->out_q, pktbuf, tmo); // 将数据包发送到输入队列
    if(err < 0){
        dbg_warning(DBG_NETIF, "netif out_q full\n");
        return NET_ERR_FULL; // 返回队列已满
    }
    return NET_ERR_OK; // 返回成功
}
// 从输出队列中接收数据包
pktbuf_t *netif_get_out(netif_t *netif, int tmo){
    pktbuf_t *buf = fixq_recv(&netif->out_q, tmo); // 从输入队列中接收数据包
    if(buf){
        pktbuf_reset_acc(buf);  // 重置数据包的接收状态
        return buf; // 返回数据包
    }
    dbg_info(DBG_NETIF, "netif out_q emptry\n");
    return (pktbuf_t *)0; // 返回NULL
}



net_err_t netif_out(netif_t *netif, ipaddr_t *ipaddr, pktbuf_t *buf)
{
    // 发送数据包到输出队列
    net_err_t err = netif_put_out(netif, buf, -1); // 将数据包发送到输出队列
    if(err < 0){
        dbg_info(DBG_NETIF, "netif out err\n");
    }
    return netif->ops->xmit(netif);  // 调用底层驱动发送数据包
}