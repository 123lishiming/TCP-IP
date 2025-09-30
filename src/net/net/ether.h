#ifndef __ETHER_H__
#define __ETHER_H__
#include "net_err.h"
#include <stdint.h>

#define ETHER_HWA_SIZE 6
#define ETHER_MIU 1500
#pragma pack(1)
// 定义包头
typedef struct _ether_hdr_t {
    uint8_t dest[ETHER_HWA_SIZE];
    uint8_t src[ETHER_HWA_SIZE];
    uint16_t protocal; //可能当作四字节处理，结构体对齐问题(此时应该禁用自动填充对齐才能解析正确)
}ether_hdr_t;

// 定义数据包类型
typedef struct _ether_pkt_t {
    ether_hdr_t hdr;
    uint8_t  data[ETHER_MIU];
}ether_pkt_t;
#pragma pack()
net_err_t ether_init();
#endif