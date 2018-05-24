

#include <stdio.h>
#include <stdarg.h>
#include "../lib/ls1c_uart.h"


#define PRINTF_BUF_SIZE                 (512)

int printf (const char *fmt, ...)
{
    int     len;
    va_list ap;
    char    buf[PRINTF_BUF_SIZE];

    va_start(ap, fmt);

    // 格式化字符串
    len = vsprintf (buf, fmt, ap);

    // 调用龙芯1c库中的串口函数打印字符串
    uart_debug_print(buf);
    
    va_end(ap);
    
    return (len);
}
