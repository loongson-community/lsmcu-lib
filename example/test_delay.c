// 测试延时函数的源码


#include "../lib/ls1c_gpio.h"
#include "../lib/ls1c_delay.h"


// 测试延时函数delay_1ms()
void test_delay_1ms(void)
{
    unsigned int gpio = 6;
    int time = 0;

    gpio_init(gpio, gpio_mode_output);
    gpio_set(gpio, gpio_level_high);

    // 产生不同宽度的高低电平，用示波器观察高低电平宽度是否正确
    while (1)
    {
        // 2ms
        time = 2/2;
        delay_ms(time);
        gpio_set(gpio, gpio_level_low);
        delay_ms(time);
        gpio_set(gpio, gpio_level_high);

        // 10ms
        time = 10/2;
        delay_ms(time);
        gpio_set(gpio, gpio_level_low);
        delay_ms(time);
        gpio_set(gpio, gpio_level_high);

        // 100ms
        time = 100/2;
        delay_ms(time);
        gpio_set(gpio, gpio_level_low);
        delay_ms(time);
        gpio_set(gpio, gpio_level_high);

        // 500ms
        time = 500/2;
        delay_ms(time);
        gpio_set(gpio, gpio_level_low);
        delay_ms(time);
        gpio_set(gpio, gpio_level_high);        
    }
}



// 测试延时函数delay_1us()
void test_delay_1us(void)
{
    unsigned int gpio = 6;
    int time;

    gpio_init(gpio, gpio_mode_output);
    gpio_set(gpio, gpio_level_high);

    // 产生不同宽度的高低电平，用示波器观察高低电平宽度是否正确
    while (1)
    {
        // 2us
        time = 2/2;
        delay_us(time);
        gpio_set(gpio, gpio_level_low);
        delay_us(time);
        gpio_set(gpio, gpio_level_high);

        // 10us
        time = 10/2;
        delay_us(time);
        gpio_set(gpio, gpio_level_low);
        delay_us(time);
        gpio_set(gpio, gpio_level_high);

        // 50us
        time = 50/2;
        delay_us(time);
        gpio_set(gpio, gpio_level_low);
        delay_us(time);
        gpio_set(gpio, gpio_level_high);

        // 100us
        time = 100/2;
        delay_us(time);
        gpio_set(gpio, gpio_level_low);
        delay_us(time);
        gpio_set(gpio, gpio_level_high);

        // 500us
        time = 500/2;
        delay_us(time);
        gpio_set(gpio, gpio_level_low);
        delay_us(time);
        gpio_set(gpio, gpio_level_high);
    }
}


// 测试延时函数delay_1s()
void test_delay_1s(void)
{
    unsigned int gpio = 6;
    int time;

    gpio_init(gpio, gpio_mode_output);
    gpio_set(gpio, gpio_level_high);

    while (1)
    {
        // 2s
        time = 2/2;
        delay_s(time);
        gpio_set(gpio, gpio_level_low);
        delay_s(time);
        gpio_set(gpio, gpio_level_high);

        // 10s
        time = 10/2;
        delay_s(time);
        gpio_set(gpio, gpio_level_low);
        delay_s(time);
        gpio_set(gpio, gpio_level_high);
    }
}




