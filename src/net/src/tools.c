#include "tools.h"

// 判断大小端模式
static is_little_endian(void)
{
    uint16_t v = 0x1234;
    return *(uint8_t *)&v == 0x34; // 小端模式返回1，大端模式返回0
}
net_err_t tools_init(void)
{
    dbg_info(DGB_TOOLS, "init tools");
    if(is_little_endian() !=NET_ENDIAN_LITTLE){
        dbg_error(DGB_TOOLS, "check endian failed");
        return NET_ERR_SYS;
    }
    dbg_info( DGB_TOOLS, "tools init done");
    return NET_ERR_OK;
}