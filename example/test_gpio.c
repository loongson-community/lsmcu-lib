// 测试gpio接口的测试用例源文件


#include "../lib/ls1c_public.h"
#include "../lib/ls1c_gpio.h"
#include "../lib/ls1c_delay.h"
#include "../lib/ls1c_irq.h"
#include "led.h"


// 按键中断的标志
volatile int gpio_key_irq_flag = 0;


/*
 * 测试库中gpio作为输出时的相关接口
 * led闪烁10次
 */
void test_gpio_output(void)
{
    int i;
    unsigned int gpio = 32;     // 智龙首发版、v2.0和v2.1都有这个led
//    unsigned int gpio = 6;
//    unsigned int gpio = 74;

    // 初始化
    gpio_init(gpio, gpio_mode_output);

    // 输出10个矩形波，如果gpio50上有led，则可以看见led闪烁10次
    for (i=0; i<10; i++)
    {
        gpio_set(gpio, gpio_level_low);
        delay_s(1);
        gpio_set(gpio, gpio_level_high);
        delay_s(1);
    }
    
    return ;
}



/*
 * 测试库中gpio作为输入时的相关接口
 * 按键按下时，指示灯点亮，否则，熄灭
 */
void test_gpio_input(void)
{
    unsigned int key_gpio = 85;     // GPIO85/I2C_SDA0，智龙v2.1上的按键S3
    unsigned int led_gpio = 32;     // 用于当按键被按下时，此led点亮，否则熄灭

    // 初始化
    gpio_init(key_gpio, gpio_mode_input);
    led_init(led_gpio);

    while (1)
    {
        if (gpio_level_low != gpio_get(key_gpio))
            continue;       // 按键没有按下

        // 延时(软件消抖)后再次确认按键是否按下
        delay_ms(10);
        if (gpio_level_low != gpio_get(key_gpio))
            continue;       // 按键没有按下

        // 点亮指示灯
        led_on(led_gpio);

        // 等待释放按键
        while (gpio_level_high != gpio_get(key_gpio))
            ;
        delay_ms(10);

        // 熄灭指示灯
        led_off(led_gpio);
    }
}


/*
 * 按键中断的处理函数
 * @IRQn 中断号
 * @param 传递给中断处理函数的参数
 */
void test_gpio_key_irqhandler(int IRQn, void *param)
{
    gpio_key_irq_flag = 1;
}

/*
 * 测试库中外部中断(gpio中断，按键中断)的相关接口
 * 按键被按下后，会产生一个中断
 */
void test_gpio_key_irq(void)
{
    unsigned int key_gpio = 85;     // GPIO85/I2C_SDA0
    int key_irq = LS1C_GPIO_TO_IRQ(key_gpio);
    static int count = 0;

    // 初始化按键中断
    gpio_init(key_gpio, gpio_mode_input);
    gpio_set_irq_type(key_gpio, IRQ_TYPE_LEVEL_LOW);
//    gpio_set_irq_type(key_gpio, IRQ_TYPE_EDGE_FALLING);     // 当设为下降沿触发时，按下和弹起都会产生一次中断，理论上不应该这样，暂不知原因，改为低电平触发一切正常
    irq_install(key_irq, test_gpio_key_irqhandler, NULL);
    irq_enable(key_irq);

    while (1)
    {
        if (1 == gpio_key_irq_flag)
        {
            delay_ms(10);               // 延时10ms，消抖
            gpio_key_irq_flag = 0;      // 清标志
            count++;                    // 计数器加一
            printf("[%s] count=%d, key press\r\n", __FUNCTION__, count);
        }
    }
}


