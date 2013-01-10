/*
 * hal_clock_lpc1788.c
 *
 *  Created on: 2012-12-21
 *      Author: YangZhiyong
 */



#include <rtconfig.h>
#include "LPC177x_8x.h"
#include "system_LPC177x_8x.h"

void Clock_Init(void)
{

    /* init systick */
    SysTick_Config( SystemCoreClock/RT_TICK_PER_SECOND - 1);
    /* set pend exception priority */
    NVIC_SetPriority(PendSV_IRQn, (1<<__NVIC_PRIO_BITS) - 1);

}
