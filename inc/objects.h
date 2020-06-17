#ifndef OBJECTS_H_
#define OBJECTS_H_

#include "comptypes.h"

enum Function {
	ActBase   = 0,
	ActVol0   = 0,
	ActVol1   = 1,
	ActLoad0  = 2,
	ActShear0 = 3,

	SensBase  = 4,
	SensPrs0  = 4,
	SensPrs1  = 5,
	SensPrs2  = 6,
	SensLoad0 = 7,
	SensLoad1 = 8,
	SensDef0  = 9,
	SensDef1  = 10,

	CtrlBase   = 11,
	CtrlVol0   = 11,
	CtrlVol1   = 12,
	CtrlLoad0  = 13,
	CtrlShear0 = 14,

	FuncEnd    = 15,
};

extern struct ModbusData modbusData;
extern struct Modbus modbus;
extern void *functions[];

extern struct SDADC sdadc1;
extern struct SDADC sdadc3;

extern struct Stepper actvol0;
extern struct Stepper actvol1;
extern struct Stepper actframe0;
extern struct Stepper actshear0;

#endif /* OBJECTS_H_ */
