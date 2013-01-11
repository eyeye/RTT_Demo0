/*
 * main.c
 *
 *  Created on: 2012-12-11
 *      Author: YangZhiyong
 */



#include <rtthread.h>
#include "bsp/bsp.h"
#include <rthw.h>
#include "app/app.h"
#include "bsp/driver_periph/usart/lpc1788_usart0.h"
#include "bsp/driver_periph/usart/lpc1788_usart1.h"

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define STM32_SRAM_BEGIN    (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define SRAM_BEGIN    (__segment_end("HEAP"))
#else
extern int __bss_end__;
#define SRAM_BEGIN    (&__bss_end__)
#endif


void assert_failed(rt_uint8_t* file, rt_uint32_t line)
{
	rt_kprintf("\n\r Wrong parameter value detected on\r\n");
	rt_kprintf("       file  %s\r\n", file);
	rt_kprintf("       line  %d\r\n", line);

	while (1) ;
}

/**
 * This function will startup RT-Thread RTOS.
 */
void rtthread_startup(void)
{
	/* initialize board */
	rt_hw_board_init();

#ifdef RT_USING_HEAP
    rt_system_heap_init((void*)SRAM_BEGIN, (void*)LPC1788_SRAM_END);
#endif
    rt_hw_usart0_init();
    rt_hw_usart1_init();

#ifdef RT_USING_CONSOLE
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif

	/* show version */
	rt_show_version();

	/* initialize scheduler system */
	rt_system_scheduler_init();

	/* initialize application */
	rt_application_init();

    /* initialize timer thread */
    rt_system_timer_thread_init();
	/* initialize idle thread */
	rt_thread_idle_init();

	/* start scheduler */
	rt_system_scheduler_start();

	/* never reach here */
	return ;
}


int main(void)
{
	/* disable interrupt first */
	rt_hw_interrupt_disable();

	/* startup RT-Thread RTOS */
	rtthread_startup();

	return 0;
}




