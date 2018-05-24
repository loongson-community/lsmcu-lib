// 测试硬件pwm的测试用例头文件

#ifndef __OPENLOONGSON_TEST_PWM_H
#define __OPENLOONGSON_TEST_PWM_H



// 测试硬件pwm产生连续的pwm波形
void test_pwm_normal(void);


// 测试硬件pwm产生pwm脉冲
void test_pwm_pulse(void);


/*
 * 测试gpio04复用为pwm,gpio06作为普通gpio使用
 * PWM0的默认引脚位GPIO06，但也可以复用为GPIO04
 * 当gpio06还是保持默认为pwm时，复用gpio04为pwm0，那么会同时在两个引脚输出相同的pwm波形
 * 本函数旨在证明可以在gpio04复用为pwm0时，还可以将(默认作为pwm0的)gpio06作为普通gpio使用
 */
void test_pwm_gpio04_gpio06(void);


// 测试pwm最大周期
void test_pwm_max_period(void);





#endif


