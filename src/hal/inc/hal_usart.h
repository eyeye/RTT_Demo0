/*
 * hal_usart.h
 *
 *  Created on: 2012-9-13
 *      Author: YangZhiyong
 */

#ifndef HAL_USART_H_
#define HAL_USART_H_

#ifdef __cplusplus
extern "C" {
#endif



#include <stdint.h>


/**
 * ��������жϷ�������
 */
typedef void (*USART_SendDoneISR)(void);

/**
 * �����ֽ�����жϷ�������
 * @param byte ���յ����ֽ�����
 */
typedef void (*USART_RecvByteDoneISR)(uint8_t byte);

/**
 * �������ݳ�ʱ( �������Ͽ�����һ��ʱ�� )�жϷ�������
 */
typedef void (*USART_RecvTimeoutISR)(void);





/**
 * ��ʼ������
 */
void USART1_Init(void);
void USART2_Init(void);
void USART3_Init(void);
void USART4_Init(void);
void USART5_Init(void);
/**
 * �ֽڷ���
 * @param byte  �����ֽ�
 */
void USART1_SendByte(uint8_t byte);
void USART2_SendByte(uint8_t byte);
void USART4_SendByte(uint8_t byte);
void USART5_SendByte(uint8_t byte);
/**
 *
 * @param buffer
 * @param length
 * @return
 */
uint32_t USART1_Send(void* buffer, uint32_t length);
uint32_t USART2_Send(void* buffer, uint32_t length);
uint32_t USART3_Send(void* buffer, uint32_t length);
uint32_t USART4_Send(void* buffer, uint32_t length);

/**
 *
 * @param buffer
 * @param max_length
 * @return
 */
uint32_t USART1_Recv(uint8_t* buffer, uint32_t max_length);
uint32_t USART2_Recv(uint8_t* buffer, uint32_t max_length);
uint32_t USART3_Recv(uint8_t* buffer, uint32_t max_length);
uint32_t USART4_Recv(uint8_t* buffer, uint32_t max_length);

uint32_t USART1_DataAvailable(void);
uint32_t USART2_DataAvailable(void);
uint32_t USART3_DataAvailable(void);
uint32_t USART4_DataAvailable(void);

/**
 *
 * @return
 */
uint32_t USART1_Flush(void);
uint32_t USART2_Flush(void);
uint32_t USART3_Flush(void);
uint32_t USART4_Flush(void);
uint32_t USART5_Flush(void);

///**
// * ���ý����ֽ�����жϷ���
// * @param isr   �жϷ�����
// */
//void USART2_SetRecvByteDoneISR   (USART_RecvByteDoneISR isr);
void USART5_SetRecvByteDoneISR    (USART_RecvByteDoneISR isr);
/**
 *
 * @param isr
 */
void USART1_SetRecvTimeoutISR    (USART_RecvTimeoutISR isr);
void USART2_SetRecvTimeoutISR    (USART_RecvTimeoutISR isr);
void USART3_SetRecvTimeoutISR    (USART_RecvTimeoutISR isr);
void USART4_SetRecvTimeoutISR    (USART_RecvTimeoutISR isr);
//void USART5_SetRecvTimeoutISR    (USART_RecvTimeoutISR isr);

/**
 *
 * @param isr
 */
void USART1_SetSendDoneISR       (USART_SendDoneISR isr);
void USART2_SetSendDoneISR       (USART_SendDoneISR isr);
void USART3_SetSendDoneISR       (USART_SendDoneISR isr);
void USART5_SetSendDoneISR       (USART_SendDoneISR isr);
void USART4_SetSendDoneISR       (USART_SendDoneISR isr);

#ifdef __cplusplus
}
#endif

#endif /* HAL_USART_H_ */



