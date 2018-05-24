// 测试串口的源文件

#include "../lib/ls1c_public.h"
#include "../lib/ls1c_uart.h"
#include "../lib/start.h"
#include "../lib/ls1c_irq.h"
#include "../lib/ls1c_pin.h"
#include "../lib/ls1c_delay.h"


/*
 * 串口的中断处理函数
 * 如有需要，可以给每个串口单独写个中断处理函数，或者通过入参"中断号"来区分串口，并单独处理
 * @IRQn 中断号
 * @param 传递给中断处理函数的参数
 */
void test_uart_irqhandler(int IRQn, void *param)
{
    ls1c_uart_t uartx = uart_irqn_to_uartx(IRQn);
    void *uart_base = uart_get_base(uartx);
    unsigned char iir = reg_read_8(uart_base + LS1C_UART_IIR_OFFSET);

    // 判断是否为接收超时或接收到有效数据
    if ((IIR_RXTOUT & iir) || (IIR_RXRDY & iir))
    {
        // 是，则读取数据，并原样发送回去
        while (LSR_RXRDY & reg_read_8(uart_base + LS1C_UART_LSR_OFFSET))
        {
            uart_putc(uartx, reg_read_8(uart_base + LS1C_UART_DAT_OFFSET));
        }
    }
    
    return ;
}


/*
 * 测试串口2的收发功能是否正常
 */
void test_uart2_send_recv(void)
{
    unsigned int tx_gpio = 37;
    unsigned int rx_gpio = 36;
    ls1c_uart_info_t uart2_info = {0};
        
    printf("\n\n[%s] uart2 echo, please input\r\n", __FUNCTION__);

    // 设置复用
    pin_set_remap(tx_gpio, PIN_REMAP_SECOND);
    pin_set_remap(rx_gpio, PIN_REMAP_SECOND);

    // 重新初始化串口2(使能接收中断)
    uart2_info.UARTx    = LS1C_UART2;
    uart2_info.baudrate = 115200;
    uart2_info.rx_enable= TRUE;         // 使能接收中断
    uart_init(&uart2_info);
    
    // 设置中断处理函数
    irq_install(LS1C_UART2_IRQ, test_uart_irqhandler, NULL);
    irq_enable(LS1C_UART2_IRQ);
    
    while (1)
    {
        delay_s(1);
    }
}


/*
 * 测试串口1的收发功能是否正常
 */
void test_uart1_send_recv(void)
{
    // EJTAG引脚复用比较特殊，除了需要设置复用寄存器外，
    // 还需要把引脚JTAG_SEL（注意，不是EJTAG_SEL）拉高，可能板子上有个跳线帽，注意观察
    unsigned int tx_gpio = 3;   // EJTAG_TDO/GPIO03
    unsigned int rx_gpio = 2;   // EJTAG_TDI/GPIO02
    ls1c_uart_info_t uart1_info = {0};

    printf("[%s] uart1 test echo. please input\r\n", __FUNCTION__);

    // 设置复用
    pin_set_remap(tx_gpio, PIN_REMAP_FOURTH);
    pin_set_remap(rx_gpio, PIN_REMAP_FOURTH);

    // 初始化串口
    uart1_info.UARTx    = LS1C_UART1;
    uart1_info.baudrate = 115200;
    uart1_info.rx_enable= TRUE;     
    uart_init(&uart1_info);

    // 设置中断处理函数
    irq_install(LS1C_UART1_IRQ, test_uart_irqhandler, NULL);
    irq_enable(LS1C_UART1_IRQ);

    while (1)
    {
        delay_s(1);
    }
}


// 测试串口3的收发功能是否正常
void test_uart3_send_recv(void)
{
    // EJTAG引脚复用比较特殊，除了需要设置复用寄存器外，
    // 还需要把引脚JTAG_SEL（注意，不是EJTAG_SEL）拉高，可能板子上有个跳线帽，注意观察
    unsigned int tx_gpio = 1;   // EJTAG_TCK/GPIO01
    unsigned int rx_gpio = 0;   // EJTAG_SEL/GPIO00
    ls1c_uart_info_t uart3_info = {0};

    printf("[%s] uart3 test echo. please input\r\n", __FUNCTION__);

    // 设置复用
    pin_set_remap(tx_gpio, PIN_REMAP_FOURTH);
    pin_set_remap(rx_gpio, PIN_REMAP_FOURTH);

    // 初始化串口
    uart3_info.UARTx    = LS1C_UART3;
    uart3_info.baudrate = 115200;
    uart3_info.rx_enable= TRUE;
    uart_init(&uart3_info);

    // 设置中断处理函数
    irq_install(LS1C_UART3_IRQ, test_uart_irqhandler, NULL);
    irq_enable(LS1C_UART3_IRQ);

    // 通过串口3先发送一个字符串，用于单独判断发送功能是否正常
    uart_print(LS1C_UART3, "uart3 send\r\n");

    while (1)
    {
        delay_s(1);
        printf("[%s] ......\r\n", __FUNCTION__);
    }
}

// 测试printf()函数
void test_printf(void)
{
    int i = 0;
    float f = -5.1;
    
    printf("[%s] hello!\r\n", __FUNCTION__);

    while (1)
    {
        f += i;
        printf("[%s] i=%d, f=%f\r\n", __FUNCTION__, i++, f);
        delay_s(1);
    }
}


// 通过串口2打印helloworld
void test_uart2_print_helloworld(void)
{
    while (1)
    {
        uart2_print("hello world!\r\n");
        delay_s(1);
    }
}



