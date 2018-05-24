// 测试模拟i2c


#include "../lib/ls1c_public.h"
#include "../lib/ls1c_delay.h"
#include "../lib/ls1c_simulate_i2c.h"
#include "test_simulate_i2c.h"


// 接收缓存大小
#define RECV_BUFF_SIZE                (8)


// 温湿度传感器AM2320的引脚接线信息
simulate_i2c_t am2320_info = { 57, 56, 10 };        // SCL=gpio57, SDA=gpio56, delay_time=10us


// 温湿度传感器AM2320初始化
void am2320_init(void)
{
    // 模拟i2c初始化
    simulate_i2c_init(&am2320_info);

    return ;
}


/*
 * 计算crc
 * @ptr 待计算crc的数据的首地址
 * @len 数据长度
 */
unsigned short am2320_crc16(unsigned char *ptr, unsigned char len)
{
    unsigned short crc = 0xFFFF;
    unsigned char i;

    while (len--)
    {
        crc ^= *ptr++;
        for (i=0; i<8; i++)
        {
            if (crc & 0x01)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return crc;
}


// 从AM2320读取温湿度信息
void am2320_get_temp_humi(void)
{
    const unsigned char addr = 0xB8; 
    unsigned char recv_buff[RECV_BUFF_SIZE] = {0};
    unsigned short recved_crc, calced_crc;
    int temp, humi;
    int i;
    
    // 唤醒AM2320
    simulate_i2c_start(&am2320_info);
    simulate_i2c_write_byte(&am2320_info, addr);
    simulate_i2c_read_ack(&am2320_info);
    delay_ms(1);
    simulate_i2c_stop(&am2320_info);

    // 发送读指令
    simulate_i2c_start(&am2320_info);
    simulate_i2c_write_byte(&am2320_info, addr);
    simulate_i2c_read_ack(&am2320_info);
    simulate_i2c_write_byte(&am2320_info, 0x03);
    simulate_i2c_read_ack(&am2320_info);
    simulate_i2c_write_byte(&am2320_info, 0x00);
    simulate_i2c_read_ack(&am2320_info);
    simulate_i2c_write_byte(&am2320_info, 0x04);
    simulate_i2c_read_ack(&am2320_info);
    simulate_i2c_stop(&am2320_info);

    // 读回数据
    delay_ms(2);
    simulate_i2c_start(&am2320_info);
    simulate_i2c_write_byte(&am2320_info, addr | 0x01);
    simulate_i2c_read_ack(&am2320_info);
    delay_us(50);
    for (i=0; i<RECV_BUFF_SIZE; i++)
    {
        recv_buff[i] = simulate_i2c_read_byte(&am2320_info);
        simulate_i2c_send_ack(&am2320_info);
    }
    simulate_i2c_stop(&am2320_info);

    recved_crc = (recv_buff[AM2320_RSP_CRC_HIGH] << 8) + recv_buff[AM2320_RSP_CRC_LOW];
    calced_crc = am2320_crc16(recv_buff, 6);
    if (recved_crc != calced_crc)
    {
        printf("[%s] crc error! recved_crc=0x%x, calced_crc=0x%x\n", __FUNCTION__, recved_crc, calced_crc);
        printf("[%s] recved data: func_id=%d, len=%d, humi[1]=0x%x, humi[0]=0x%x, temp[1]=0x%x, temp[0]=0x%x, crc[0]=0x%x, crc[1]=0x%x\n",
                  __FUNCTION__, 
                  recv_buff[AM2320_RSP_FUNC_ID],
                  recv_buff[AM2320_RSP_LEN],
                  recv_buff[AM2320_RSP_HUMI_HIGH],
                  recv_buff[AM2320_RSP_HUMI_LOW],
                  recv_buff[AM2320_RSP_TEMP_HIGH],
                  recv_buff[AM2320_RSP_TEMP_LOW],
                  recv_buff[AM2320_RSP_CRC_LOW],
                  recv_buff[AM2320_RSP_CRC_HIGH]);
        return ;
    }  

    humi = (recv_buff[AM2320_RSP_HUMI_HIGH] * 0xff + recv_buff[AM2320_RSP_HUMI_LOW]) / 10;
    temp = (recv_buff[AM2320_RSP_TEMP_HIGH] * 0xff + recv_buff[AM2320_RSP_TEMP_LOW]) / 10;
    printf("[%s] humi=%d, temp=%d\n", __FUNCTION__, humi, temp);

    return ;
}



// 用模拟i2c接口与温湿度传感器AM2320通信，读取温湿度信息
void test_simulate_i2c_am2320(void)
{
    // 温湿度传感器AM2320初始化
    am2320_init();

    while (1)
    {
        // 从AM2320读取温湿度信息
        am2320_get_temp_humi();

        // 等待3s
        delay_s(3);
    }
}



