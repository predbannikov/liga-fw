#include "stm32f3xx.h"
#include "timebase.h"

volatile unsigned int __ticks;

void delay_ms(unsigned int delay)
{
	unsigned int wait = __ticks + delay;
	while(__ticks < wait) ;
}

void delay_micros(unsigned int delay)
{
	unsigned int wait = micros() + delay;
	while(micros() < wait) ;
}
