// 测试硬件pwm的测试用例源文件

#include "../lib/ls1c_public.h"
#include "../lib/ls1c_gpio.h"
#include "../lib/ls1c_delay.h"
#include "../lib/ls1c_pwm.h"


// 测试硬件pwm产生连续的pwm波形
void test_pwm_normal(void)
{
    pwm_info_t pwm_info;

    pwm_info.gpio = LS1C_PWM0_GPIO06;               // pwm引脚位gpio06
//    pwm_info.gpio = LS1C_PWM0_GPIO04;
    pwm_info.mode = PWM_MODE_NORMAL;                // 正常模式--连续输出pwm波形
    pwm_info.duty = 0.25;                           // pwm占空比
    pwm_info.period_ns = 5*1000*1000;               // pwm周期5ms

    // pwm初始化，初始化后立即产生pwm波形
    pwm_init(&pwm_info);

    while (1)
    {
        // 延时100ms
        delay_ms(100);

        // 禁止pwm
        pwm_disable(&pwm_info);

        // 延时100ms
        delay_ms(100);

        // 使能pwm
        pwm_enable(&pwm_info);
    }

    return ;
}


// 测试硬件pwm产生pwm脉冲
void test_pwm_pulse(void)
{
    int i;
    pwm_info_t pwm_info;

    pwm_info.gpio = LS1C_PWM0_GPIO06;               // 输出pwm波形的引脚
    pwm_info.mode = PWM_MODE_PULSE;                 // 单脉冲模式，每次调用只发送一个脉冲，调用间隔必须大于pwm周期
    pwm_info.duty = 0.25;                           // pwm占空比
    pwm_info.period_ns = 1*1000*1000;               // pwm周期1ms

    // 为了便于用示波器观察，这里选择每个1s就发送10个脉冲
    while (1)
    {
        // 发送10个脉冲
        for (i=0; i<10; i++)
        {
            pwm_init(&pwm_info);
            delay_ms(2);
        }

        // 延时10ms
        delay_ms(10);
    }

    return ;
}


/*
 * 测试gpio04复用为pwm,gpio06作为普通gpio使用
 * PWM0的默认引脚位GPIO06，但也可以复用为GPIO04
 * 当gpio06还是保持默认为pwm时，复用gpio04为pwm0，那么会同时在两个引脚输出相同的pwm波形
 * 本函数旨在证明可以在gpio04复用为pwm0时，还可以将(默认作为pwm0的)gpio06作为普通gpio使用
 */
void test_pwm_gpio04_gpio06(void)
{
    pwm_info_t pwm_info;
    unsigned int gpio = 6;

    // 在gpio04引脚输出pwm波形
    pwm_info.gpio = LS1C_PWM0_GPIO04;           // gpio04引脚作为pwm使用
    pwm_info.mode = PWM_MODE_NORMAL;            // 输出连续的pwm波形
    pwm_info.duty = 0.25;                       // 占空比0.25
    pwm_info.period_ns = 1*1000*1000;           // pwm周期1ms
    pwm_init(&pwm_info);

    // gpio06引脚作为普通gpio使用
    gpio_init(gpio, gpio_mode_output);
    gpio_set(gpio, gpio_level_low);

    while (1)
        ;

    return ;
}


// 测试pwm最大周期
void test_pwm_max_period(void)
{
    pwm_info_t pwm_info;

    // 在gpio04引脚输出pwm波形
    pwm_info.gpio = LS1C_PWM0_GPIO04;           // gpio04引脚作为pwm使用
    pwm_info.mode = PWM_MODE_NORMAL;            // 输出连续的pwm波形
    pwm_info.duty = 0.25;                       // 占空比0.25
    pwm_info.period_ns = 130*1000*1000;         // pwm周期130ms

    // pwm周期超过最大值时会出现浮点异常，暂不知道为什么
//    pwm_info.period_ns = 134*1000*1000;         // pwm周期134ms，超过了最大值，会被自动设为最大值

    pwm_init(&pwm_info);

    while (1)
        ;
}




