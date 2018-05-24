

/*
 * 汇编文件的扩展名必须是大写的S，不能用小写的s。否则不会执行预处理，
 * 即把#include, #define等这些当作以'#'开头的注释，而忽略掉，最终导致编译错误
 */
#include "ls1c_regdef.h"
#include "ls1c_mipsregs.h"
#include "ls1c_asm.h"
#include "ls1c_stackframe.h"
#include "ls1c_cacheops.h"
#include "ls1c_regs.h"
#include "start.h"
#include "cpu.h"
#include "sdram_cfg.h"

#define DEBUG_LOCORE


// 配置调试串口
#define UART_BASE_ADDR      LS1C_UART2_BASE     // 串口2作为调试串口
#define CONS_BAUD           B115200             // 波特率115200

/* config pll div for cpu and sdram */
#define PLL_MULT            (0x54)  // 晶振为24Mhz时，PLL=504Mhz
#define SDRAM_DIV           (0)     // SDRAM为CPU的2分频
#define CPU_DIV             (2)     // CPU为PLL的2分频

// 配置内存大小
#define MEM_SIZE    (0x02000000)        // 32MByte



/***********************************MAIN FLOW****************************************/

#ifndef BOOT_FROM_EJTAG      
#define BOOT_FROM_EJTAG
#endif
#undef BOOT_FROM_EJTAG

#ifdef DEBUG_LOCORE
#define	TTYDBG(x) \
	.rdata;98: .asciz x; .text; la a0, 98b; bal stringserial; nop
#else
#define TTYDBG(x)
#endif

#ifdef	FAST_STARTUP
#define	PRINTSTR(x) 
#else
#define	PRINTSTR(x) \
	.rdata;98: .asciz x; .text; la a0, 98b; bal stringserial; nop
#endif

#ifdef	FAST_STARTUP
#define	FCALL_PRINTSTR(x) 
#else
#define	FCALL_PRINTSTR(x) \
	.rdata;98: .asciz x; .text; la a0, 98b; la v0, stringserial; addu v0,s0;jalr v0; nop	
#endif

#undef BAL
#define BAL(x) \
	la v0,x; \
	addu v0,s0; \
	jalr v0; \
	nop;

/* Delay macro */
#define	DELAY(count)	\
	li v0, count;	\
99:			\
	bnez	v0, 99b;\
	addiu	v0, -1

#define tmpsize		s1
#define msize		s2
#define	output_en	s3
#define bonito		s4
#define dbg			s5
#define sdCfg		s6

#define CP0_CONFIG $16
#define CP0_TAGLO  $28
#define CP0_TAGHI  $29

/*
 *   Register usage:
 *
 *	s0	link versus load offset, used to relocate absolute adresses.
 *	s1	free
 *	s2	memory size.
 *	s3	free.
 *	s4	Bonito base address.
 *	s5	dbg.
 *	s6	sdCfg.
 *	s7	rasave.
 *	s8	L3 Cache size.
 */

	.set	noreorder
	.set	mips32
	.globl	_start
	.globl	start
	.globl	__main
_start:
start:
    /*
    设置栈指针为start地址之前0x4000的位置
    mips架构堆栈寄存器实际只是通用寄存器，并没有规定生长方向，但软件约定“堆栈指针向下生长”
    */
	.globl	stack
stack = start - 0x4000		/* Place PMON stack below PMON start in RAM */

/* NOTE!! Not more that 16 instructions here!!! Right now it's FULL! */
/*
    根据“《see mips run》第5.3节——异常向量：异常处理开始的地方”中的描述，
    异常向量间的距离为128字节(0x80)，可容纳32条指令（每条指令4字节）。
    而这里原来的英文注释为“ Not more that 16 instructions here!!!”，即最大16条指令
    我认为需要进一步斟酌，到底是最大16字节，还是32字节
*/
	mtc0	zero, COP_0_STATUS_REG  // 清零cp0 status寄存器
	mtc0	zero, COP_0_CAUSE_REG   // 清零cp0 cause寄存器

    /*
    设置启动异常向量入口地址为ROM地址(0xbfc00000)
    将寄存器cp0 status的BEV置1，使CPU采用ROM(kseg1)空间的异常入口点
    */
	li	t0, SR_BOOT_EXC_VEC	/* Exception to Boostrap Location */
	mtc0	t0, COP_0_STATUS_REG
	
	la	sp, stack       // 加载栈地址
	la	gp, _gp         // 加载全局指针gp

	/* initialize spi */
	li  t0, 0xbfe80000      //地址0xbfe80000为SPI0的寄存器基地址
	li  t1, 0x17	        // div 4, fast_read + burst_en + memory_en double I/O 模式 部分SPI flash可能不支持
	sb  t1, 0x4(t0)	        // 设置寄存器sfc_param
	li  t1, 0x05
	sb  t1, 0x6(t0)         // 设置寄存器sfc_timing

	/* 设置sdram cs1复用关系，开发板使用ejtag_sel gpio_0引脚(第五复用)作为第二片sdram的片选
	  注意sw2拨码开关的设置，使用ejtag烧录pmon时需要调整拨码开关，烧录完再调整回来 */
	li		a0, 0xbfd011c0
	lw		a1, 0x40(a0)
	ori	a1, 0x01
	sw		a1, 0x40(a0)

#if defined(NAND_BOOT_EN) && defined(LS1CSOC)
#include "nand_boot.S"
	la	t0, start
	la	t1, real_bin
	sub	t1, t0	/* t1 = start - real_bin */
	li	t0, DATA_BUFF
	add	t1, t0	/* t1 = (start - real_bin) + DATA_BUFF */
	jr	t1
	nop
real_bin:
#endif

//	bal	uncached		/* Switch to uncached address space */
//	nop

    // 这里是异常向量的入口，不宜放太多代码，所以这里使用“bal	locate”跳到后面继续执行
	bal	locate			/* Get current execute address，cpu会自动将前地址(大概在0xBFC00000附近)保存到ra */
	nop
/*
uncached:
	or	ra, UNCACHED_MEMORY_ADDR
	j	ra
	nop*/

/***********************************EXC VECTOR***************************************/

/*
 *  Reboot vector usable from outside pmon.
 */
	.align	9
ext_map_and_reboot:
	li	a0,0x10000000 /*test from 0xbfcxxxxx or 0xff20xxxx */
	and	a0,ra
	bnez	a0,1f
	la	a0,_start
	li	s0,0xbfc00000
	subu	s0,a0
1:
	la	a0, v200_msg
	bal	stringserial
	nop
	b	exc_common

	.align	7			/* bfc00280 */
	la	a0, v280_msg
	bal	stringserial
	nop
	b	exc_common

/* Cache error */
	.align	8			/* bfc00300 */
	PRINTSTR("\r\nPANIC! Unexpected Cache Error exception! ")
	mfc0	a0, COP_0_CACHE_ERR
	bal	hexserial
	nop
	b	exc_common

/* General exception */
	.align	7			/* bfc00380 */
	li	a0,0x10000000 /*test from 0xbfcxxxxx or 0xff20xxxx */
	and	a0,ra
	bnez	a0,1f
	la	a0,_start
	li	s0,0xbfc00000
	subu	s0,a0
1:
	la	a0, v380_msg
	bal	stringserial
	nop
	b	exc_common
	
	.align	9			/* bfc00400 */
	la	a0, v400_msg
	bal	stringserial
	nop
	b	exc_common

/*acpi: set ddr autorefresh and suspend */
	.align	7			/* bfc00480 */
	li	t0, 0xaffffe30
	lw	t1, 0x4(t0)
	li	t2, 0x1
	or	t1, t1, t2
	sw	t1, 0x4(t0)

	li	t0, 0xbfe7c008
	lw	t1, 0x0(t0)
	ori t1, t1, 0x2000
	sw	t1, 0x0(t0)
	
	.align	8			/* bfc00500 */
exc_common:
	PRINTSTR("\r\nCAUSE=")
	mfc0	a0, COP_0_CAUSE_REG
	bal	hexserial
	nop
	PRINTSTR("\r\nSTATUS=")
	mfc0	a0, COP_0_STATUS_REG
	bal	hexserial
	nop
	PRINTSTR("\r\nERRORPC=")
	mfc0	a0, COP_0_ERROR_PC
	bal	hexserial
	nop
	PRINTSTR("\r\nEPC=")
	mfc0	a0, COP_0_EXC_PC
	bal	hexserial
	nop

	PRINTSTR("\r\nBADADDR=")
	mfc0	a0, COP_0_BAD_VADDR
	bal	hexserial
	nop

//	bal mydebug_main
	nop
1:
	b 1b
	nop


/****************************************LOCATE*********************************/

/*
 *  We get here from executing a bal to get the PC value of the current execute
 *  location into ra. Check to see if we run from ROM or if this is ramloaded.
 *  寄存器ra内保持着函数的返回地址，根据ra的值来判断当前是从ROM冷启动，还是从RAM热复位的
 *  ROM冷启动由通电引起，RAM热复位为各种异常引起，比如看门狗引起的复位等，
 *  也就是RAM热复位之前CPU已经开始运行了
 *  如果是从ROM冷启动，则寄存器ra的值为指令"bal	locate"所在位置加8字节，大概在0xBFC00000附近
 *  如果是从RAM热复位，则集成器ra的值为0x80xxxxxx
 */
locate:
//	la		s0, uncached
//	subu	s0, ra, s0
    /*
     * start.s的这段汇编程序在ROM（入口点为0xBFC00000）中运行
     * 而编译链接时指定的起始地址是0x80100000，所以需要修正一下地址
     * s0中保存着ra与start的差值，在后续的代码中可以起到修正地址的作用
     * 在看看文件开始的时候，对寄存器s0用途的描述是“         link versus load offset, used to relocate absolute adresses”
     * 除了修正地址外，还通过s0的值来判断是从ROM冷启动，还是从RAM热启动
     */
	la		s0, start           // s0 = start， 其中start的地址为编译链接时，指定的0x80010000
	subu	s0, ra, s0          // s0 = ra - s0，其中ra的值在ROM入口地址0xBFC00000附近
	and	s0, 0xffff0000          // s0 = s0 & 0xffff0000

    /*
     * 初始化cp0的status寄存器和cause寄存器
     * 在异常引起的(从RAM)热复位后，需要重新初始化cp0的status和cause，
     * 如果是从ROM冷启动的，那么前面已经初始化了，这里是再次重复初始化，没有影响的
     */
	li		t0, SR_BOOT_EXC_VEC
	mtc0	t0, COP_0_STATUS_REG        // 重新初始化cp0的status寄存器
	mtc0	zero, COP_0_CAUSE_REG       // 重新清零cp0的cause寄存器
	.set	noreorder
    
	li	t0, 0xbfe78030          // 地址0xbfe78030为PLL/SDRAM频率配置寄存器的地址
	/* 设置PLL倍频 及SDRAM分频 */
	li	t2, (0x80000008 | (PLL_MULT << 8) | (0x3 << 2) | SDRAM_DIV)
	/* 设置CPU分频 */
	li	t3, (0x00008003 | (CPU_DIV << 8))
	/* 注意：首先需要把分频使能位清零 */
	li	t1, 0x2
	sw	t1, 0x4(t0)         // 清零CPU_DIV_VALID，即disable
	sw	t2, 0x0(t0)         // 写寄存器START_FREQ
	sw	t3, 0x4(t0)         // 写寄存器CLK_DIV_PARAM
	DELAY(2000)

	/* initialize UART */
	li a0, 0
	bal	initserial          // 初始化串口
	nop
	PRINTSTR("\r\asm uart2 init ok!\r\n");  // 打印一条提示信息，表示串口初始化成功了

	/* 芯片上电默认使用gpio(输入模式）但大多时候是使用模块的功能，如lcd i2c spi ac97等
	   所以这里把gpio都关闭，方便使用模块功能。如果上电后需要gpio输出一个确定电平，
	   如继电器、LDE等，可以修改这里的代码。*/
	/* disable all gpio */
	li a0,0xbfd00000
	sw zero,0x10c0(a0)	/* disable gpio 0-31 */
	sw zero,0x10c4(a0)	/* disable gpio 32-63 */
	sw zero,0x10c8(a0)	/* disable gpio 64-95 */
	sw zero,0x10cc(a0)

	li t0, 0xffffffff
	sw t0, 0x10d0(a0)
	sw t0, 0x10d4(a0)
	sw t0, 0x10d8(a0)
	sw t0, 0x10dc(a0)

	sw t0, 0x10f0(a0)
	sw t0, 0x10f4(a0)
	sw t0, 0x10f8(a0)
	sw t0, 0x10fc(a0)

	/* lcd soft_reset and panel config & timing */
#ifdef DC_FB0
/*	li a0, 0xbc301240
	li a1, 0x00100103
	sw a1, 0x0(a0)
	li a1, 0x00000103
	sw a1, 0x0(a0)		//soft_reset
	li a1, 0x00100103
	sw a1, 0x0(a0)

	li a1, 0x80001111
	sw a1, 0x180(a0)	//panel config
	li a1, 0x33333333
	sw a1, 0x1a0(a0)*/
#endif

	li output_en, 0x1
#ifdef FAST_STARTUP
	li a1, 0x03000000
	sw a1, 0x10c4(a0)
	sw a1, 0x10d4(a0)
	lw a2, 0x10e4(a0)
	and a2, a1
	beq a2, a1, get_pin_val_finish
	nop
	li output_en, 0x1

get_pin_val_finish:

#endif

	/* Initializing. Standby... */
    /*
     *  根据s0的值判断是否为ROM冷启动
     *  如果s0不等于0，则是ROM冷启动；如果等于0，则是RAM热复位
     *  冷启动，则需要初始化内存，cache，加载代码到内存等
     */
	bnez s0, 1f     // 如果寄存器s0不等于0，则说明是ROM冷启动，则跳转到下一个标号1处进行彻底初始化
	nop
	li a0, 128
	jal main    // 热复位，则直接跳转到函数main
	nop
1:

/* use only 8wins */
#define CPU_WIN_BASE 0xbfd00000
#define CPU_WIN_MASK 0xbfd00040
#define CPU_WIN_MMAP 0xbfd00080

#define set_cpu_window(id, base, mask, mmap) \
        li      t0, CPU_WIN_BASE          ;  \
        sw      $0, 0x80+id*8(t0)         ;  \
        li      t1, base                  ;  \
        sw      t1, 0x00+id*8(t0)         ;  \
        sw      $0, 0x04+id*8(t0)         ;  \
        li      t1, mask                  ;  \
        sw      t1, 0x40+id*8(t0)         ;  \
        sw      $0, 0x44+id*8(t0)         ;  \
        li      t1, mmap                  ;  \
        sw      t1, 0x80+id*8(t0)         ;  \
        sw      $0, 0x84+id*8(t0)

/* fixup cpu window */
cpu_win_fixup:
	//
	// hit         = (paddr & mask) == (mmap & mask)
	// mapped_addr =  paddr &~mask | mmap & mask
	//
	// mmap[7] -> enable
	// mmap[5] -> block trans enable
	// mmap[4] -> cachable
	// mmap[1:0] -> destination
	//
	// NOTE: the address windows has priority, win0 > win1 > ... > win7

/*	set_cpu_window(0, 0x1c280000, 0xfff80000, 0x1c280083) // camera 512K
	set_cpu_window(1, 0x1c300000, 0xfff00000, 0x1c300081) // dc 1M
	set_cpu_window(2, 0x1fe10000, 0xffffe000, 0x1fe10082) // gmac0	8K
	set_cpu_window(3, 0x1fe10000, 0xffff0000, 0x1fe100d0) // gmac0	64K
	set_cpu_window(4, 0x1f000000, 0xff000000, 0x1f000082) // AXIMUX   16M
	set_cpu_window(5, 0x00000000, 0x00000000, 0x000000f0) // ddr 0
	set_cpu_window(6, 0x00000000, 0x00000000, 0x000000f0) // ddr 0
	set_cpu_window(7, 0x00000000, 0x00000000, 0x000000f0) // ddr 0*/

/*	set_cpu_window(0, 0x1c280000, 0xfff80000, 0x1c2800d3) // camera
//	set_cpu_window(1, 0x1fc00000, 0xfff00000, 0x1fc000f2) //
	set_cpu_window(2, 0x1c300000, 0xfff00000, 0x1c3000d1) // dc 1M
//	set_cpu_window(3, 0x1f000000, 0xff000000, 0x1f0000d2) //
	set_cpu_window(4, 0x00000000, 0x00000000, 0x000000f0)
	set_cpu_window(5, 0x00000000, 0x00000000, 0x000000f0)
	set_cpu_window(6, 0x00000000, 0x00000000, 0x000000f0) // ddr 0
	set_cpu_window(7, 0x00000000, 0x00000000, 0x000000f0) // ddr 0*/

	// after this fixup, the kernel code should be compiled with
	// uncached instruction fetch patch

	/* 配置内存 */
	li msize, MEM_SIZE    
#if !defined(NAND_BOOT_EN)

    /* 
       手册建议，先写寄存器SD_CONFIG[31:0]，然后再写寄存器的SD_CONFIG[63:32]，
       即先写低32位，再写高32位。
       写三次寄存器，最后一次将最高位置一，即使能
    */

    // 写第一次
	li  	t1, 0xbfd00410      // 寄存器SD_CONFIG[31:0]的地址为0xbfd00410
	li		a1, SD_PARA0        // 宏SD_PARA0在sdram_cfg.S中定义的
	sw		a1, 0x0(t1)         // 将宏SD_PARA0的值写入寄存器SD_CONFIG[31:0]
	li		a1, SD_PARA1
	sw		a1, 0x4(t1)         // 同理，将宏SD_PARA1的值写入寄存器SD_CONFIG[63:32]

	// 写第二次
	li		a1, SD_PARA0
	sw		a1, 0x0(t1)
	li		a1, SD_PARA1
	sw		a1, 0x4(t1)

    // 写第三次	
	li		a1, SD_PARA0
	sw		a1, 0x0(t1)
	li		a1, SD_PARA1_EN     // 使能
	sw		a1, 0x4(t1)
//	DELAY(100)
#endif

/**************************************CACHE*****************************/

#define CF_7_SE         (1 << 3)        /* Secondary cache enable */
#define CF_7_SC         (1 << 31)       /* Secondary cache not present */
#define CF_7_TE         (1 << 12)       /* Tertiary cache enable */
#define CF_7_TC         (1 << 17)       /* Tertiary cache not present */
#define CF_7_TS         (3 << 20)       /* Tertiary cache size */
#define CF_7_TS_AL      20              /* Shift to align */
#define NOP8 nop;nop;nop;nop;nop;nop;nop;nop

do_caches:
	/* Init caches... */
	li	s7, 0                   /* no L2 cache */
	li	s8, 0                   /* no L3 cache */

    bal     cache_init          // 调用汇编函数cache_init
    nop

	mfc0   a0, COP_0_CONFIG         // 将协处理器0的config寄存器的值加载到寄存器a0
	and    a0, a0, ~((1<<12) | 7)   // a0 = a0 & ~((1<<12) | 7)
	or     a0, a0, 2                // a0 |= 2
	mtc0   a0, COP_0_CONFIG         // 将寄存器a0的值写入协处理器0的config寄存器




/***********************MEMORY DEBUGGING AND COPY SELF TO RAM***********************/
//#include "newtest.32/mydebug.S"
bootnow:
	/* copy program to sdram to make copy fast */
    /* 先将执行拷贝pmon到内存任务的代码，拷贝到内存0xa0000000 */
    
    /* 先确定需要拷贝的代码段为标号121到标号122之间的代码
     * 由于链接时指定的起始地址是0x80010000，
     * 而目前正在ROM（SPI NOR FLASH，起始地址为0xBFC00000）运行
     * 所以需要用寄存器s0来修正一下地址
     */
	la		t0, 121f            // 将下一个标号121所在地址，加载到寄存器t0
	addu	t0, s0              // 使用寄存器s0修正t0中的(标号121的)地址
	la		t1, 122f            // 将下一个标号122所在地址，加载到寄存器t1
	addu	t1, s0              // 使用寄存器s0修正t1中的(标号122的)地址
	
	li		t2, 0xa0000000      // 将立即数0xa0000000（起始地址）加载到寄存器t2
1:
	lw		v0, (t0)            // 将寄存器t0所指的内存地址开始4字节的数据加载到寄存器v0
	sw		v0, (t2)            // 将寄存器v0的内容保存到寄存器t2所指的内存中
	addu	t0, 4               // 寄存器t0向后移4字节
	addu	t2, 4               // 寄存器t2向后移4字节
	ble	t0, t1, 1b              // 如果t0 <= t1，则跳转到上一个标号1处，继续拷贝后面的4字节
	nop

	li		t0, 0xa0000000      // 将立即数0xa0000000加载到寄存器t0
	jr		t0	                // 跳转到起始地址0xa0000000处开始执行（拷贝任务）
	nop		

121: 
	/* Copy PMON to execute location... */
    /* 将固件拷贝到起始地址为0xa0010000的内存空间
       由于kseg0(0x8000 0000 - 0x9FFF FFFF)和kseg1(0xA000 0000 - 0xBFFF FFFF)是映射到物理内存的相同区域
       即拷贝到0xA000 0000开始的kseg1，就相当于拷贝到0x8000 0000开始的kseg0
       这就是为什么链接时，指定的地址是0x8001 0000，而拷贝的目标起始地址是0xA001 0000
    */
	la		a0, start           // 加载符号start所在地址0x80010000加载到寄存器a0中
	addu	a1, a0, s0          // 使用寄存器s0修正寄存器a0中的地址，a1=0xBFC00000
	la		a2, _edata          // 加载_edata（链接脚本中的一个符号）到寄存器a2
	or		a0, 0xa0000000      // a0 = a0 | 0xa0000000 = 0xa0010000
	or		a2, 0xa0000000      // a2 = a2 | 0xa0000000，修正地址_edata
	subu	t1, a2, a0          // t1 = a2 - a0，即计算从start到_edata之间的长度（字节数）
	srl	t1, t1, 2               // t1 >>= 2，即t1除以4。(和前面类似，每次拷贝4字节，所以除以4)
	                            // 似乎t1计算结果没有被使用，马上就被后面的覆盖了

	move	t0, a0              // t0 = a0 = 0xa0010000 (目标起始地址)
	move	t1, a1              // t1 = a1 = 0xBFC00000 (start在ROM中的地址，源起始地址)
	move	t2, a2              // t2 = a2 (_edata在ROM中的地址，源结束地址)

	/* copy text section */
1:	and	t3, t0, 0x0000ffff      // t3 = t0 & 0x0000ffff，取低16位
	bnez	t3, 2f              // 如果t3不等于0，则跳转到下一个标号2处继续执行，t3的计算结果似乎没被使用，就被后面的覆盖了
	nop
2:	lw		t3, 0(t1)           // 从源地址t1处加载4字节到寄存器t3中
	nop
	sw		t3, 0(t0)           // 将寄存器t3中的4字节数据保存到目标地址t0处
	addu	t0, 4               // 目标地址t0后移4字节
	addu	t1, 4               // 源地址t1    后移4字节
	bne	t2, t0, 1b              // 如果t2不等于t0，则跳到上一个标号1处继续拷贝，总的来说就是判断拷贝是否结束
	nop
	/* copy text section done. */
	
	/* Clear BSS */
    /* BSS段为未初始化的全局变量的内存区间，这部分不需要从ROM中拷贝，也就不需要做地址修正 */
	la		a0, _edata          // 加载_edata的地址到寄存器a0
	la		a2, _end            // 加载_end的地址到寄存器a2
2:	sw		zero, 0(a0)         // 将寄存器a0所指的4字节清零
	bne	a2, a0, 2b              // 如果a2不等于a0，则跳到上一个标号2处，继续清零下一个4字节
	addu	a0, 4               // a0 += 4，注意，这条汇编在延迟槽内，所以仍然会被执行到
	/* Copy PMON to execute location done */

    /* 将内存大小（单位M）作为入参，放在寄存器a0中 */
	move	a0, msize           // a0 = msize（内存大小）
	srl	a0, 20                  // a0 >>= 20，将单位转换为M

    /* 调用函数main */
	la		v0, main            // 将main（）函数地址加载到寄存器v0中
	jalr	v0                  // 调用寄存器v0所指的函数
	nop

122:

stuck:
	b	stuck
	nop


/*
 * Resume the CPU state, jump to the kernel
 */
LEAF(suspend_resume)
	li	t0,	0xa01ffc00
	lw	ra,	(t0)
	lw	sp,	4(t0)
	lw	s8,	8(t0)
	lw	gp,	12(t0)
	lw	s0,	16(t0)
	lw	s1,	20(t0)
	lw	s2,	24(t0)
	lw	s3,	28(t0)
	lw	s4,	32(t0)
	lw	s5,	36(t0)
	lw	s6,	40(t0)
	lw	s7,	44(t0)

	lw	k0,	48(t0)
	lw	k1,	52(t0)

	lw	v0,	56(t0)
	lw	v1,	60(t0)

	lw	t1,	64(t0)
	mtc0	t1,	$12
	lw	t1,	68(t0)
	mtc0	t1,	$4
	lw	t1,	72(t0)
	mtc0	t1,	$5

	jr	ra
	nop
END(suspend_resume)

LEAF(suspend_save)
	li	t0,	0xa01ffc00
#	sw	ra,	(t0)
	sw	sp,	4(t0)
	sw	s8,	8(t0)
	sw	gp,	12(t0)
	sw	s0,	16(t0)
	sw	s1,	20(t0)
	sw	s2,	24(t0)
	sw	s3,	28(t0)
	sw	s4,	32(t0)
	sw	s5,	36(t0)
	sw	s6,	40(t0)
	sw	s7,	44(t0)

	sw	k0,	48(t0)
	sw	k1,	52(t0)

	sw	v0,	56(t0)
	sw	v1,	60(t0)
                                
	mfc0	t1,	$12
	sw	t1,	64(t0)
	mfc0	t1,	$4
	sw	t1,	68(t0)
	mfc0	t1,	$5
	sw	t1,	72(t0)

	jr	ra
	nop
END(suspend_save)

/***********************************FUNCTIONS**********************************/
// 打印字符串
LEAF(stringserial)
	nop
	move	a2, ra          // 将函数返回地址保存到寄存器a2中
	addu	a1, a0, s0      // 修正地址
	lbu	a0, 0(a1)           // 读取第一个字节到寄存器a0中
1:
	beqz	a0, 2f          // if (a0 == 0) 则跳转到下一个标号2处，（说明是空字符串，无需打印）直接返回
	nop
	bal	tgt_putchar         // 打印一个字符
	addiu	a1, 1           // a1 += 1，移动到下一个字节
	b	1b                  // 跳转到上一个标号1处（继续打印下一个字符），注意延迟槽中的指令也会执行
	lbu	a0, 0(a1)           // a0 = *a1，加载下一个字符到寄存器a0中，此语句位于延迟槽内

2:
	j	a2                  // 返回
	nop
END(stringserial)

LEAF(outstring)
	move	a2, ra
	move	a1, a0
	lbu	a0, 0(a1)
1:
	beqz	a0, 2f
	nop
	bal	tgt_putchar
	addiu	a1, 1
	b	1b
	lbu	a0, 0(a1)

2:
	j	a2
	nop
END(outstring)

LEAF(hexserial)
	nop
	move	a2, ra
	move	a1, a0
	li	a3, 7
1:
	rol	a0, a1, 4
	move	a1, a0
	and	a0, 0xf
	la	v0, hexchar
	addu	v0, s0
	addu	v0, a0
	bal	tgt_putchar
	lbu	a0, 0(v0)

	bnez	a3, 1b
	addu	a3, -1

	j	a2
	nop
END(hexserial)

LEAF(tgt_putchar)
	move	AT, ra
	
	la	v0, UART_BASE_ADDR          // 加载串口基地址到寄存器v0中
#ifdef	HAVE_MUT_COM
	bal	1f
	nop
	
	la	v0, COM3_BASE_ADDR
	bal	1f
	nop
	
	jr	AT
	nop
#endif
1:
	lbu	v1, LS1C_UART_LSR_OFFSET(v0)    // 将线路状态寄存器（LSR）的值加载到寄存器v1中
	and	v1, LSR_TXRDY                   // v1 &= LSR_TXRDY，取出位TFE的值，用于判断是否可以发送了
	beqz	v1, 1b                      // if (v1 == 0)，说明传输FIFO不为空，还不可以发送，则跳转到上一个标号1处，继续等待
	nop

	sb	a0, LS1C_UART_DAT_OFFSET(v0)    // 将寄存器a0的值写入数据寄存器（DAT）中，即发送
2:
	j	ra                              // 返回
	nop
END(tgt_putchar)


LEAF(initserial)
	move AT,ra                      // 把返回地址暂时保存在寄存器AT中
	
	la	v0, UART_BASE_ADDR          // 加载串口基地址到寄存器v0中
#ifdef	HAVE_MUT_COM
	bal	1f
	nop

	li	a0, 0
	la	v0, COM3_BASE_ADDR
	bal	1f
	nop

	jr	AT
	nop
#endif
1:
	li	v1, FIFO_ENABLE|FIFO_RCV_RST|FIFO_XMT_RST|FIFO_TRIGGER_4    // 清空Rx,Tx的FIFO，申请中断的trigger为4字节
	sb	v1, LS1C_UART_FCR_OFFSET(v0)        // 写FIFO控制寄存器（FCR）
	li	v1, CFCR_DLAB                       // 访问操作分频锁存器
	sb	v1, LS1C_UART_LCR_OFFSET(v0)        // 写线路控制寄存器（LCR）

	/* uart3 config mux 默认第一复用 */
#if (UART_BASE_ADDR == 0xbfe4c000)
	li		a0, 0xbfd011c4
//	lw		a1, 0x00(a0)
//	and		a1, 0xfffffff9
//	sw		a1, 0x00(a0)
	lw		a1, 0x10(a0)
	ori		a1, 0x06
	sw		a1, 0x10(a0)
//	lw		a1, 0x20(a0)
//	and		a1, 0xfffffff9
//	sw		a1, 0x20(a0)
//	lw		a1, 0x30(a0)
//	and		a1, 0xfffffff9
//	sw		a1, 0x30(a0)

/*	li		a0, 0xbfd011f0
	lw		a1, 0x00(a0)
	ori		a1, 0x03
	sw		a1, 0x00(a0)*/
#elif (UART_BASE_ADDR == 0xbfe48000)
	/* UART2 使用gpio36，gpio37的第二复用*/
	li		a0, LS1C_CBUS_FIRST1        // 加载复用寄存器CBUS_FIRST1的地址到寄存器a0
	lw		a1, 0x10(a0)                // 加载复用寄存器CBUS_SECOND1的值到寄存器a1
	ori		a1, 0x30                    // a1 |= 0x30，即GPIO36，GPIO37配置为第二复用
	sw		a1, 0x10(a0)                // 将寄存器a1的值写入寄存器CBUS_SECOND1中
#elif (UART_BASE_ADDR == 0xbfe44000)
	/* UART1 */
	li		a0, 0xbfd011f0
	lw		a1, 0x00(a0)
	ori		a1, 0x0c
	sw		a1, 0x00(a0)
#endif

    // 设置波特率
    // 计算pll频率
	li		a0, 0xbfe78030      // 0xbfe78030为PLL/SDRAM频率配置寄存器START_FREQ，将地址0xbfe78030加载到寄存器a0中
	lw		a1, 0(a0)           // 加载寄存器START_FREQ的值到寄存器a1中
	srl		a1, 8               // a1 >>= 8
	andi	a1, 0xff            // a1 &= 0xff，即a1=PLL_MULT（PLL倍频系数）
	li		a2, APB_CLK         // a2 = APB_CLK = 24Mhz（外部晶振频率）
	srl		a2, 2			    // a2 = a2 >> 2 = APB_CLK/4
	multu	a1, a2              // hilo = a1 * a2 = PLL_MULT * APB_CLK /4
	mflo	v1				    // v1 = lo，将a1 * a2的结果的低32位放到v1中，即v1为pll频率
	// 判断是否对时钟分频
	lw		a1, 4(a0)           // 加载寄存器CLK_DIV_PARAM的值到寄存器a1中
	andi	a2, a1, DIV_CPU_SEL // a2 = a1 & DIV_CPU_SEL，即读取位CPU_SEL的值，如果=1，则分频时钟；如果=0，则晶振输入时钟(bypass模式)
	bnez	a2, 1f              //if (a2 != 0) 则跳转到下一个标号1处
	nop
	li		v1, APB_CLK         // v1 = APB_CLK，即cpu时钟为晶振频率
	b		3f
	nop
1:  // 判断cpu分频系数是否有效
	andi	a2, a1, DIV_CPU_EN  // a2 = a1 & DIV_CPU_EN，即读取位CPU_DIV_EN的值，判断配置参数是否有效
	bnez	a2, 2f              // if (a2 != 0) 则跳转到下一个标号2处
	nop
	srl		v1, 1			    //v1 >>= 1，即v1 = APB_CLK/4 * PLL_MULT     / 2
	b		3f
	nop
2:  // 计算cpu频率
	andi	a1, DIV_CPU         // a1 &= DIV_CPU
	srl		a1, DIV_CPU_SHIFT   // a1 >>= DIV_CPU_SHIFT，即a1为cpu分频系数
	divu	v1, a1              // lo = v1 / a1; hi = v1 % a1
	mflo	v1				    // v1 = lo，即v1为cpu频率
3:
//	li	v1, ((APB_CLK / 4) * (PLL_MULT / CPU_DIV)) / (16*CONS_BAUD) / 2
	li		a1, 16*CONS_BAUD    // a1 = 16 * 波特率
	divu	v1, v1, a1          // v1 = v1 / a1
	srl     v1, 1               // v1 >>= 1，即v1 /= 2
	sb	v1, LS1C_UART_LSB_OFFSET(v0)    // 将低8位写入分频锁存器1
	srl	v1, 8                           // v1 >>= 8
	sb	v1, LS1C_UART_MSB_OFFSET(v0)    // 将低8位写入分频锁存器2
	
	li	v1, CFCR_8BITS                  // 8个数据位，1个停止位，无校验
	sb	v1, LS1C_UART_LCR_OFFSET(v0)    // 写线路控制寄存器（LCR）
//	li	v1, MCR_DTR|MCR_RTS             // 使能DTR和RTS
//	sb	v1, LS1C_UART_MCR_OFFSET(v0)    // 写MODEM控制寄存器（MCR）
	li	v1, 0x0                         // 关闭所有中断
	sb	v1, LS1C_UART_IER_OFFSET(v0)    // 写中断使能控制寄存器（IER）
	j   ra
	nop
END(initserial)

__main:
	j	ra
	nop


	.rdata
transmit_pat_msg:
	.asciz	"\r\nInvalid transmit pattern.  Must be DDDD or DDxDDx\r\n"
v200_msg:
	.asciz	"\r\nPANIC! Unexpected TLB refill exception!\r\n"
v280_msg:
	.asciz	"\r\nPANIC! Unexpected XTLB refill exception!\r\n"
v380_msg:
	.asciz	"\r\nPANIC! Unexpected General exception!\r\n"
v400_msg:
	.asciz	"\r\nPANIC! Unexpected Interrupt exception!\r\n"
v480_msg:
	.asciz	"\r\nPANIC! You have been in the Ejtag Debug MOde Trap is 0!\r\n"
hexchar:
	.ascii	"0123456789abcdef"

	.text
	.align	2

#define Index_Store_Tag_D			0x09
#define Index_Invalidate_I			0x00
#define Index_Writeback_Inv_D		0x01


	.ent	CPU_DisableCache
	.global CPU_DisableCache
	.set noreorder
	.set mips32
CPU_DisableCache:
	mfc0   	t0, CP0_CONFIG, 0
	and   	t0, 0xfffffff8
	or	t0, 0x2
	mtc0   	t0, CP0_CONFIG, 0
	jr	ra
   	nop
	.set reorder
	.end 	CPU_DisableCache

LEAF(nullfunction)
	jr ra
	nop
END(nullfunction)

	.ent		cache_init
	.global	cache_init
	.set		noreorder
cache_init:
	move t1, ra
####part 2####
cache_detect_4way:
	.set	mips32
	mfc0	t4, CP0_CONFIG,1        // 将cp0的config1寄存器的值加载到寄存器t4中
	lui		v0, 0x7		            // v0 = 0x7 << 16
	and		v0, t4, v0              // v0 = t4 & v0
	srl		t3, v0, 16              // t3 = v0 >> 16  Icache组相联数 IA

	li		t5, 0x800 		//32*64
	srl		v1, t4,22		//v1 = t4 >> 22
	andi	v1, 7			//Icache每路的组数 64x2^S IS
	sll		t5, v1			//InstCacheSetSize
	sll		t5, t3			//t5 InstCacheSize


	andi	v0, t4, 0x0380
	srl		t7, v0, 7		//DA

	li		t6, 0x800       // 32*64
	srl		v1, t4,13
	andi	v1, 7			//DS
	sll		t6, v1          // DataCacheSetSize
	sll		t6, t7          // t5 DataCacheSize

####part 3####
#	.set	mips3
	lui		a0, 0x8000			//a0 = 0x8000 << 16
	addu	a1, $0, t5
	addu	a2, $0, t6
cache_init_d2way:
/******************************/	//lxy
//	addiu	t3, t3, 1
//	li	t4, 0
//5:
/******************************/
// a0=0x80000000, a1=icache_size, a2=dcache_size
// a3, v0 and v1 used as local registers
	mtc0	$0, CP0_TAGHI
	addu	v0, $0, a0		//v0 = 0 + a0
	addu	v1, a0, a2		//v1 = a0 + a2
1:	slt		a3, v0, v1		//a3 = v0 < v1 ? 1 : 0
	beq		a3, $0, 1f		//if (a3 == 0) goto 1f
	nop
	mtc0	$0, CP0_TAGLO
	cache	Index_Store_Tag_D, 0x0(v0)	        // 1 way
4:	beq		$0, $0, 1b
	addiu	v0, v0, 0x20
1:
cache_flush_i2way:
	addu	v0, $0, a0
	addu	v1, a0, a1
1:	slt		a3, v0, v1
	beq		a3, $0, 1f
	nop
	cache	Index_Invalidate_I, 0x0(v0)	        // 1 way
4:	beq		$0, $0, 1b
	addiu	v0, v0, 0x20
1:
cache_flush_d2way:
	addu	v0, $0, a0
	addu	v1, a0, a2
1:	slt		a3, v0, v1
	beq		a3, $0, 1f
	nop
	cache	Index_Writeback_Inv_D, 0x0(v0) 	    // 1 way
4:	beq		$0, $0, 1b
	addiu	v0, v0, 0x20
/******************************/	//lxy
//	addiu	t4, t4, 1
//	addiu	a0, a0, 1
//	slt		t5, t4, t3
//	bne		t5, $0, 5b
//	nop
/******************************/
//	.set	mips0

1:
cache_init_finish:
	jr	t1
	nop
	.set	reorder
	.end	cache_init

    


/*
* General exception vector for all other CPUs.
*
* Be careful when changing this, it has to be at most 128 bytes
* to fit into space reserved for the exception handler.
* 
* 整个异常向量的入口处理函数
* void general_exception(void)
*/
    .set noreorder
    .globl general_exception
general_exception:
    mfc0    k1, CP0_CAUSE
    andi    k1, k1, 0x7c
# 注意，这里exception_handlers不是函数，而是地址
# 即exception_handlers+k1 = exception_handlers+ExcCode*4
# exception_handlers[]中保存了各个异常处理函数的首地址，每个占4字节
# 这里看上去很巧妙，实际上个人认为很不好理解，很容易混淆
    PTR_L   k0, exception_handlers(k1)
    jr  k0
    nop
    .set reorder


    .set noreorder
    .globl handle_int
handle_int:
    SAVE_ALL

    jal     plat_irq_dispatch
    nop
    
    RESTORE_ALL_AND_RET
    .set reorder




    .set noreorder
    .globl handle_reserved
handle_reserved:
    .set    mips3
    eret
    .set    mips0
    .set reorder


/*
 *  Modify SR value, arg 1 = set bits, arg 2 = clear bits.
 */
LEAF(CPU_SetSR)
    mfc0    v0, COP_0_STATUS_REG
    not v1, a1
    and v1, v0
    or  v1, a0
    mtc0    v1, COP_0_STATUS_REG
    NOP8
    j   ra
    nop
END(CPU_SetSR)


