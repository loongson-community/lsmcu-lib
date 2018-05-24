// 测试硬件定时器的头文件

#ifndef __OPENLOONGSON_TEST_TIMER_H
#define __OPENLOONGSON_TEST_TIMER_H




// 测试硬件定时器的定时功能(读取中断状态位的方式判断是否超时)
void test_timer_poll_time_out(void);


// 测试硬件定时器的中断
void test_timer_irq(void);



/*
 * 用硬件定时器计时
 * 用硬件定时器测量红外接收头HX1838产生的波形中高低电平的时长，
 * 来识别红外遥控板上是那个按键被按下
 */
void test_timer_get_time(void);



#endif

