/*
 * bsp.h
 *
 *  Created on: 2013-1-10
 *      Author: YangZhiyong
 */

#ifndef BSP_H_
#define BSP_H_

#define LPC1788_SRAM_SIZE         64
#define LPC1788_SRAM_END          (0x10000000 + LPC1788_SRAM_SIZE * 1024)


void rt_hw_board_init(void);


#endif /* BSP_H_ */
