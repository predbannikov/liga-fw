#ifndef TIMEBASE_H_
#define TIMEBASE_H_

#include "stm32f3xx.h"

extern volatile unsigned int __ticks;

void delay_ms(unsigned int delay);
void delay_micros(unsigned int delay);

inline unsigned int millis(void) {
	return __ticks;
}

inline unsigned int micros(void) {
	unsigned int ticks, counts;

	do {
		ticks = __ticks;
		counts = SysTick->VAL;
	} while(ticks != __ticks);

	return ticks * 1000U + ((SYSCLK_VALUE / 1000U) - counts) / (SYSCLK_VALUE / 1000000U);
}

#endif /* TIMEBASE_H_ */
