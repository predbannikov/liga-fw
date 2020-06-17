#ifndef SDADC_H_
#define SDADC_H_

#include "stm32f3xx.h"

#define AIN0 SDADC_JCHGR_JCHG_4
#define AIN1 SDADC_JCHGR_JCHG_5
#define AIN2 SDADC_JCHGR_JCHG_6
#define AIN3 SDADC_JCHGR_JCHG_7
#define AIN4 SDADC_JCHGR_JCHG_8
#define AIN5 SDADC_JCHGR_JCHG_7
#define AIN6 SDADC_JCHGR_JCHG_8

struct SDADC
{
	SDADC_TypeDef *sdadc;
	unsigned int numChannels;
	unsigned int activeChannels;
};

void SDADC_Init(struct SDADC *hSdadc);
void SDADC_InitDma(struct SDADC *hSdadc);

unsigned int SDADC_Read(struct SDADC *hSdadc, unsigned int channel);
unsigned int SDADC_ReadDma(struct SDADC *hSdadc, unsigned int channel);
unsigned int SDADC_ReadDmaAverage(struct SDADC *hSdadc, unsigned int channel);

#endif /* SDADC_H_ */
