#include "timer.h"

void Timer_Init(TIM_TypeDef *timer)
{
	unsigned int irqn;

	if(timer == TIM12) {
		RCC->APB1ENR |= RCC_APB1ENR_TIM12EN;
		irqn = TIM12_IRQn;

	} else if(timer == TIM13) {
		RCC->APB1ENR |= RCC_APB1ENR_TIM13EN;
		irqn = TIM13_IRQn;

	} else if(timer == TIM14) {
		RCC->APB1ENR |= RCC_APB1ENR_TIM14EN;
		irqn = TIM14_IRQn;

	} else if(timer == TIM15) {
		RCC->APB2ENR |= RCC_APB2ENR_TIM15EN;
		irqn = TIM15_IRQn;

	} else if(timer == TIM16) {
		RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;
		irqn = TIM16_IRQn;

	} else if(timer == TIM17) {
		RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;
		irqn = TIM17_IRQn;

	} else {
		return;

	}

	NVIC_EnableIRQ(irqn);

	timer->CR1   |= TIM_CR1_ARPE;
	timer->CCMR1 |= TIM_CCMR1_OC1PE;
	timer->DIER  |= TIM_DIER_CC1IE | TIM_DIER_UIE;
}

void Timer_SetPrescaler(TIM_TypeDef *timer, unsigned int prescaler)
{
	timer->PSC = prescaler - 1;
}

void Timer_Start(TIM_TypeDef *timer, unsigned int maxCounts)
{
	timer->ARR = maxCounts;
	timer->CCR1 = maxCounts / 4; /* Duty cycle = 25% */
	timer->CR1 |= TIM_CR1_CEN;
}

void Timer_EnableIsr(TIM_TypeDef *timer, int enableIsr)
{
	if(enableIsr)
		timer->DIER |= TIM_DIER_CC1IE | TIM_DIER_UIE;
	else
		timer->DIER &= ~(TIM_DIER_CC1IE | TIM_DIER_UIE);
}

void Timer_Stop(TIM_TypeDef *timer)
{
	timer->CR1 &= ~TIM_CR1_CEN;
}
