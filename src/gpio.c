#include "gpio.h"

/*
 * -- List of used GPIO --
 *
 * - USART2 : PORTA -
 * PA2 .... Rx
 * PA3 .... Tx
 * PA4 .... RE/DE
 *
 * - SDADC : PORTB, PORTE -
 * PB0 .... A2
 * PB1 .... A1
 * PB2 .... A0
 * PE8 .... A4
 * PE9 .... A3
 * PB14 ... A6
 * PB15 ... A5
 *
 * - STEPPERS : PORTA, PORTB, PORTC, PORTD, PORTF -
 * PA0 .... HALL0A
 * PA1 .... HALL0B
 * PA5 .... HALL1A
 * PA6 .... HALL1B
 * PA8 .... STEP0
 * PA9 .... DIR0
 * PA10 ... EN0
 * PA11 ... STEP1
 * PA12 ... DIR1
 * PB5 .... DIR2
 * PB6 .... EN2
 * PB7 .... STEP3
 * PB8 .... DIR3
 * PB9 .... EN3
 * PC13 ... HALL3B
 * PC14 ... HALL2A
 * PC15 ... HALL2B
 * PD8 .... HALL3A
 * PF6 .... EN1
 * PF7 .... STEP2
 *
 * - JTAG/SWD -
 * PA13 ... SWDIO
 * PA14 ... SWCLK
 * PA15 ... TDI
 * PB3 .... TDO
 *
 */

//#define ENDSTOP_PULLUPS
#define ENDSTOP_PULLDOWNS
#define PRODUCTION_MODE

void GPIO_Init_SDADC(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOEEN;

	GPIOB->MODER |= GPIO_MODER_MODER0 | GPIO_MODER_MODER1  |
	                GPIO_MODER_MODER2 | GPIO_MODER_MODER14 |
	                GPIO_MODER_MODER15;
	GPIOE->MODER |= GPIO_MODER_MODER8 | GPIO_MODER_MODER9;
}

void GPIO_Init_Steppers(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN |
	               RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN |
	               RCC_AHBENR_GPIOFEN;

	/* Outputs for STEP/DIR controls */
	GPIOA->MODER |= GPIO_MODER_MODER8_0  | GPIO_MODER_MODER9_0  |
	                GPIO_MODER_MODER10_0 | GPIO_MODER_MODER11_0 |
	                GPIO_MODER_MODER12_0;

	GPIOB->MODER |= GPIO_MODER_MODER5_0 | GPIO_MODER_MODER6_0 |
	                GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 |
	                GPIO_MODER_MODER9_0;

	GPIOF->MODER |= GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0;

#ifdef PRODUCTION_MODE

	/* Open drain mode for STEP/DIR controls */
	GPIOA->OTYPER |= GPIO_OTYPER_OT_8  | GPIO_OTYPER_OT_9  |
	                 GPIO_OTYPER_OT_10 | GPIO_OTYPER_OT_11 |
	                 GPIO_OTYPER_OT_12;

	GPIOB->OTYPER |= GPIO_OTYPER_OT_5 | GPIO_OTYPER_OT_6 |
	                 GPIO_OTYPER_OT_7 | GPIO_OTYPER_OT_8 |
	                 GPIO_OTYPER_OT_9;

	GPIOF->OTYPER |= GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7;

#endif

#ifdef ENDSTOP_PULLUPS
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_0 | GPIO_PUPDR_PUPDR1_0 |
	                GPIO_PUPDR_PUPDR5_0 | GPIO_PUPDR_PUPDR6_0;

	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR13_0 | GPIO_PUPDR_PUPDR14_0 |
	                GPIO_PUPDR_PUPDR15_0;

	GPIOD->PUPDR |= GPIO_PUPDR_PUPDR8_0;
#endif

#ifdef ENDSTOP_PULLDOWNS
	GPIOA->PUPDR |= GPIO_PUPDR_PUPDR0_1 | GPIO_PUPDR_PUPDR1_1 |
	                GPIO_PUPDR_PUPDR5_1 | GPIO_PUPDR_PUPDR6_1;

	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR13_1 | GPIO_PUPDR_PUPDR14_1 |
	                GPIO_PUPDR_PUPDR15_1;

	GPIOD->PUPDR |= GPIO_PUPDR_PUPDR8_1;
#endif
}

void GPIO_Init_USART2(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	GPIOA->MODER |= GPIO_MODER_MODER2_1 |
	                GPIO_MODER_MODER3_1 |
	                GPIO_MODER_MODER4_0;

	GPIOA->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR2_1 |
	                  GPIO_OSPEEDER_OSPEEDR3_1 |
	                  GPIO_OSPEEDER_OSPEEDR4_1;

	GPIOA->AFR[0] |= (7U << GPIO_AFRL_AFRL2_Pos) |
	                 (7U << GPIO_AFRL_AFRL3_Pos);
}
