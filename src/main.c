#include "loop.h"
#include "init.h"
#include "stm32f3xx.h"

int main(void)
{
	Peripherals_Init();

	/* Main loop goes here */
	for(;;) {
		/* TODO Maybe move it into timer ISR */
		Loop_ControllersExec();
	}
}
