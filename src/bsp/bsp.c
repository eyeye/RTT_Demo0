
/*
 * bsp.c
 *
 *  Created on: 2013-1-10
 *      Author: EYE
 */

#include <rtthread.h>
#include "LPC177x_8x.h"
#include "system_LPC177x_8x.h"




void SysTick_Handler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

	rt_tick_increase();

	/* leave interrupt */
	rt_interrupt_leave();
}



void rt_hw_board_init(void)
{
    /* init systick */
    SysTick_Config( SystemCoreClock/RT_TICK_PER_SECOND - 1);
    /* set pend exception priority */
    NVIC_SetPriority(PendSV_IRQn, (1<<__NVIC_PRIO_BITS) - 1);

}
