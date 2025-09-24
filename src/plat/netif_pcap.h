#ifndef __NETIF_PCAP_H__
#define __NETIF_PCAP_H__
#include "net_err.h"
#include "stdint.h"
#include "netif.h"
typedef struct _pcap_data_t{
    const char *ip;
    const uint8_t *hwaddr;
}pcap_data_t;
extern const netif_ops_t netdev_ops;
#endif