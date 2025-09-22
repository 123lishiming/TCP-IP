#ifndef  __IPADDR_H__
#define  __IPADDR_H__

#include "stdint.h"
#include "net_err.h"
#define IPV4_ADDR_SIZE  4

typedef struct _ipaddr_t {
    enum {
        IPADDR_V4 // IPv4地址
    } type; // 地址类型
 
    union {
        uint32_t q_addr; // 32位整数表示的IP地址
        uint8_t addr[IPV4_ADDR_SIZE]; // 4个字节表示的IP地址
    };
} ipaddr_t;
void ipaddr_set_any(ipaddr_t *ip); // 设置为任意地址
net_err_t ipaddr_from_str(ipaddr_t *dest, const char *str);
// net_err_t ipaddr_to_str(ipaddr_t *ip, char *str, int len); // 将IP地址转换为字符串
void ipaddr_copy(ipaddr_t *dest, const ipaddr_t *src); // 复制IP地址     
ipaddr_t *ipaddr_get_any(void); // 获取任意地址
#endif // ! __IPADDR_H__

