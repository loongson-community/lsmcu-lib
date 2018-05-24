// 测试硬件i2c源文件


#include "../lib/ls1c_public.h"
#include "../lib/ls1c_delay.h"
#include "../lib/ls1c_i2c.h"
#include "../lib/ls1c_pin.h"
#include "test_simulate_i2c.h"


// am2320返回的数据长度
#define AM2320_RSP_DATA_SIZE            (8)

// I2C1，引脚CAMDATA4(GPIO54)和CAMDATA5(GPIO55)的第四复用
#define LS1C_I2C_SDA1_GPIO54            (54)
#define LS1C_I2C_SCL1_GPIO55            (55)

// I2C2，引脚CAMDATA6(GPIO56)和CAMDATA7(GPIO57)的第四复用
#define LS1C_I2C_SDA2_GPIO56            (56)
#define LS1C_I2C_SCL2_GPIO57            (57)


// 用温湿度传感器测试硬件i2c
void test_i2c_am2320(void)
{
    ls1c_i2c_info_t i2c_info;
    int slave_addr = 0xb8 >> 1;     // AM2320手册中的地址0xB8是8位的，而我们这里地址是指其前7位
    unsigned char send_buff[] = {0x03, 0x00, 0x04};
    unsigned char recv_buff[AM2320_RSP_DATA_SIZE] = {0};
    unsigned short recved_crc, calced_crc;
    int temp, humi;
    
    // I2C2，引脚CAMDATA6(GPIO56)和CAMDATA7(GPIO57)的第四复用
    pin_set_remap(LS1C_I2C_SDA2_GPIO56, PIN_REMAP_FOURTH);
    pin_set_remap(LS1C_I2C_SCL2_GPIO57, PIN_REMAP_FOURTH);
    i2c_info.clock = 50*1000;       // 50kb/s
    i2c_info.I2Cx = LS1C_I2C_2;
    i2c_init(&i2c_info);
    
/*
    // 使用I2C1，引脚CAMDATA4(GPIO54)和CAMDATA5(GPIO55)的第四复用
    pin_set_remap(LS1C_I2C_SDA1_GPIO54, PIN_REMAP_FOURTH);
    pin_set_remap(LS1C_I2C_SCL1_GPIO55, PIN_REMAP_FOURTH);
    i2c_info.clock = 50*1000;       // 50kb/s
    i2c_info.I2Cx = LS1C_I2C_1;
    i2c_init(&i2c_info);
*/

/*
    // 使用I2C0，不需要复用
    i2c_info.clock = 50*1000;       // 50kb/s
    i2c_info.I2Cx = LS1C_I2C_0;
    i2c_init(&i2c_info);
*/

    while (1)
    {
        // 唤醒传感器
        i2c_send_start_and_addr(&i2c_info, slave_addr, LS1C_I2C_DIRECTION_WRITE);
        i2c_receive_ack(&i2c_info);
        delay_ms(1);
        i2c_send_stop(&i2c_info);

        // 发送读指令
        i2c_send_start_and_addr(&i2c_info, slave_addr, LS1C_I2C_DIRECTION_WRITE);
        i2c_receive_ack(&i2c_info);
        i2c_send_data(&i2c_info, send_buff, sizeof(send_buff));
        i2c_send_stop(&i2c_info);

        // 读取温湿度信息
        delay_ms(2);
        i2c_send_start_and_addr(&i2c_info, slave_addr, LS1C_I2C_DIRECTION_READ);
        i2c_receive_ack(&i2c_info);
        i2c_receive_data(&i2c_info, recv_buff, AM2320_RSP_DATA_SIZE);
        i2c_send_stop(&i2c_info);

        // 校验crc
        recved_crc = (recv_buff[AM2320_RSP_CRC_HIGH] << 8) + recv_buff[AM2320_RSP_CRC_LOW];
        calced_crc = am2320_crc16(recv_buff, 6);
        if (recved_crc != calced_crc)
        {
            printf("[%s] crc error! recved_crc=0x%x, calced_crc=0x%x\n", 
                      __FUNCTION__, recved_crc, calced_crc);
        }

        // 计算温度湿度
        humi = (recv_buff[AM2320_RSP_HUMI_HIGH] * 0xff + recv_buff[AM2320_RSP_HUMI_LOW]) / 10;
        temp = (recv_buff[AM2320_RSP_TEMP_HIGH] * 0xff + recv_buff[AM2320_RSP_TEMP_LOW]) / 10;
        printf("[%s] temp=%d, humi=%d\n", __FUNCTION__, temp, humi);
        
        // 间隔3s，再采集
        delay_s(3);
    }

    return ;
}



