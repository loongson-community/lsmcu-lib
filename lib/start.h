

#ifndef __OPENLOONGSON_START_H
#define __OPENLOONGSON_START_H


// 串口各寄存器相对基地址的偏移
#define LS1C_UART_DAT_OFFSET            (0)
#define LS1C_UART_IER_OFFSET            (1)
#define LS1C_UART_IIR_OFFSET            (2)
#define LS1C_UART_FCR_OFFSET            (2)
#define LS1C_UART_LCR_OFFSET            (3)
#define LS1C_UART_MCR_OFFSET            (4)
#define LS1C_UART_LSR_OFFSET            (5)
#define LS1C_UART_MSR_OFFSET            (6)

#define LS1C_UART_LSB_OFFSET            (0)     // 分频锁存器1
#define LS1C_UART_MSB_OFFSET            (1)     // 分频锁存器2

/* interrupt enable register */
#define	IER_IRxE	0x1	/* 接收有效数据中断使能 */
#define	IER_ITxE	0x2	/* 传输保存寄存器为空中断使能 */
#define	IER_ILE	    0x4	/* 接收器线路状态中断使能 */
#define	IER_IME	    0x8	/* Modem状态中断使能 */

/* interrupt identification register */
#define	IIR_IMASK	0xf	/* mask */
#define	IIR_RXTOUT	0xc	/* receive timeout */
#define	IIR_RLS		0x6	/* receive line status */
#define	IIR_RXRDY	0x4	/* receive ready */
#define	IIR_TXRDY	0x2	/* transmit ready */
#define	IIR_NOPEND	0x1	/* nothing */
#define	IIR_MLSC	0x0	/* modem status */
#define	IIR_FIFO_MASK	0xc0	/* set if FIFOs are enabled */

/* fifo control register */
#define	FIFO_ENABLE	0x01	/* enable fifo */
#define	FIFO_RCV_RST	0x02	/* reset receive fifo */
#define	FIFO_XMT_RST	0x04	/* reset transmit fifo */
#define	FIFO_DMA_MODE	0x08	/* enable dma mode */
#define	FIFO_TRIGGER_1	0x00	/* trigger at 1 char */
#define	FIFO_TRIGGER_4	0x40	/* trigger at 4 chars */
#define	FIFO_TRIGGER_8	0x80	/* trigger at 8 chars */
#define	FIFO_TRIGGER_14	0xc0	/* trigger at 14 chars */

// 线路控制寄存器
/* character format control register */
#define	CFCR_DLAB	0x80	/* divisor latch */
#define	CFCR_SBREAK	0x40	/* send break */
#define	CFCR_PZERO	0x30	/* zero parity */
#define	CFCR_PONE	0x20	/* one parity */
#define	CFCR_PEVEN	0x10	/* even parity */
#define	CFCR_PODD	0x00	/* odd parity */
#define	CFCR_PENAB	0x08	/* parity enable */
#define	CFCR_STOPB	0x04	/* 2 stop bits */
#define	CFCR_8BITS	0x03	/* 8 data bits */
#define	CFCR_7BITS	0x02	/* 7 data bits */
#define	CFCR_6BITS	0x01	/* 6 data bits */
#define	CFCR_5BITS	0x00	/* 5 data bits */

/* modem control register */
#define	MCR_LOOPBACK	0x10	/* loopback */
#define	MCR_IENABLE	0x08	/* output 2 = int enable */
#define	MCR_DRS		0x04	/* output 1 = xxx */
#define	MCR_RTS		0x02	/* enable RTS */
#define	MCR_DTR		0x01	/* enable DTR */

/* line status register */
#define	LSR_RCV_FIFO	0x80	/* error in receive fifo */
#define	LSR_TSRE	0x40	/* transmitter empty */
#define	LSR_TXRDY	0x20	/* transmitter ready */
#define	LSR_BI		0x10	/* break detected */
#define	LSR_FE		0x08	/* framing error */
#define	LSR_PE		0x04	/* parity error */
#define	LSR_OE		0x02	/* overrun error */
#define	LSR_RXRDY	0x01	/* receiver ready */
#define	LSR_RCV_MASK	0x1f


// 波特率
#define B0      0
#define B50     50      
#define B75     75
#define B110    110
#define B134    134
#define B150    150
#define B200    200
#define B300    300
#define B600    600
#define B1200   1200
#define B1800   1800
#define B2400   2400
#define B4800   4800
#define B9600   9600
#define B19200  19200
#define B38400  38400
#define B57600  57600
#define B115200 115200
#define B230400 230400
#define B380400 380400
#define B460800 460800
#define B921600 921600



// 晶振的频率
#define AHB_CLK                 (24000000)
#define APB_CLK                 (AHB_CLK)


// START_FREQ寄存器bits
#define M_PLL_SHIFT             (8)
#define M_PLL                   (0xff << M_PLL_SHIFT)       // PLL倍频系数的整数部分
#define FRAC_N_SHIFT            (16)
#define FRAC_N                  (0xff << FRAC_N_SHIFT)      // PLL倍频系数的小数部分
#define DIV_SDRAM_SHIFT         (0)
#define DIV_SDRAM               (0x3  << DIV_SDRAM_SHIFT)

// CLK_DIV_PARAM寄存器bits
#define DIV_PIX_EN              (0x1  << 31)
#define DIV_PIX                 (0x7f << 24)
#define DIV_CAM_EN              (0x1  << 23)
#define DIV_CAM                 (0x7f << 16)
#define DIV_CPU_EN              (0x1  << 15)
#define DIV_CPU                 (0x7f << 8)
#define DIV_PIX_VALID           (0x1  << 5)
#define DIV_PIX_SEL             (0x1  << 4)
#define DIV_CAM_VALID           (0x1  << 3)
#define DIV_CAM_SEL             (0x1  << 2)
#define DIV_CPU_VALID           (0x1  << 1)
#define DIV_CPU_SEL             (0x1  << 0)

#define DIV_PIX_SHIFT           (24)
#define DIV_CAM_SHIFT           (16)
#define DIV_CPU_SHIFT           (8)


#endif

