#ifndef __DBG_H
#define __DBG_H

#include <stdio.h>

// 调试功能开关
#ifndef DBG_ENABLE
#define DBG_ENABLE 1  // 默认启用调试
#endif

// 调试打印宏
#if DBG_ENABLE
#define DBG_PRINTF(...) do { printf(__VA_ARGS__); } while(0)  // 启用调试时打印信息
#else
#define DBG_PRINTF(...) do { } while(0)  // 禁用调试时不打印信息
#endif

#endif
