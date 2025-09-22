#include "dbg.h"
#include "sys_plat.h"
#include <stdarg.h>
// d:/xxx/xxx/main.c
void dbg_print(int m_level, int s_level, const char *file, const char *func, int line, const char *fmt, ...) {
    static const char * title[] ={
        [DBG_LEVEL_NONE] = "none",
        [DBG_LEVEL_WARNING] = DBG_STYLE_WARNING"waring",
        [DBG_LEVEL_ERROR] = DBG_STYLE_ERROR"error",
        [DBG_LEVEL_INFO] = "info",
    };
    if (m_level >= m_level) {
    const char *end = file + plat_strlen(file);
    while(end >= file){
        if(*end == '/' || *end == '\\'){
            break;
        }
        end--;
    }
    end++;

    plat_printf("%s(%s-%s-%d): ", title[s_level],end, func, line );

    char buf[256];
    va_list args;
    va_start(args, fmt);
    plat_vsprintf(buf, fmt, args);
    plat_printf("%s\n"DBG_STYLE_RESET, buf);
    va_end(args);
    }
}


void dbg_dump_hwaddr(const char *msg, const uint8_t *hwaddr, int len)
{
    if(msg){
        plat_printf("%s", msg);
    }
    if(len){
    for(int i = 0 ; i < len; i++){
        plat_printf("%02x-", hwaddr[i]); // 打印硬件地址)
        }
    }else{
        plat_printf("none ");
    }
}
void dbg_dump_ip(const char *msg, const ipaddr_t *ipaddr)
{
    if(msg){
        plat_printf("%s", msg);
    }
    if(ipaddr){
        for(int i = 0 ; i < IPV4_ADDR_SIZE; i++){
            if(i == IPV4_ADDR_SIZE - 1){
                plat_printf("%d ", ipaddr->addr[i]); // 打印IP地址
                break;
            }
             plat_printf("%d.", ipaddr->addr[i]); // 打印IP地址
        }
        
    }else{
        plat_printf("0.0.0.0");
    }

}