#include "fifo.h"
#include "gpio.h"
#include "usart.h"

void USART_Init(struct USART *usart)
{
	IRQn_Type irqn;
	USART_TypeDef *port = usart->port;

	if(port == USART1) {
		irqn = USART1_IRQn;
		RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

	} else if(port == USART2) {
		irqn = USART2_IRQn;
		RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

	} else if(port == USART3) {
		irqn = USART3_IRQn;
		RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

	} else return;

	port->BRR = PCLK1_VALUE / usart->baudRate;
	port->CR1 |= USART_CR1_TE |
	             USART_CR1_RE |
	             USART_CR1_M0 |
	             USART_CR1_PCE |
	             USART_CR1_RXNEIE |
	             USART_CR1_TCIE;

	if(usart->swapPins)
		port->CR2 |= USART_CR2_SWAP;

	port->CR3 |= USART_CR3_OVRDIS;
	port->CR1 |= USART_CR1_UE;
	port->ICR |= USART_ICR_TCCF;

	NVIC_EnableIRQ(irqn);
	/* Low interrupt priority so it doesn't interfere with other routines */
	NVIC_SetPriority(irqn, 15);
}

int USART_Available(struct USART *usart)
{
	return FIFO_Available(usart->rxFifo);
}

void USART_Putc(struct USART *usart, unsigned char ch)
{
	FIFO_Put(usart->txFifo, ch);

	usart->port->CR1 |= USART_CR1_TXEIE;
}

void USART_Getc(struct USART *usart, unsigned char *ch)
{
	FIFO_Get(usart->rxFifo, ch);
}

void USART_Gets(struct USART *usart, unsigned char *str, int len)
{
	int available, num, i;

	available = FIFO_Available(usart->rxFifo);

	if(available >= len) num = len;
	else num = available;

	for(i = 0; i < num; ++i) {
		FIFO_Get(usart->rxFifo, str + i);
	}
}

void USART_Puts(struct USART *usart, unsigned char *str, int maxlen)
{
	int i;
	for(i = 0; i < maxlen; ++i) {
		FIFO_Put(usart->txFifo, str[i]);
	}

	usart->port->CR1 |= USART_CR1_TXEIE;
}

void USART_Flush(struct USART *usart)
{
	FIFO_Flush(usart->rxFifo);
}

void USART_ProcessCharacter(struct USART *usart)
{
	USART_TypeDef *port = usart->port;

	/* TODO Implement checks for framing error etc */
	if(port->ISR & USART_ISR_RXNE) {
		FIFO_Put(usart->rxFifo, (unsigned char)port->RDR);

	} else if(port->ISR & USART_ISR_TXE) {
		if(FIFO_Available(usart->txFifo)) {
			/* Driver enable - not using hardware control */
			GPIO_SetPin(usart->driverPort, usart->driverPin);
			FIFO_Get(usart->txFifo, (unsigned char*)&(port->TDR));
			return;

		} else {
			port->CR1 &= ~USART_CR1_TXEIE;
		}
	}

	if(port->ISR & USART_ISR_TC) {
		/* Driver disable - not using hardware control */
		GPIO_ResetPin(usart->driverPort, usart->driverPin);
		port->ICR |= USART_ICR_TCCF;
	}
}
