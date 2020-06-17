#ifndef GPIO_H_
#define GPIO_H_

#include "stm32f3xx.h"

void GPIO_Init_SDADC(void);
void GPIO_Init_Steppers(void);
void GPIO_Init_USART2(void);

inline void GPIO_SetPin(GPIO_TypeDef *port, unsigned int pin)
{
	port->BSRR = pin;
}

inline void GPIO_ResetPin(GPIO_TypeDef *port, unsigned int pin)
{
	port->BRR = pin;
}

inline unsigned int GPIO_ReadPin(GPIO_TypeDef *port, unsigned int pin)
{
	return port->IDR & pin;
}

#endif /* GPIO_H_ */
