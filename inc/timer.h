#ifndef TIMER_H_
#define TIMER_H_

#include "stm32f3xx.h"

#define TIMER_COUNTS_MAX 0xffff

void Timer_Init(TIM_TypeDef *timer);
void Timer_SetPrescaler(TIM_TypeDef *timer, unsigned int prescaler);
void Timer_Start(TIM_TypeDef *timer, unsigned int maxCounts);
void Timer_EnableIsr(TIM_TypeDef *timer, int enableIsr);
void Timer_Stop(TIM_TypeDef *timer);

#endif /* TIMER_H_ */
