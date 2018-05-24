// 测试滴答定时器的源文件


#include "../lib/ls1c_public.h"
#include "../lib/ls1c_sys_tick.h"
#include "../lib/ls1c_delay.h"


// 每秒的产生的tick数
#define TICK_PER_SECOND                 (1000)


/*
 * 测试滴答定时器
 */
void test_sys_tick(void)
{
    int i;
    unsigned int tick_per_second = TICK_PER_SECOND;
    
    // 滴答定时器初始化
    // 每次滴答定时器中断就会将tick加一，这里只需要读取tick值，并打印出来即可
    sys_tick_init(tick_per_second);     // 每秒产生1000个tick    
    printf("sys tick init ok! tick = %u\r\n", tick_per_second);
    for (i=0; i<5; i++)
    {
        printf("[%s] tick=%d\r\n", __FUNCTION__, sys_tick_get());
        delay_s(1);
    }

    // 禁止滴答定时器后，看看tick值是否变化
    sys_tick_disable();
    printf("sys tick disable\r\n");
    for (i=0; i<5; i++)
    {
        printf("[%s] tick=%d\r\n", __FUNCTION__, sys_tick_get());
        delay_s(1);
    }


    // 重新使能滴答定时器后，再看看tick是否变化
    sys_tick_enable();
    printf("sys tick enable\r\n");
    for (i=0; i<5; i++)
    {
        printf("[%s] tick=%d\r\n", __FUNCTION__, sys_tick_get());
        delay_s(1);
    }


    // 修改tick
    tick_per_second = TICK_PER_SECOND/2;
    sys_tick_disable();
    sys_tick_init(tick_per_second);
    printf("modify tick = %u\r\n", tick_per_second);
    for (i=0; i<5; i++)
    {
        printf("[%s] tick=%d\r\n", __FUNCTION__, sys_tick_get());
        delay_s(1);
    }

    while (1)
        ;
}

