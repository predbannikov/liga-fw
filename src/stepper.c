#include "gpio.h"
#include "timer.h"
#include "logic.h"
#include "stepper.h"
#include "timebase.h"

#include <math.h>

#define STEPPER_PRESCALER_FAST 72
#define STEPPER_PRESCALER_SLOW 65536

#define STEPPER_TIMER_FAST ((float)SYSCLK_VALUE / STEPPER_PRESCALER_FAST) /* 1 microsecond timer (1 MHz) */
#define STEPPER_TIMER_SLOW ((float)SYSCLK_VALUE / STEPPER_PRESCALER_SLOW) /* 1 millisecond-ish timer (1.1 kHz) */

#define MAX_ENDSTOP_EVENT_COUNT 10 /* Endstop noise sensitivity (less is more) */

enum Flags {
	NONE = 0,
	ENDPOINT_A = 0x1,
	ENDPOINT_B = 0x2,
};

enum Direction {
	CW,
	CCW,
};

enum State {
	IDLE,
	ZEROCROSS,
	ACCELERATING,
	DECELERATING,
	CRUISING,
	SLOWMODE,
};

/* Returns 0 if the stepper is rotating in the wrong direction */
static inline int good_direction(struct Stepper *stepper)
{
	if(stepper->direction == CW)
		return stepper->maxSpeed > 0;
	else
		return stepper->maxSpeed < 0;
}

static void update_speed(struct Stepper *stepper)
{
	switch(stepper->state) {
	case CRUISING:
	case SLOWMODE:
		return;

	case ACCELERATING:
		stepper->timerCountsCur *= 1.0f - stepper->accelCoeff * powf(stepper->timerCountsCur, 2);

		if(stepper->timerCountsCur < stepper->timerCountsMin) {
			stepper->timerCountsCur = stepper->timerCountsMin;
			stepper->state = CRUISING;
		}

		break;

	case DECELERATING:
		stepper->timerCountsCur *= 1.0f + stepper->accelCoeff * powf(stepper->timerCountsCur, 2);

		if(stepper->timerCountsCur > stepper->timerCountsMin && good_direction(stepper)) {
			stepper->timerCountsCur = stepper->timerCountsMin;
			stepper->state = CRUISING;

		} else if(stepper->timerCountsCur > stepper->timerCountsInit) {
			stepper->state = ZEROCROSS;
			return;
		}

		break;

	case IDLE:
		if(stepper->maxSpeed == 0) {
			return;
		}

	/* FALLTHRU */

	case ZEROCROSS:
		if(stepper->maxSpeed > 0) {
			stepper->direction = CW;
			GPIO_SetPin(stepper->dirPort, stepper->dirPin);

		} else if(stepper->maxSpeed < 0) {
			stepper->direction = CCW;
			GPIO_ResetPin(stepper->dirPort, stepper->dirPin);

		} else {
			Timer_Stop(stepper->timer);
			GPIO_SetPin(stepper->dirPort, stepper->dirPin); /* Reduce current through DIR optocoupler */

			stepper->state = IDLE;
			stepper->speed = 0;

			return;
		}

		/* Fast mode */
		if(fabsf(stepper->maxSpeed) > stepper->slowModeThresh) {
			stepper->timerFreq = STEPPER_TIMER_FAST;
			Timer_SetPrescaler(stepper->timer, STEPPER_PRESCALER_FAST);
			stepper->timerCountsMin = fminf(STEPPER_TIMER_FAST / fabsf(stepper->maxSpeed), TIMER_COUNTS_MAX);

			stepper->timerCountsCur = stepper->timerCountsInit;
			stepper->state = ACCELERATING;

		} else { /* Slow mode */
			stepper->timerFreq = STEPPER_TIMER_SLOW;
			Timer_SetPrescaler(stepper->timer, STEPPER_PRESCALER_SLOW);

			stepper->timerCountsMin = fminf(STEPPER_TIMER_SLOW / fabsf(stepper->maxSpeed), TIMER_COUNTS_MAX);
			stepper->timerCountsCur = stepper->timerCountsMin;
			stepper->state = SLOWMODE;
		}
	}

	Timer_Start(stepper->timer, (unsigned int)stepper->timerCountsCur);

	stepper->speed = stepper->timerFreq / stepper->timerCountsCur;
	if(stepper->direction == CCW)
		stepper->speed = copysignf(stepper->speed, -1);
}

void Stepper_SetSpeed(struct Stepper *stepper, float newSpeed)
{
	if(newSpeed == stepper->maxSpeed)
		return;
	else if(newSpeed > 0 && !GPIO_ReadPin(stepper->hall1Port, stepper->hall1Pin))
		return;
	else if(newSpeed < 0 && !GPIO_ReadPin(stepper->hall2Port, stepper->hall2Pin))
		return;

	Timer_EnableIsr(stepper->timer, FALSE);

	stepper->maxSpeed = newSpeed;

	switch(stepper->state) {
	case IDLE:
	case ZEROCROSS:
		update_speed(stepper);
		break;

	case SLOWMODE:
		stepper->state = ZEROCROSS;
		break;

	default:
		stepper->timerCountsMin = fminf(STEPPER_TIMER_FAST / fabsf(stepper->maxSpeed), TIMER_COUNTS_MAX);

		if(stepper->speed > 0)
			stepper->state = (newSpeed > stepper->speed) ? ACCELERATING : DECELERATING;
		else
			stepper->state = (newSpeed < stepper->speed) ? ACCELERATING : DECELERATING;

		break;
	}

	Timer_EnableIsr(stepper->timer, TRUE);
}

/* NOTE: The acceleration profile is used only in fast mode */
void Stepper_SetAcceleration(struct Stepper *stepper, float acceleration)
{
	stepper->acceleration = fabsf(acceleration);
	stepper->accelCoeff = stepper->acceleration / powf(STEPPER_TIMER_FAST, 2);
	stepper->timerCountsInit = STEPPER_TIMER_FAST / sqrtf(2 * stepper->acceleration);
	stepper->slowModeThresh = STEPPER_TIMER_FAST / stepper->timerCountsInit;
}

/* TODO: Move function (if needed)
void Stepper_Move(struct Stepper *stepper, int newpos)
{
	Stepper_MoveTo(stepper, stepper->currentPos + newpos);
}
*/

/* TODO: MoveTo function (if needed)
void Stepper_MoveTo(struct Stepper *stepper, int newpos)
{
}
*/

void Stepper_Stop(struct Stepper *stepper)
{
	Stepper_SetSpeed(stepper, 0);
}

void Stepper_Enable(struct Stepper *stepper, int enabled)
{
	if(enabled)
		GPIO_ResetPin(stepper->enablePort, stepper->enablePin);
	else
		GPIO_SetPin(stepper->enablePort, stepper->enablePin);
}

int Stepper_DistanceToGo(struct Stepper *stepper)
{
	return stepper->targetPos - stepper->currentPos;
}

void Stepper_MakeStep(struct Stepper *stepper)
{
	if(stepper->timer->SR & TIM_SR_UIF) {
		GPIO_ResetPin(stepper->stepPort, stepper->stepPin);

		if(stepper->direction == CW) {
			++stepper->currentPos;

			if(!GPIO_ReadPin(stepper->hall1Port, stepper->hall1Pin)) {
				if(++(stepper->endstopEvtCount) >= MAX_ENDSTOP_EVENT_COUNT) {
					stepper->endstopEvtCount = 0;
					Stepper_Stop(stepper);
				}

			} else if(stepper->endstopEvtCount > 0) {
				--stepper->endstopEvtCount;
			} else {}

		} else {
			--stepper->currentPos;

			if(!GPIO_ReadPin(stepper->hall2Port, stepper->hall2Pin)) {
				if(++(stepper->endstopEvtCount) >= MAX_ENDSTOP_EVENT_COUNT) {
					stepper->endstopEvtCount = 0;
					Stepper_Stop(stepper);
				}

			} else if(stepper->endstopEvtCount > 0) {
				--stepper->endstopEvtCount;
			} else {}
		}

	} else if(stepper->timer->SR & TIM_SR_CC1IF) {
		GPIO_SetPin(stepper->stepPort, stepper->stepPin);
		update_speed(stepper);
	}

	stepper->timer->SR &= ~(TIM_SR_UIF | TIM_SR_CC1IF);
}

int Stepper_GetStatus(struct Stepper *stepper)
{
	int status = NONE;

	if(!GPIO_ReadPin(stepper->hall1Port, stepper->hall1Pin))
		status |= ENDPOINT_A;

	if(!GPIO_ReadPin(stepper->hall2Port, stepper->hall2Pin))
		status |= ENDPOINT_B;

	return status;
}

void Stepper_Null(struct Stepper *stepper)
{
	stepper->currentPos = 0;
}
