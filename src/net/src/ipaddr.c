#include "ipaddr.h"

void ipaddr_set_any(ipaddr_t *ip)
{
    ip->type = IPADDR_V4; // 设置地址类型为IPv4
    ip->q_addr = 0; // 设置IP地址为
}



net_err_t ipaddr_from_str(ipaddr_t *dest, const char *str)
{
    if(!dest || !str){
        return NET_ERR_PARAM;
    }
    dest->type = IPADDR_V4; // 设置地址类型为IPv4
    dest->q_addr = 0; // 初始化IP地址为0
    //192.168.245.1
    //"192" -> 192 -> dest -> a_addr[0]
    uint8_t *p = dest -> addr;
    char c;
    uint8_t sub_addr = 0;
    while((c = *str++) != '\0'){
        // '1' -> '9'
        if(c >= '0' && c <= '9'){
            sub_addr = sub_addr * 10 + (c - '0'); // 将字符转换为数字
        }else if(c == '.'){
            *p++ = sub_addr; // 将子地址赋值给IP地址
            sub_addr = 0;
        }else{
            return NET_ERR_PARAM; // 返回参数错误
        }
    }
    *p = sub_addr; // 处理最后一个子地址

}
// net_err_t ipaddr_to_str(ipaddr_t *ip, char *str, int len) // 将IP地址转换为字符串
// {

// }


void ipaddr_copy(ipaddr_t *dest, const ipaddr_t *src) // 复制IP地址     
{
    if(!dest || !src){
        return;
    }
    dest->type = src->type; // 复制地址类型
    dest->q_addr = src->q_addr; // 复制IP地址
}
ipaddr_t *ipaddr_get_any(void) // 获取任意地址
{
    static const ipaddr_t ipaddr_any = {.type = IPADDR_V4, .q_addr = 0}; // 定义任意地址
    return (ipaddr_t *)&ipaddr_any; // 返回任意地址
}