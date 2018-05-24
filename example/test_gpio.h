// 测试gpio接口的测试用例的头文件

#ifndef __OPENLOOGNSON_TEST_GPIO_H
#define __OPENLOOGNSON_TEST_GPIO_H



/*
 * 测试库中gpio作为输出时的相关接口
 * led闪烁10次
 */
void test_gpio_output(void);



/*
 * 测试库中gpio作为输入时的相关接口
 * 按键按下时，指示灯点亮，否则，熄灭
 */
void test_gpio_input(void);


/*
 * 测试库中外部中断(gpio输入中断)的相关接口
 * 按键被按下后，会产生一个中断
 */
void test_gpio_key_irq(void);



#endif

