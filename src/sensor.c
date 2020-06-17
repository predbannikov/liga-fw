#include "sensor.h"
#include "sdadc.h"
#include "flash.h"

#define EEPROM_OFFSET 8
#define CAL_SIZE (sizeof(struct Calibration) / sizeof(int))

void Sensor_Init(struct Sensor *sensor)
{
	Flash_Read(sensor->id * CAL_SIZE + EEPROM_OFFSET, (unsigned int*)&(sensor->calibration), CAL_SIZE);
}

void Sensor_Null(struct Sensor *sensor)
{
	Sensor_SetOffset(sensor, 0);
	Sensor_SetOffset(sensor, Sensor_Read(sensor));
}

void Sensor_SetOffset(struct Sensor *sensor, float offset)
{
	sensor->offset = offset;
}

float Sensor_Read(struct Sensor *sensor)
{
	return (float)Sensor_ReadRaw(sensor) * sensor->calibration.slope +
			sensor->calibration.intercept - sensor->offset;
}

unsigned int Sensor_ReadRaw(struct Sensor *sensor)
{
	return SDADC_ReadDmaAverage(sensor->hSdadc, sensor->channel);
}

void Sensor_SetCalibration(struct Sensor *sensor, struct Calibration calibration)
{
	sensor->calibration = calibration;
	Flash_Write((CAL_SIZE * sensor->id) + EEPROM_OFFSET, (unsigned int*)&(sensor->calibration), CAL_SIZE);
}

void Sensor_SetCalibration2f(struct Sensor *sensor, float slope, float intercept)
{
	sensor->calibration.slope = slope;
	sensor->calibration.intercept = intercept;
	Flash_Write((CAL_SIZE * sensor->id) + EEPROM_OFFSET, (unsigned int*)&(sensor->calibration), CAL_SIZE);
}

struct Calibration Sensor_GetCalibration(struct Sensor *sensor)
{
	return sensor->calibration;
}
