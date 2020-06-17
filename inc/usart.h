#ifndef USART_H_
#define USART_H_

#include "stm32f3xx.h"

#define PCLK1_VALUE (SYSCLK_VALUE / 2)

struct USART
{
	USART_TypeDef *port;
	struct FIFO *txFifo;
	struct FIFO *rxFifo;

	GPIO_TypeDef *driverPort;
	unsigned int driverPin;

	unsigned int baudRate;
	/* Needed due to PCB layout */
	int swapPins;
};

void USART_Init(struct USART *usart);

int USART_Available(struct USART *usart);

void USART_Putc(struct USART *usart, unsigned char ch);
void USART_Getc(struct USART *usart, unsigned char *ch);

void USART_Gets(struct USART *usart, unsigned char *str, int len);
void USART_Puts(struct USART *usart, unsigned char *str, int maxlen);

void USART_Flush(struct USART *usart);

void USART_ProcessCharacter(struct USART*);

#endif /* USART_H_ */
