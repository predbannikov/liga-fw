#ifndef COMPTYPES_H_
#define COMPTYPES_H_

#include <stdint.h>
#include "config.h"

union Int2F {
	float f;
	unsigned int u32;
	int i32;
};

struct Calibration
{
  float slope;
  float intercept;
};

struct ModbusData {
	uint16_t command;
	uint16_t idata;
	struct Calibration caldata;
	union Int2F func[N_FUNCTIONS];
};

#endif /* COMPTYPES_H_ */
