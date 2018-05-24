// 测试模拟i2c


#ifndef __OPENLOONGSON_TEST_SIMULATE_I2C_H
#define __OPENLOONGSON_TEST_SIMULATE_I2C_H


// 读取的消息中每字节的含义
enum
{
    AM2320_RSP_FUNC_ID = 0,             // 功能码
    AM2320_RSP_LEN,                     // 数据长度
    AM2320_RSP_HUMI_HIGH,               // 湿度高位
    AM2320_RSP_HUMI_LOW,                // 湿度低位
    AM2320_RSP_TEMP_HIGH,               // 温度高位
    AM2320_RSP_TEMP_LOW,                // 温度低位
    AM2320_RSP_CRC_LOW,                 // CRC低位
    AM2320_RSP_CRC_HIGH,                // CRC高位
};


/*
 * 计算crc
 * @ptr 待计算crc的数据的首地址
 * @len 数据长度
 */
unsigned short am2320_crc16(unsigned char *ptr, unsigned char len);


// 用模拟i2c接口与温湿度传感器AM2320通信，读取温湿度信息
void test_simulate_i2c_am2320(void);


#endif

