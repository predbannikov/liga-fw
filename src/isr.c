#include "loop.h"
#include "gpio.h"
#include "usart.h"
#include "modbus.h"
#include "objects.h"
#include "stepper.h"
#include "stm32f3xx.h"

extern volatile unsigned int __ticks;

void SysTick_Handler(void)
{
	__ticks++;
}

void USART2_Handler(void)
{
	USART_ProcessCharacter(modbus.usart);
	if(Modbus_ProcessFrame(&modbus)) {
		Loop_CommandExec();
	}
}

void TIM12_Handler(void)
{
	Stepper_MakeStep(&actvol0);
}

void TIM13_Handler(void)
{
	Stepper_MakeStep(&actvol1);
}

void TIM14_Handler(void)
{
	Stepper_MakeStep(&actframe0);
}

void TIM15_Handler(void)
{
	Stepper_MakeStep(&actshear0);
}
