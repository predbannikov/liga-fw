#include "loop.h"
#include "logic.h"
#include "sensor.h"
#include "modbus.h"
#include "stepper.h"
#include "objects.h"
#include "controller.h"

static inline struct Sensor *get_sensor(int index)
{
	return (struct Sensor*)functions[index];
}

static inline struct Stepper *get_stepper(int index)
{
	return (struct Stepper*)functions[index];
}

static inline struct Controller *get_controller(int index)
{
	return (struct Controller*)functions[index];
}

void Loop_CommandExec(void)
{
	const int func = modbusData.command & FUNCTION_MASK;
	const int module = modbusData.command & MODULE_MASK;
	const int opcode = modbusData.command & OPCODE_MASK;

	struct Controller *ctrl;

	switch(module) {
	case System:
		switch(opcode) {
		case Nop:
			break;

		case GetID:
			modbusData.idata = Modbus_ServerAddress(&modbus);
			break;

		case GetNumFunc:
			/* Reserved for future use */
			break;

		case GetFuncType:
			/* Reserved for future use */
			break;

		case SoftReset:
			/* Reserved for future use */
			break;

		case HardReset:
			NVIC_SystemReset();
			break;
		}

		break;

	case Sensor:
		switch(opcode) {
		case SensorRead:
			if (func == SensDef0)
			{
				ctrl = get_controller(CtrlLoad0);
				modbusData.func[func].f = ctrl->valueK * 1000.;

			} else {
				modbusData.func[func].f = Sensor_Read(get_sensor(func));
			}
			break;

		case SensorReadRaw:
			modbusData.func[func].u32 = Sensor_ReadRaw(get_sensor(func));
			break;

		case SensorCalStore:
			Sensor_SetCalibration(get_sensor(func), modbusData.caldata);
			break;

		case SensorCalRead:
			break;

		case SensorNull:
			Sensor_Null(get_sensor(func));
			break;

		case SensorReset:
			Sensor_SetOffset(get_sensor(func), 0);
			break;
		}

		break;

	case Stepper:
		switch(opcode) {
		case StepperPos:
			modbusData.func[func].i32 = Stepper_GetPosition(get_stepper(func));
			break;

		case StepperStatus:
			modbusData.func[func].i32 = Stepper_GetStatus(get_stepper(func));
			break;

		case StepperSpeed:
			Stepper_SetSpeed(get_stepper(func), modbusData.func[func].f);
			break;

		case StepperStop:
			Stepper_Stop(get_stepper(func));
			break;

		case StepperNull:
			Stepper_Null(get_stepper(func));
			break;

//		case StepperMove:
//			Stepper_Move(get_stepper(func), modbusData.func[func].i32);
//			break;
//
//		case StepperMoveTo:
//			Stepper_MoveTo(get_stepper(func), modbusData.func[func].i32);
//			break;
		}

		break;

	case Controller:
		switch(opcode) {
		case ControllerSet:
			Controller_Set(get_controller(func), modbusData.func[func].f);
			break;

		case ControllerLock:
			Controller_Lock(get_controller(func), modbusData.func[func].i32);
			break;

		case ControllerSetKp:
			ctrl = get_controller(func);
			Controller_SetTunings(ctrl, modbusData.func[func].f, ctrl->ki, ctrl->kd);
			break;

		case ControllerSetKi:
			ctrl = get_controller(func);
			Controller_SetTunings(ctrl, ctrl->kp, modbusData.func[func].f, ctrl->kd);
			break;

		case ControllerSetKd:
			ctrl = get_controller(func);
			Controller_SetTunings(ctrl, ctrl->kp, ctrl->ki, modbusData.func[func].f);
			break;

		case ControllerStatus:
			ctrl = get_controller(func);
			modbusData.func[func].i32 = Controller_Status(ctrl);
			break;
		}

		break;

	default:
		break;
	}

	modbusData.command = Nop;
}


void Loop_ControllersExec(void)
{
	Controller_Exec(get_controller(CtrlVol0));
	Controller_Exec(get_controller(CtrlVol1));
	Controller_Exec(get_controller(CtrlLoad0));
	Controller_Exec(get_controller(CtrlShear0));
}
