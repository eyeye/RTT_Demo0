/*
 * hal_capture.h
 *
 *  Created on: 2012-9-18
 *      Author: YangZhiyong
 */

#ifndef HAL_CAPTURE_H_
#define HAL_CAPTURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>


typedef void (*CAPTURE_PWM_ISR)(uint32_t period, uint32_t pluse);

void Capture_TIM3_Init(void);
void Capture_TIM4_Init(void);
void Capture_TIM8_Init(void);

void Capture_Set_TIM3_PwmInputISR(uint8_t channel, CAPTURE_PWM_ISR isr);
void Capture_Set_TIM4_PwmInputISR(uint8_t channel, CAPTURE_PWM_ISR isr);
void Capture_Set_TIM8_PwmInputISR(uint8_t channel, CAPTURE_PWM_ISR isr);


#ifdef __cplusplus
}
#endif

#endif /* HAL_CAPTURE_H_ */


