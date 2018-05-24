#include "../lib/ls1c_public.h"
#include "../lib/ls1c_irq.h"
#include "../lib/ls1c_gpio.h"
#include "../lib/ls1c_delay.h"
#include "../lib/ls1c_mipsregs.h"
#include "../lib/ls1c_uart.h"
#include "../lib/ls1c_sys_tick.h"
#include "../lib/ls1c_clock.h"
#include "../example/test_gpio.h"
#include "../example/test_pwm.h"
#include "../example/test_delay.h"
#include "../example/test_simulate_i2c.h"
#include "../example/test_timer.h"
#include "../example/test_fpu.h"
#include "../example/test_i2c.h"
#include "../example/test_uart.h"
#include "../example/test_sys_tick.h"
#include "../example/test_spi.h"




// 硬浮点初始化
void fpu_init(void)
{
    unsigned int c0_status = 0;
    unsigned int c1_status = 0;

    // 使能协处理器1--FPU
    c0_status = read_c0_status();
    c0_status |= (ST0_CU1 | ST0_FR);
    write_c0_status(c0_status);

    // 配置FPU
    c1_status = read_c1_status();
    c1_status |= (FPU_CSR_FS | FPU_CSR_FO | FPU_CSR_FN);    // set FS, FO, FN
    c1_status &= ~(FPU_CSR_ALL_E);                          // disable exception
    c1_status = (c1_status & (~FPU_CSR_RM)) | FPU_CSR_RN;   // set RN
    write_c1_status(c1_status);

    return ;
}


void bsp_init(void)
{
    // 初始化调试串口
    uart2_init();
    
    // 硬浮点初始化
    fpu_init();

    // 初始化异常
    exception_init();

    // 显示时钟信息
    clk_print_all();

    return ;
}


int main(void)
{
    bsp_init();

    // -------------------------测试gpio----------------------
    /*
     * 测试库中gpio作为输出时的相关接口
     * led闪烁10次
     */
//    test_gpio_output();
    
    /*
     * 测试库中gpio作为输入时的相关接口
     * 按键按下时，指示灯点亮，否则，熄灭
     */
//    test_gpio_input();

    /*
     * 测试库中外部中断(gpio输入中断)的相关接口
     * 按键被按下后，会产生一个中断
     */
//    test_gpio_key_irq();


    // ------------------------测试串口-----------------------
    // 通过串口2打印helloworld
//    test_uart2_print_helloworld();
    
    // 测试串口2的收发功能是否正常
//    test_uart2_send_recv();

    // 测试串口2的收发功能是否正常
//    test_uart1_send_recv();

    // 测试串口3的收发功能是否正常
//    test_uart3_send_recv();

    // 测试printf()函数
    test_printf();


    // ------------------------测试PWM--------------------------------    
    // 测试硬件pwm产生连续的pwm波形
//    test_pwm_normal();

    // 测试硬件pwm产生pwm脉冲
//    test_pwm_pulse();
    
    /*
     * 测试gpio04复用为pwm,gpio06作为普通gpio使用
     * PWM0的默认引脚位GPIO06，但也可以复用为GPIO04
     * 当gpio06还是保持默认为pwm时，复用gpio04为pwm0，那么会同时在两个引脚输出相同的pwm波形
     * 本函数旨在证明可以在gpio04复用为pwm0时，还可以将(默认作为pwm0的)gpio06作为普通gpio使用
     */
//    test_pwm_gpio04_gpio06();

    // 测试pwm最大周期
//    test_pwm_max_period();


    // ------------------------测试软件延时--------------------------------  
    // 测试延时函数delay_1ms()
//    test_delay_1ms();
    
    // 测试延时函数delay_1us()
//    test_delay_1us();
    
    // 测试延时函数delay_1s()
//    test_delay_1s();


    // ------------------------测试模拟I2C------------------------------  
    // 测试模拟I2C
//    test_simulate_i2c_am2320();

    // ------------------------测试硬件I2C---------------------------
    // 用温湿度传感器测试硬件i2c
//    test_i2c_am2320();


    // ------------------------测试硬件定时器---------------------------  
    // 测试硬件定时器的定时功能(读取中断状态位的方式判断是否超时)
//    test_timer_poll_time_out();

    // 测试硬件定时器的中断
//    test_timer_irq();

    // 测试硬件定时器的计时
//    test_timer_get_time();

    // ------------------------测试硬浮点(FPU)---------------------------
    // 测试使用硬浮点进行浮点数的加减乘除
//    test_fpu();

    // ------------------------测试滴答定时器---------------------------
    // 通过获取当前tick值来测试滴答定时器，默认已经使能了滴答定时器，每秒1000个tick
//    test_sys_tick();


    // ------------------------测试硬件SPI---------------------------
    // 用tm7705测试硬件SPI
    // 具体为tm7705+ntc热敏电阻实现温度测量(3d打印机就可以采用此方案测量温度)
//    test_spi_tm7705();

    // ------------------------测试CAN---------------------------
    // 详情请参考文件test_can.c


    while (1)
        ;

	return(0);
}





