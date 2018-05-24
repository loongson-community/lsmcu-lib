// 测试硬件spi源文件

#include "../lib/ls1c_public.h"
#include "../lib/ls1c_pin.h"
#include "../lib/ls1c_spi.h"
#include "../lib/ls1c_gpio.h"
#include "../lib/ls1c_delay.h"


// spi复用
#define LS1C_SPI_1_CS_0_GPIO            (49)        // gpio49/spi1_cs0/CAMHSYNC
#define LS1C_SPI_1_CS_1_GPIO            (50)        // gpio50/spi1_cs1/CAMDATA0
#define LS1C_SPI_1_CS_2_GPIO            (51)        // gpio51/spi1_cs2/CAMDATA1
#define LS1C_SPI_1_CS_3_GPIO            (52)        // gpio52/spi1_cs3/CAMDATA2
#define LS1C_SPI_1_MISO_GPIO            (47)        // gpio47/spi1_miso/CAMCLKOUT
#define LS1C_SPI_1_MOSI_GPIO            (48)        // gpio48/spi1_mosi/CAMVSYNC
#define LS1C_SPI_1_CLK_GPIO             (46)        // gpio46/spi1_clk/CAMPCLKIN


// 通信寄存器bit定义
enum 
{
	// 寄存器选择  RS2 RS1 RS0
	TM7705_REG_COMM	        = (0 << 4), // 通信寄存器
	TM7705_REG_SETUP	    = (1 << 4), // 设置寄存器
	TM7705_REG_CLOCK	    = (2 << 4), // 时钟寄存器
	TM7705_REG_DATA	        = (3 << 4), // 数据寄存器
	TM7705_REG_TEST         = (4 << 4), // 测试寄存器
	TM7705_REG_OFFSET       = (6 << 4), // 偏移寄存器
	TM7705_REG_GAIN         = (7 << 4), // 增益寄存器
	
    // 读写操作
	TM7705_WRITE 		    = (0 << 3), // 写操作
	TM7705_READ 		    = (1 << 3), // 读操作

	// 通道
	TM7705_CH_1		        = 0,    // AIN1+  AIN1-
	TM7705_CH_2		        = 1,    // AIN2+  AIN2-
	TM7705_CH_3		        = 2,    // AIN1-  AIN1-
	TM7705_CH_4		        = 3     // AIN1-  AIN2-
};


/* 设置寄存器bit定义 */
enum
{
	TM7705_MD_NORMAL		= (0 << 6),	/* 正常模式 */
	TM7705_MD_CAL_SELF		= (1 << 6),	/* 自校准模式 */
	TM7705_MD_CAL_ZERO		= (2 << 6),	/* 校准0刻度模式 */
	TM7705_MD_CAL_FULL		= (3 << 6),	/* 校准满刻度模式 */

	TM7705_GAIN_1			= (0 << 3),	/* 增益 */
	TM7705_GAIN_2			= (1 << 3),	/* 增益 */
	TM7705_GAIN_4			= (2 << 3),	/* 增益 */
	TM7705_GAIN_8			= (3 << 3),	/* 增益 */
	TM7705_GAIN_16			= (4 << 3),	/* 增益 */
	TM7705_GAIN_32			= (5 << 3),	/* 增益 */
	TM7705_GAIN_64			= (6 << 3),	/* 增益 */
	TM7705_GAIN_128		    = (7 << 3),	/* 增益 */

	/* 无论双极性还是单极性都不改变任何输入信号的状态，它只改变输出数据的代码和转换函数上的校准点 */
	TM7705_BIPOLAR			= (0 << 2),	/* 双极性输入 */
	TM7705_UNIPOLAR		    = (1 << 2),	/* 单极性输入 */

	TM7705_BUF_NO			= (0 << 1),	/* 输入无缓冲（内部缓冲器不启用) */
	TM7705_BUF_EN			= (1 << 1),	/* 输入有缓冲 (启用内部缓冲器) */

	TM7705_FSYNC_0			= 0,    // 模拟调制器和滤波器正常处理数据
	TM7705_FSYNC_1			= 1		// 模拟调制器和滤波器不启用
};



/* 时钟寄存器bit定义 */
enum
{
	TM7705_CLKDIS_0	        = (0 << 4),		/* 时钟输出使能 （当外接晶振时，必须使能才能振荡） */
	TM7705_CLKDIS_1	        = (1 << 4),		/* 时钟禁止 （当外部提供时钟时，设置该位可以禁止MCK_OUT引脚输出时钟以省电 */

    TM7705_CLKDIV_0         = (0 << 3),     // 不分频
    TM7705_CLKDIV_1         = (1 << 3),     // 2分频，外部晶振为4.9152Mhz时，应2分频

    TM7705_CLK_0            = (0 << 2),     // 主时钟=1Mhz并且CLKDIV=0，主时钟=2Mhz并且CLKDIV=1
    TM7705_CLK_1            = (1 << 2),     // 主时钟=2.4576Mhz并且CLKDIV=0, 主时钟=4.9152Mhz并且CLKDIV=1

    // 注意输出更新率与clk位有关
    // 当TM7705_CLK_0时，输出更新率只能为20,25,100,200
    TM7705_UPDATE_20        = (0),
    TM7705_UPDATE_25        = (1),
    TM7705_UPDATE_100       = (2),
    TM7705_UPDATE_200       = (3),
    // 当TM7705_CLK_1时，输出更新率只能为50,60,250,500
    TM7705_UPDATE_50        = (0),
    TM7705_UPDATE_60        = (1),
    TM7705_UPDATE_250       = (2),
    TM7705_UPDATE_500       = (3)
};



#define TM7705_CHANNEL_NUM              (2)     // tm7705通道个数
#define TM7705_DRDY_PIN                 (87)    // GPIO87/I2S_DI   tm7705的引脚DRDY 
#define TM7705_RESET_PIN                (89)    // GPIO89/I2S_LRCK  tm7705的引脚RESET

#define TM7705_AD_MAX                   ((0x1 << 10) - 1)   // ad的最大值，只用了十位的精度
#define TM7705_IS_VALID_AD(ad)          ((TM7705_AD_MAX >= (ad)) && (0 <= (ad)))

// 返回值
#define TM7705_RET_TIMEOUT              (-1)    // 超时
#define TM7705_RET_OK                   (0)     // 正常返回
#define TM7705_RET_OTHER_ERR            (1)     // 其它错误


// 以下根据ntc热敏电阻参数用脚本生成的adc值与温度一一对应的表格
// 左边为adc值，右边为温度(单位:摄氏度)
// 详细请参考源码目录中的脚本"createTemperatureLookup.py"
// python createTemperatureLookup.py
// Thermistor lookup table for RepRap Temperature Sensor Boards (http://make.rrrf.org/ts)
// Made with createTemperatureLookup.py (http://svn.reprap.org/trunk/reprap/firmware/Arduino/utilities/createTemperatureLookup.py)
// ./createTemperatureLookup.py --r0=100000 --t0=25 --r1=0 --r2=4700 --beta=3950 --max-adc=1023
// r0: 100000
// t0: 25
// r1: 0
// r2: 4700
// beta: 3950
// max adc: 1023
#define TM7705_NTC_NUMTEMPS 40
const short tm7705_ntc_temptable[TM7705_NTC_NUMTEMPS][2] = {
   {1, 938},
   {27, 326},
   {53, 269},
   {79, 239},
   {105, 219},
   {131, 204},
   {157, 192},
   {183, 182},
   {209, 174},
   {235, 166},
   {261, 160},
   {287, 153},
   {313, 148},
   {339, 143},
   {365, 138},
   {391, 133},
   {417, 129},
   {443, 125},
   {469, 120},
   {495, 116},
   {521, 113},
   {547, 109},
   {573, 105},
   {599, 101},
   {625, 98},
   {651, 94},
   {677, 90},
   {703, 86},
   {729, 82},
   {755, 78},
   {781, 74},
   {807, 70},
   {833, 65},
   {859, 60},
   {885, 54},
   {911, 48},
   {937, 41},
   {963, 31},
   {989, 18},
   {1015, -8}
};



ls1c_spi_info_t tm7705_spi_info = {0};


/*
 * 通过RESET引脚复位tm7705
 */
void tm7705_reset(void)
{
    gpio_set(TM7705_RESET_PIN, gpio_level_high);
    delay_ms(1);
    gpio_set(TM7705_RESET_PIN, gpio_level_low);
    delay_ms(2);
    gpio_set(TM7705_RESET_PIN, gpio_level_high);
    delay_ms(1);

    return ;
}


/*
 * 同步SPI接口时序
 */
void tm7705_sync_spi(void)
{
    // 在至少32个串行时钟内向TM7705的DIN脚写入逻辑'1'
    spi_set_cs(&tm7705_spi_info, 0);
    delay_us(1);
    
    spi_txrx_byte(&tm7705_spi_info, 0xFF);
    spi_txrx_byte(&tm7705_spi_info, 0xFF);
    spi_txrx_byte(&tm7705_spi_info, 0xFF);
    spi_txrx_byte(&tm7705_spi_info, 0xFF);
    
    spi_set_cs(&tm7705_spi_info, 1);
    delay_us(0);

    return ;
}


/*
 * 等待内部操作完成， 时间较长，约180ms
 * @ret 成功 或者 超时
 */
int tm7705_wait_DRDY(void)
{
    int i = 0;
    int time_cnt = 500;

    for (i=0; i<time_cnt; i++)
    {
        if (gpio_level_low == gpio_get(TM7705_DRDY_PIN))
        {
            break;
        }
        delay_ms(1);
    }

    if (i >= time_cnt)
    {
        return TM7705_RET_TIMEOUT;
    }

    return TM7705_RET_OK;
}


/*
 * 自校准
 * @channel 通道
 */
void tm7705_calib_self(unsigned char channel)
{
    unsigned char val = 0;

    spi_set_cs(&tm7705_spi_info, 0);
    delay_us(1);

    val = TM7705_REG_SETUP | TM7705_WRITE | channel;
    spi_txrx_byte(&tm7705_spi_info, val);
    val = TM7705_MD_CAL_SELF | TM7705_GAIN_1 | TM7705_UNIPOLAR | TM7705_BUF_EN | TM7705_FSYNC_0;
    spi_txrx_byte(&tm7705_spi_info, val);

    spi_set_cs(&tm7705_spi_info, 1);
    delay_us(1);

    // 等待内部操作完成， 时间较长，约180ms
    tm7705_wait_DRDY();

    delay_ms(50);

    return ;
}



/*
 * 配置tm7705的指定通道
 * @channel 通道
 */
void tm7705_config_channel(unsigned char channel)
{
    unsigned char val = 0;

    spi_set_cs(&tm7705_spi_info, 0);
    delay_us(1);

    val = TM7705_REG_CLOCK | TM7705_WRITE | channel;
    spi_txrx_byte(&tm7705_spi_info, val);
    val = TM7705_CLKDIS_0 | TM7705_CLKDIV_1 | TM7705_CLK_1 | TM7705_UPDATE_50;
    spi_txrx_byte(&tm7705_spi_info, val);

    spi_set_cs(&tm7705_spi_info, 1);
    delay_us(1);

    // 自校准
    tm7705_calib_self(channel);

    return ;
}


/*
 * 复位tm7705并重新配置
 */
void tm7705_reset_and_reconfig(void)
{
    // 通过RESET引脚复位tm7705
    tm7705_reset();

    // 同步SPI接口时序
    delay_ms(5);
    tm7705_sync_spi();
    delay_ms(5);

    // 配置tm7705的指定通道
    tm7705_config_channel(TM7705_CH_1);

    return ;
}


/*
 * tm7705初始化
 */
void tm7705_init(void)
{
    // 初始化DRDY和RESET引脚
    gpio_init(TM7705_DRDY_PIN, gpio_mode_input);
    gpio_init(TM7705_RESET_PIN, gpio_mode_output);
    gpio_set(TM7705_RESET_PIN, gpio_level_high);

    // 初始化SPI
/*    
    // tm7705在SPI0的CS1上，通信速度为100khz，cpol=1，cpha=1
    tm7705_spi_info.SPIx   = LS1C_SPI_0;
    tm7705_spi_info.cs     = LS1C_SPI_CS_1;
*/
/*
    // SPI0 CS3
    tm7705_spi_info.SPIx   = LS1C_SPI_0;
    tm7705_spi_info.cs     = LS1C_SPI_CS_3;
*/
/*
    // SPI1 CS0
    pin_set_remap(LS1C_SPI_1_MISO_GPIO, PIN_REMAP_THIRD);
    pin_set_remap(LS1C_SPI_1_MOSI_GPIO, PIN_REMAP_THIRD);
    pin_set_remap(LS1C_SPI_1_CLK_GPIO,  PIN_REMAP_THIRD);
    pin_set_remap(LS1C_SPI_1_CS_0_GPIO, PIN_REMAP_THIRD);   // cs0
    tm7705_spi_info.SPIx   = LS1C_SPI_1;
    tm7705_spi_info.cs     = LS1C_SPI_CS_0;                 // cs0
*/

    // SPI1 CS1
    pin_set_remap(LS1C_SPI_1_MISO_GPIO, PIN_REMAP_THIRD);
    pin_set_remap(LS1C_SPI_1_MOSI_GPIO, PIN_REMAP_THIRD);
    pin_set_remap(LS1C_SPI_1_CLK_GPIO,  PIN_REMAP_THIRD);
    pin_set_remap(LS1C_SPI_1_CS_1_GPIO, PIN_REMAP_THIRD);   // cs1
    tm7705_spi_info.SPIx   = LS1C_SPI_1;
    tm7705_spi_info.cs     = LS1C_SPI_CS_1;                 // cs1


    tm7705_spi_info.max_speed_hz   = 100*1000;
    tm7705_spi_info.cpol   = SPI_CPOL_1;
    tm7705_spi_info.cpha   = SPI_CPHA_1;

    spi_init(&tm7705_spi_info);
    
    // 复位tm7705并重新配置
    tm7705_reset_and_reconfig();

    return ;
}


/*
 * 读取指定通道的ad值
 * @channel 通道
 * @adc_p 读取的AD值
 * @ret 成功 或 失败
 */
int tm7705_read_channel(int channel, unsigned short *adc_p)
{
    int ret = TM7705_RET_TIMEOUT;
    unsigned char tx = 0;
    unsigned char rx[2] = {0};
    unsigned short ad = 0;

    // 等待转换完成
    ret = tm7705_wait_DRDY();
    if (TM7705_RET_OK != ret)
    {
        printf("[%s] tm7705 timeout!\r\n", __FUNCTION__);
        return ret;
    }

    // 读
    spi_set_cs(&tm7705_spi_info, 0);
    delay_us(1);
    
    tx = TM7705_REG_DATA | TM7705_READ | channel;
    spi_txrx_byte(&tm7705_spi_info, tx);
    rx[0] = spi_txrx_byte(&tm7705_spi_info, 0);
    rx[1] = spi_txrx_byte(&tm7705_spi_info, 0);
    
    spi_set_cs(&tm7705_spi_info, 1);
    delay_us(1);
    
    ad = (rx[0] << 8) + rx[1];
    if (0xfff == ad)
    {
        printf("[%s] ad=0xfff\r\n", __FUNCTION__);
        return TM7705_RET_OTHER_ERR;
    }

    *adc_p = ad;

    return TM7705_RET_OK;
}



/*
 * 获取tm7705输出的AD值
 * @channel 通道
 * @adc_p 读取的AD值
 * @ret 成功 或者 失败
 */
int tm7705_get_ad(int channel, unsigned short *adc_p)
{
    int i = 0;
    int ret = TM7705_RET_TIMEOUT;
    unsigned short ad = 0;

    // 连续读2次
    // 第一次读取的值为上一次采集的结果，第二次读取的才是当前的结果     *
    for (i=0; i<2; i++)
    {
        ret = tm7705_read_channel(channel, &ad);
        if (TM7705_RET_OK != ret)
        {
            // 失败，则重启tm7705并重新配置
            tm7705_reset_and_reconfig();
            printf("[%s] tm7705 reset and reconfig!\r\n", __FUNCTION__);
            return ret;
        }

        // ls1c速度相对tm7705太快，延时一下避免在一次读完后DRDY还未及时改变状态
        delay_ms(1);
    }
//    spi_print_all_regs_info(&tm7705_spi_info);

    printf("[%s] ad=0x%x\r\n", __FUNCTION__, ad);

    ad = ad >> 6;       // 只需要10位的精度
    *adc_p = ad;

    return TM7705_RET_OK;
}


/*
 * 根据ad值计算温度值
 * @ad 从tm7705读取的AD值
 * @ret 温度值
 *
 * ntc热敏电阻的阻值温度曲线被分为n段，每段可以近似为直线
 * 所以温度值的计算就转变为查表再计算
 */
float tm7705_calc_temp_from_ad(unsigned short ad)
{
    float temp_f;
    int i = 0;
    
    // 判断ad值是否在量程范围内
    if (!(TM7705_IS_VALID_AD(ad)))
    {
        return 0;
    }

    // 判断是否在表格所表示的范围内
    if (ad < tm7705_ntc_temptable[0][0])                        // 小于表格的最小adc
    {
        return tm7705_ntc_temptable[0][1];                      // 取最小值
    }
    if (ad > tm7705_ntc_temptable[TM7705_NTC_NUMTEMPS-1][0])    // 大于表格的最大adc
    {
        return tm7705_ntc_temptable[TM7705_NTC_NUMTEMPS-1][1];  // 取最大值
    }

    // 查表
    for (i=1; i<TM7705_NTC_NUMTEMPS; i++)
    {
        if (ad < tm7705_ntc_temptable[i][0])
        {
//            printf("[%s] ad=%d\r\n", __FUNCTION__, ad);
            // t = t0 + (adc - adc0) * k
            temp_f = tm7705_ntc_temptable[i-1][1] +               // t0
                   (ad - tm7705_ntc_temptable[i-1][0]) *        // adc - adc0
                   ((float)(tm7705_ntc_temptable[i][1] - tm7705_ntc_temptable[i-1][1]) 
                    / (float)(tm7705_ntc_temptable[i][0] - tm7705_ntc_temptable[i-1][0]));
            return temp_f;
        }
    }

    return 0;
}


/*
 * 获取温度值
 * @ret 温度值
 */
float tm7705_get_temperature(void)
{
    int ret = TM7705_RET_TIMEOUT;
    float temp_f = 0;
    unsigned short ad = 0;

    // 获取AD值
    ret = tm7705_get_ad(TM7705_CH_1, &ad);
    if (TM7705_RET_OK != ret)
    {
        printf("[%s] tm7705_get_ad() fail, ret=%d\r\n", __FUNCTION__, ret);
        return ret;
    }

    // 计算温度值
//    temp_f = tm7705_calc_temp_from_ad(ad);

    return temp_f;
}


/*
 * 用tm7705测试硬件SPI
 * 具体为tm7705+ntc热敏电阻实现温度测量(3d打印机就可以采用此方案测量温度)
 */
void test_spi_tm7705(void)
{
    float temp_f;
    int temp = 0;       // 温度值
    
    // tm7705初始化
    tm7705_init();

    while (1)
    {
        // 获取温度值
        // 这里浮点运算有点小问题，暂不清楚原因，不知道是不是栈没有8字节对齐
        // 所以只把从tm7705读取的AD值打印出来
        temp_f = tm7705_get_temperature();
        temp = temp_f;
//        printf("[%s] temp=%d\r\n", __FUNCTION__, temp);

        // 延时
        delay_s(2);
    }
}




