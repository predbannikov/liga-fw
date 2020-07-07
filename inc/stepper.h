#ifndef STEPPER_H_
#define STEPPER_H_

#include "stm32f3xx.h"

#define STEPPER_SLOWEST_SPEED 50

struct Stepper
{
	GPIO_TypeDef *stepPort;
	GPIO_TypeDef *dirPort;
	GPIO_TypeDef *enablePort;
	GPIO_TypeDef *hall1Port;
	GPIO_TypeDef *hall2Port;

	unsigned int stepPin;
	unsigned int dirPin;
	unsigned int enablePin;
	unsigned int hall1Pin;
	unsigned int hall2Pin;

	TIM_TypeDef *timer;

	int targetPos;
	int currentPos;

	float speed;
	float maxSpeed;
	float acceleration;
	float accelCoeff;

	float timerCountsInit;
	float timerCountsCur;
	float timerCountsMin;

	float timerFreq;
	float slowModeThresh;

	int direction;
	int state;
	int endstopEvtCount;
};

void Stepper_SetSpeed(struct Stepper *stepper, float newSpeed);
void Stepper_SetAcceleration(struct Stepper *stepper, float acceleration);

void Stepper_Move(struct Stepper *stepper, int newpos);
void Stepper_MoveTo(struct Stepper *stepper, int newpos);
void Stepper_Stop(struct Stepper *stepper);

void Stepper_Enable(struct Stepper *stepper, int enabled);

int Stepper_DistanceToGo(struct Stepper *stepper);

void Stepper_MakeStep(struct Stepper *stepper);

int Stepper_GetStatus(struct Stepper *stepper);
void Stepper_Null(struct Stepper *stepper);

inline float Stepper_GetSpeed(struct Stepper *stepper)
{
	return stepper->speed;
}

inline float Stepper_GetMaxSpeed(struct Stepper *stepper)
{
	return stepper->maxSpeed;
}

inline int Stepper_GetPosition(struct Stepper *stepper)
{
	return stepper->currentPos;
}

#endif /* STEPPER_H_ */
