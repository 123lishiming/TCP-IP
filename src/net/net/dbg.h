#ifndef __DBG_H__
#define __DBG_H__
#include "net_cfg.h"


/*
作用:
提供调试日志功能，支持不同级别的日志输出（INFO、WARNING、ERROR）。
提供断言功能 dbg_assert，用于在条件不满足时输出错误信息并停止程序。
关键功能:
调试日志输出宏：dbg_info、dbg_warning、dbg_error。
调试断言宏：dbg_assert。
*/
// Debug information colors
#define DBG_STYLE_RESET             "\033[0m"   // Reset color
#define DBG_STYLE_ERROR               "\033[31m"  // Red
#define DBG_STYLE_WARNING             "\033[33m"  // Yellow

// Debug information levels
#define DBG_LEVEL_NONE               0
#define DBG_LEVEL_WARNING            1
#define DBG_LEVEL_ERROR              2
#define DBG_LEVEL_INFO               3

void dbg_print(int m_level, int s_level, const char* file, const char* func, int line, const char* fmt, ...);

#define dbg_info(moudle, fmt, ...) \
    dbg_print(moudle, DBG_LEVEL_INFO, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define dbg_warning(moudle, fmt, ...) \
    dbg_print(moudle, DBG_LEVEL_WARNING, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

#define dbg_error(moudle, fmt, ...) \
    dbg_print(moudle, DBG_LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

// dbg_assert(3 == 0, "error");
// 3==0, error

// 使用 \ 的写法（多行）
#define dbg_assert(expr, msg) { \
    if (!(expr)) { \
        dbg_print(DBG_LEVEL_ERROR, DBG_LEVEL_ERROR, __FILE__, __FUNCTION__, __LINE__, "assert failed: " #expr ", " msg); \
        while(1); \
    } \
}

#define DBG_DISP_ENABLED(moudle)  (moudle >= DBG_LEVEL_INFO) // 判断是否启用调试信息

#endif // DBG_H