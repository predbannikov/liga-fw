#include "fifo.h"
#include "sdadc.h"
#include "usart.h"
#include "logic.h"
#include "config.h"
#include "sensor.h"
#include "modbus.h"
#include "stepper.h"
#include "objects.h"
#include "controller.h"

#include "stm32f3xx.h"

#define RESERVED 0

/* Modbus data structure */
struct ModbusData modbusData;

/* FIFO buffers */
static unsigned char txBuffer[MODBUS_FRAME_LENGTH];
static unsigned char rxBuffer[MODBUS_FRAME_LENGTH];
static unsigned char frameBuffer[MODBUS_FRAME_LENGTH];

/* USART FIFOs */
static struct FIFO txFifo = {
	.buffer = txBuffer,
	.size = MODBUS_FRAME_LENGTH
};

static struct FIFO rxFifo = {
	.buffer = rxBuffer,
	.size = MODBUS_FRAME_LENGTH
};

/* USART2 */
static struct USART usart2 = {
	.port = USART2,
	.txFifo = &txFifo,
	.rxFifo = &rxFifo,
	.driverPort = GPIOA,
	.driverPin = GPIO_BSRR_BS_4,
	.swapPins = TRUE,
	.baudRate = USART_BAUDRATE
};

/* Modbus server */
struct Modbus modbus = {
	.usart = &usart2,
	.data = (unsigned char*)&modbusData,
	.dataLength = sizeof(modbusData),
	.frame = frameBuffer,
	.maxFrameLength = MODBUS_FRAME_LENGTH
};

/* SDADC objects */
struct SDADC sdadc1 = {
	.sdadc          = SDADC1,
	.numChannels    = 5,
	.activeChannels = AIN0 | AIN1 | AIN2 | AIN3 | AIN4
};

struct SDADC sdadc3 = {
	.sdadc          = SDADC3,
	.numChannels    = 2,
	.activeChannels = AIN5 | AIN6
};

/* Sensor objects */
struct Sensor senspress0 = {
	.hSdadc  = &sdadc1,
	.channel = 0,
	.id      = SensPrs0
};

struct Sensor senspress1 = {
	.hSdadc  = &sdadc1,
	.channel = 1,
	.id      = SensPrs1
};

struct Sensor senspress2 = {
	.hSdadc  = &sdadc1,
	.channel = 2,
	.id      = SensPrs2
};

struct Sensor sensload0 = {
	.hSdadc  = &sdadc1,
	.channel = 3,
	.id      = SensLoad0
};

struct Sensor sensload1 = {
	.hSdadc  = &sdadc1,
	.channel = 4,
	.id      = SensLoad1
};

struct Sensor sensdeform0 = {
	.hSdadc  = &sdadc3,
	.channel = 0,
	.id      = SensDef0
};

struct Sensor sensdeform1 = {
	.hSdadc  = &sdadc3,
	.channel = 1,
	.id      = SensDef1
};

/* Stepper objects */
struct Stepper actvol0 = {
	.stepPort   = GPIOA,
	.dirPort    = GPIOA,
	.enablePort = GPIOA,
	.hall1Port  = GPIOA,
	.hall2Port  = GPIOA,

	.stepPin   = GPIO_BSRR_BS_8,
	.dirPin    = GPIO_BSRR_BS_9,
	.enablePin = GPIO_BSRR_BS_10,
	.hall1Pin  = GPIO_IDR_0,
	.hall2Pin  = GPIO_IDR_1,

	.timer = TIM12
};

struct Stepper actvol1 = {
	.stepPort   = GPIOA,
	.dirPort    = GPIOA,
	.enablePort = GPIOF,
	.hall1Port  = GPIOA,
	.hall2Port  = GPIOA,

	.stepPin   = GPIO_BSRR_BS_11,
	.dirPin    = GPIO_BSRR_BS_12,
	.enablePin = GPIO_BSRR_BS_6,
	.hall1Pin  = GPIO_IDR_5,
	.hall2Pin  = GPIO_IDR_6,

	.timer = TIM13
};

struct Stepper actframe0 = {
	.stepPort = GPIOF,
	.dirPort = GPIOB,
	.enablePort = GPIOB,
	.hall1Port = GPIOC,
	.hall2Port = GPIOC,

	.stepPin = GPIO_BSRR_BS_7,
	.dirPin = GPIO_BSRR_BS_5,
	.enablePin = GPIO_BSRR_BS_6,
	.hall1Pin = GPIO_IDR_14,
	.hall2Pin = GPIO_IDR_15,

	.timer = TIM14
};

struct Stepper actshear0 = {
	.stepPort = GPIOB,
	.dirPort = GPIOB,
	.enablePort = GPIOB,
	.hall1Port = GPIOD,
	.hall2Port = GPIOC,

	.stepPin = GPIO_BSRR_BS_7,
	.dirPin = GPIO_BSRR_BS_8,
	.enablePin = GPIO_BSRR_BS_9,
	.hall1Pin = GPIO_IDR_8,
	.hall2Pin = GPIO_IDR_13,

	.timer = TIM15
};

/* Controller objects */
struct Controller volumeter0 = {
	.sensor = &senspress0,
	.actuator = &actvol0,

	.kp = VOLUM_CTRL_KP,
	.kd = VOLUM_CTRL_KD,

	.maxError = VOLUM_ERROR_MARGIN,
	.freeAccel = VOLUM_ACCEL_FREE,
	.lockedAccel = VOLUM_ACCEL_LOCKED,

	.clampSpeed = VOLUM_CLAMP_SPEED,
	.refreshPeriod = CONTROLLER_REFRESH_PERIOD
};

struct Controller volumeter1 = {
	.sensor = &senspress1,
	.actuator = &actvol1,

	.kp = VOLUM_CTRL_KP,
	.kd = VOLUM_CTRL_KD,

	.maxError = VOLUM_ERROR_MARGIN,
	.freeAccel = VOLUM_ACCEL_FREE,
	.lockedAccel = VOLUM_ACCEL_LOCKED,

	.clampSpeed = VOLUM_CLAMP_SPEED,
	.refreshPeriod = CONTROLLER_REFRESH_PERIOD
};

struct Controller loadframe0 = {
	.sensor = &sensload0,
	.actuator = &actframe0,

	.kp = LOAD_CTRL_KP,
	.kd = LOAD_CTRL_KD,

	.maxError = LOAD_ERROR_MARGIN,
	.freeAccel = LOAD_ACCEL_FREE,
	.lockedAccel = LOAD_ACCEL_LOCKED,

	.clampSpeed = LOAD_CLAMP_SPEED,
	.refreshPeriod = CONTROLLER_REFRESH_PERIOD
};

struct Controller sheardevice0 = {
	.sensor = &sensload1,
	.actuator = &actshear0,

	.kp = SHEAR_CTRL_KP,
	.kd = SHEAR_CTRL_KD,

	.maxError = LOAD_ERROR_MARGIN,
	.freeAccel = LOAD_ACCEL_FREE,
	.lockedAccel = LOAD_ACCEL_LOCKED,

	.clampSpeed = LOAD_CLAMP_SPEED,
	.refreshPeriod = CONTROLLER_REFRESH_PERIOD
};

/* Function table */
void *functions[N_FUNCTIONS] = {
	[ActVol0]   = &actvol0,
	[ActVol1]   = &actvol1,
	[ActLoad0]  = &actframe0,
	[ActShear0] = &actshear0,

	[SensPrs0]  = &senspress0,
	[SensPrs1]  = &senspress1,
	[SensPrs2]  = &senspress2,
	[SensLoad0] = &sensload0,
	[SensLoad1] = &sensload1,
	[SensDef0]  = &sensdeform0,
	[SensDef1]  = &sensdeform1,

	[CtrlVol0]   = &volumeter0,
	[CtrlVol1]   = &volumeter1,
	[CtrlLoad0]  = &loadframe0,
	[CtrlShear0] = &sheardevice0,
};
