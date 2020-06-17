#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "sensor.h"
#include "stepper.h"

enum ControllerFlags {
	ControllerLocked = 1 << 0,
	ControllerFast   = 1 << 1
};

struct Controller
{
	struct Sensor *sensor;
	struct Stepper *actuator;

	float kp;
	float ki;
	float kd;

	float prevInput;
	float iTerm;

	float setPoint;
	float maxError;

	float freeAccel;
	float lockedAccel;
	float clampSpeed;

	unsigned int refreshPeriod;
	unsigned int prevTime;

	int active;
	int fast;
};

void Controller_Set(struct Controller *controller, float setPoint);
void Controller_SetTunings(struct Controller *controller, float kp, float ki, float kd);
void Controller_Lock(struct Controller *controller, int flags);
void Controller_Exec(struct Controller *controller);
int  Controller_Status(struct Controller *controller);

#endif /* CONTROLLER_H_ */
