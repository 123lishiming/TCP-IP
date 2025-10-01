#ifndef __TOOLS_H__
#define __TOOLS_H__
#include <stdint.h>
#include "net_cfg.h"
#include "net_err.h"
#include "dbg.h"
static inline uint16_t swap_u16(uint16_t v){
    uint16_t r = ((v &0xFF) << 8) | ((v >> 8) &0xFF);
    return r;
}

static inline uint32_t swap_u32(uint32_t v){
    uint32_t r = 
    (((v >> 0) & 0xFF) << 24)
    | (((v >> 8) & 0xFF) << 16)
    | (((v >> 16) & 0xFF) << 8)
    | (((v >> 24) & 0xFF) << 0);
    return r;

}
// 大小端转换,这是针对小端模式的
#if NET_ENDIAN_LITTLE
#define x_htons(v)   swap_u16(v)
#define x_ntohs(v)   swap_u16(v)
#define x_htonl(v)   swap_u32(v)
#define x_ntohl(v)   swap_u32(v)
#else
#define x_htons(v)
#define x_ntohs(v)
#define x_htonl(v)
#define x_ntohl(v)
#endif
/*
h host
n net
s short 16
l 32

htons  host -> net 16
htonl  host -> net 32
*/
net_err_t tools_init(void);
#endif