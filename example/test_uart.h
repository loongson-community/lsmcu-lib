// 测试串口的头文件


#ifndef __OPENLOONGSON_TEST_UART_H
#define __OPENLOONGSON_TEST_UART_H



/*
 * 测试串口2的收发功能是否正常
 */
void test_uart2_send_recv(void);


/*
 * 测试串口1的收发功能是否正常
 */
void test_uart1_send_recv(void);


// 测试串口3的收发功能是否正常
void test_uart3_send_recv(void);


// 测试printf()函数
void test_printf(void);


// 通过串口2打印helloworld
void test_uart2_print_helloworld(void);



#endif

