/*
 * main.c
 *
 *  Created on: 2012-12-11
 *      Author: YangZhiyong
 */



// ÖĞÎÄ×ÖÌå²âÊÔ

#include <stdint.h>
#include <rtthread.h>
#include "LPC177x_8x.h"

volatile uint8_t array[0x80];

volatile uint32_t temp32 = 0x32323232;


void test(void)
{
	uint8_t cnt = 0;
	uint8_t sum = 0;

	for( cnt = 0; cnt < 10; cnt ++)
	{
		sum += cnt;
	}
}




int main(void)
{
	volatile int count = 0;

	HAL_Init();

	while(1)
	{
		count ++;

		test();
	}

	return 0;
}



void SysTick_Handler(void)
{
	/* enter interrupt */
	rt_interrupt_enter();

	rt_tick_increase();

	/* leave interrupt */
	rt_interrupt_leave();
}



