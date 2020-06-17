#ifndef LOOP_H_
#define LOOP_H_

#define MODULE_MASK   0xf000
#define OPCODE_MASK   0xff00
#define FUNCTION_MASK 0x00ff

enum Module {
	System	   = 0xf000,
	Sensor     = 0x1000,
	Stepper    = 0x2000,
	Controller = 0x3000,
};

enum Command {
	GetID          = 0xf100,
	GetNumFunc     = 0xf200,
	GetFuncType    = 0xf300,

    SoftReset	   = 0xfd00,
    HardReset	   = 0xfe00,
	Nop			   = 0xff00,

	SensorRead     = 0x1000,
	SensorReadRaw  = 0x1100,
	SensorCalRead  = 0x1200,
	SensorCalStore = 0x1300,
    SensorNull     = 0x1400,
    SensorReset    = 0x1500,

	StepperMove    = 0x2000,
	StepperMoveTo  = 0x2100,
	StepperPos     = 0x2200,
	StepperStatus  = 0x2300,
	StepperSpeed   = 0x2400,
	StepperStop    = 0x2500,
	StepperNull    = 0x2600,

    ControllerSet    = 0x3000,
    ControllerLock   = 0x3100,
    ControllerSetKp  = 0x3200,
    ControllerSetKi  = 0x3300,
    ControllerSetKd  = 0x3400,
    ControllerStatus = 0x3500,
};

void Loop_CommandExec(void);
void Loop_ControllersExec(void);

#endif /* LOOP_H_ */
