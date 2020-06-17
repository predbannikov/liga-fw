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

		controller->active = FALSE;
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

		const float input = Sensor_Read(controller->sensor);
		const float error = controller->setPoint - input;

		if(fabsf(error) < controller->maxError) {
			Stepper_Stop(controller->actuator);

			if(controller->fast)
				controller->fast = FALSE;

		} else if(!controller->fast && fabsf(error) < controller->maxError * 2.0f) {
			/* Enter slow mode */
			Stepper_SetSpeed(controller->actuator, copysignf(STEPPER_SLOWEST_SPEED, error));

		} else {
			const float dInput = input - controller->prevInput;

#if CONTROLLER_INTEGRAL_ENABLE
			const float speed = controller->kp * error + controller->iTerm - dInput * controller->kd;
#else
			float speed = controller->kp * error - dInput * controller->kd;
#endif
			if(fabsf(speed) > controller->clampSpeed)
				speed = copysignf(controller->clampSpeed, speed);

#if CONTROLLER_INTEGRAL_ENABLE
			else
				controller->iTerm += error * controller->ki;
#endif
			Stepper_SetSpeed(controller->actuator, speed);
		}

		controller->prevInput = input;
	}
}

//TODO: More advanced status
int Controller_Status(struct Controller *controller)
{
	return controller->active && !Stepper_GetMaxSpeed(controller->actuator);
}
