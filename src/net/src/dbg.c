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
