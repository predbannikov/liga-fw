#include "logic.h"
#include "config.h"
#include "sensor.h"
#include "stepper.h"
#include "timebase.h"
#include "controller.h"

#include <math.h>

void Controller_Set(struct Controller *controller, float setPoint)
{
	controller->setPoint = setPoint;
	controller->sourceSetPoint = setPoint;
    DWT->CYCCNT = 0U; // обнуляем счётчик
	controller->state = STATE_ADJUST;
	controller->counterReadingValues = 0;
}

void Controller_SetTunings(struct Controller *controller, float kp, float ki, float kd)
{
	controller->kp = kp;
	controller->ki = ki;
	controller->kd = kd;

	if(ki == 0.0f)
		controller->iTerm = 0;
}

void Controller_Lock(struct Controller *controller, int flags)
{
	if(flags & ControllerLocked) {
		Stepper_SetAcceleration(controller->actuator, controller->lockedAccel);

		if(flags & ControllerFast)
			controller->fast = TRUE;
		else
			controller->fast = FALSE;

		controller->prevInput = 0;
		controller->active = TRUE;

		Controller_Exec(controller);

	} else {
		Stepper_SetAcceleration(controller->actuator, controller->freeAccel);
		Stepper_Stop(controller->actuator);

//		controller->active = FALSE;		// Если убрать controller->state = STATE_STOP переключение состояния, не забыть раскоментировать этот кусок
		controller->state = STATE_STOP;

	}
}

void Controller_Exec(struct Controller *controller)
{
	if(controller->active == FALSE)
		return;

	const unsigned int time = millis();
	const unsigned int deltaTime = time - controller->prevTime;

	if(deltaTime < controller->refreshPeriod) {
		return;

	} else {
		controller->prevTime = time;


		uint32_t dwt = DWT->CYCCNT;
		double sec = (double) dwt / 100000000. * 1.32;

		const float input = Sensor_Read(controller->sensor);
		const float error = controller->setPoint - input;

		const float dInput = input - controller->prevInput;


		switch (controller->state) {
		case STATE_ADJUST:
			if (input > 30) {
				DWT->CYCCNT = 0U;
				controller->state = STATE_PREP;
				Stepper_Stop(controller->actuator);
			} else {
				float speed = controller->kp * error - dInput * controller->kd;
				float clampSpeedAdjust = (float) controller->clampSpeed / 2.;
				if(fabsf(speed) > clampSpeedAdjust) {
					speed = copysignf(clampSpeedAdjust, speed);
				}
				Stepper_SetSpeed(controller->actuator, speed);
			}
			break;

		case STATE_PREP:
			if (sec > 1.0 ) {
				controller->state = STATE_START;
				controller->time = 1.f;
				DWT->CYCCNT = 0U;
				controller->k = 1.f / controller->time;
				controller->state_direction = STATE_DOWN;
			}
			break;

		case STATE_START:
			controller->counterReadingValues++;
			if(fabsf(error) < controller->maxError * 2.0f || sec > controller->time) {
				DWT->CYCCNT = 0U;
				controller->counter++;
				if (controller->counter >= 25) {
						controller->state = STATE_STOP;
				}
				if (controller->direction == 0) {
					controller->setPoint = 300;
					controller->direction = 1;
					controller->state_direction = STATE_UP;
				} else {
					controller->setPoint = controller->sourceSetPoint;
					controller->direction = 0;
					controller->state_direction = STATE_DOWN;
				}

			} else {
				controller->indexCalc = (int)(sec * controller->k * 100);
				controller->valueK = controller->table[controller->indexCalc] + 0.00001;
				float speed = 0.0f;
				float PID_kp;
				switch (controller->state_direction) {
				case STATE_DOWN:
	//				float calcError = controller->valueK * error;
	//				speed = controller->kp * calcError - dInput * controller->kd;

//					PID_kp = controller->kp;
//					PID_kp *= controller->valueK;
//					speed = PID_kp * error - dInput * controller->kd;

					speed = controller->kp * error - dInput * controller->kd;
////					speed = error - dInput;
					speed *= controller->valueK;
					break;

				case STATE_UP:
	//				float calcError = controller->valueK * error;
	//				speed = controller->kp * calcError - dInput * controller->kd;

//					PID_kp = controller->kp;
//					PID_kp *= controller->valueK;
//					speed = PID_kp * error - dInput * controller->kd;

					speed = controller->kp * error - dInput * controller->kd;
////					speed = error - dInput;
					speed *= controller->valueK;
				break;
				}

				if(fabsf(speed) > controller->clampSpeed) {
					speed = copysignf(controller->clampSpeed, speed);
				}
				Stepper_SetSpeed(controller->actuator, speed);
			}
			break;

		case STATE_STOP:
			controller->stateTest = 0;
			controller->direction = 0;
			controller->counter = 0;
			controller->active = FALSE;
			controller->counterReadingValues = 0;
			float speed = -10000000.;
			if(fabsf(speed) > controller->clampSpeed) {
				speed = copysignf(controller->clampSpeed, speed);
			}
			Stepper_SetSpeed(controller->actuator, speed);
			break;
		}

		controller->prevInput = input;
	}
}

//TODO: More advanced status
int Controller_Status(struct Controller *controller)
{
	return controller->active && !Stepper_GetMaxSpeed(controller->actuator);
}
