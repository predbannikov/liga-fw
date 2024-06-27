#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include "sensor.h"
#include "stepper.h"

enum ControllerFlags {
	ControllerLocked = 1 << 0,
	ControllerFast   = 1 << 1
};

enum STATE {STATE_ADJUST, STATE_PREP, STATE_START, STATE_STOP};
enum STATE_DIRECTION {STATE_DOWN, STATE_UP};

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

	int counter;
	int direction;
	float sourceSetPoint;
	int state;
	int counterReadingValues;
	float table[100];
	float time;
	float k;
	int indexCalc;
	int stateTest;
	float valueK;
	int state_direction;
};

void Controller_Set(struct Controller *controller, float setPoint);
void Controller_SetTunings(struct Controller *controller, float kp, float ki, float kd);
void Controller_Lock(struct Controller *controller, int flags);
void Controller_Exec(struct Controller *controller);
int  Controller_Status(struct Controller *controller);

#endif /* CONTROLLER_H_ */
