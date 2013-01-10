/*
 * hal_spi.h
 *
 *  Created on: 2012-8-31
 *      Author: YangZhiyong
 */

#ifndef HAL_SPI_H_
#define HAL_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>





typedef void (*SPI_ISR)(void);

/**********************************************
 *
 */

void SPI1_Init(void);

void SPI1_SetRecvDoneISR(SPI_ISR isr);

uint8_t SPI1_RecvByte(void);
void SPI1_SendByte(uint8_t byte);

void SPI1_ChipSelectEn(void);
void SPI1_ChipSelectDis(void);



void SPI2_Init(void);

/**********************************************
 *
 */

void SPI2_Init(void);


uint8_t SPI2_ReadWriteByte(uint8_t TxData);

#ifdef __cplusplus
}
#endif

#endif /* HAL_SPI_H_ */



