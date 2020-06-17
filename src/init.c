#include "gpio.h"
#include "flash.h"
#include "timer.h"
#include "sdadc.h"
#include "sensor.h"
#include "modbus.h"
#include "config.h"
#include "stepper.h"
#include "objects.h"
#include "stm32f3xx.h"
#include "controller.h"

static void sensors_init(void)
{
	SDADC_InitDma(&sdadc1);
	SDADC_InitDma(&sdadc3);

	for(int i = SensBase; i < CtrlBase; ++i) {
		struct Sensor *sensor = (struct Sensor*)functions[i];
		Sensor_Init(sensor);
	}
}

static void steppers_init(void)
{
	for(int i = ActBase; i < SensBase; ++i) {
		struct Stepper *stepper = (struct Stepper*)functions[i];

		GPIO_SetPin(stepper->stepPort, stepper->stepPin);
		GPIO_SetPin(stepper->dirPort, stepper->dirPin);
		GPIO_SetPin(stepper->enablePort, stepper->enablePin);
	}
}

static void controllers_init(void)
{
	for(int i = CtrlBase; i < FuncEnd; ++i) {
		struct Controller *controller = (struct Controller*)functions[i];

		Stepper_SetAcceleration(controller->actuator, controller->freeAccel);
		Stepper_SetSpeed(controller->actuator, 0);
	}
}

static void timers_init(void)
{
	Timer_Init(actvol0.timer);
	Timer_Init(actvol1.timer);
	Timer_Init(actframe0.timer);
	Timer_Init(actshear0.timer);
}

void Peripherals_Init(void)
{
	GPIO_Init_SDADC();
	GPIO_Init_USART2();
	GPIO_Init_Steppers();

	sensors_init();
	steppers_init();
	controllers_init();
	timers_init();

	Modbus_Init(&modbus, (unsigned char)Flash_ReadWord(0));
}

/* Called before main() from startup_stm32.s*/
void System_Init(void)
{
	/* Enable hardware FPU */
	SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));

	RCC->APB1ENR |= RCC_APB1ENR_PWREN;

	/* Configure flash latency */
	FLASH->ACR |= FLASH_ACR_PRFTBE | FLASH_ACR_LATENCY_1;
	while(!(FLASH->ACR & FLASH_ACR_PRFTBS)) ;

	/* Configure clock frequency */
	RCC->CR |= RCC_CR_HSEON;
	while(!(RCC->CR & RCC_CR_HSERDY)) ;

	RCC->CFGR  |= RCC_CFGR_PLLMUL9 | RCC_CFGR_PPRE1_DIV2 | RCC_CFGR_PLLSRC_HSE_PREDIV | RCC_CFGR_SDPRE_DIV12;
	RCC->CFGR2 |= RCC_CFGR2_PREDIV_DIV2;

	RCC->CR |= RCC_CR_PLLON;
	while(!(RCC->CR & RCC_CR_PLLRDY)) ;

	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while(!(RCC->CFGR & RCC_CFGR_SWS_PLL)) ;

	SysTick_Config(SYSCLK_VALUE / 1000U);
}
