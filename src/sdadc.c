#include "sdadc.h"
#include "config.h"
#include "stm32f3xx.h"

#include <stdint.h>

#define SDADC_COUNT 3
#define SDADC_MAX_CHANNELS 5

#define DMA_BUFFER_LENGTH (SDADC_MAX_CHANNELS * SDADC_AVERAGE_FACTOR)

static uint16_t dmaBuffer[SDADC_COUNT][DMA_BUFFER_LENGTH];

static inline uint16_t* get_dma_buffer(SDADC_TypeDef *sdadc)
{
	if(sdadc == SDADC1) {
		return dmaBuffer[0];
	} else if(sdadc == SDADC2) {
		return dmaBuffer[1];
	} else if(sdadc == SDADC3) {
		return dmaBuffer[2];
	}

	return 0;
}

void SDADC_Init(struct SDADC *hSdadc)
{
	SDADC_TypeDef *sdadc = hSdadc->sdadc;

	if(sdadc == SDADC1) {
		RCC->APB2ENR |= RCC_APB2ENR_SDADC1EN;
		PWR->CR |= PWR_CR_SDADC1EN;
	} else if(sdadc == SDADC2) {
		RCC->APB2ENR |= RCC_APB2ENR_SDADC2EN;
		PWR->CR |= PWR_CR_SDADC2EN;
	} else if(sdadc == SDADC3) {
		RCC->APB2ENR |= RCC_APB2ENR_SDADC3EN;
		PWR->CR |= PWR_CR_SDADC3EN;
	} else return;

	sdadc->CR2 |= SDADC_CR2_ADON;
	while(sdadc->ISR & SDADC_ISR_STABIP) ;

	sdadc->CR1 |= SDADC_CR1_INIT;
	while(!(sdadc->ISR & SDADC_ISR_INITRDY)) ;

	/* TODO separate calibration/settings for each channel */
	sdadc->CONF0R |= SDADC_CONF0R_SE0;

	sdadc->CR1 &= ~SDADC_CR1_INIT;
	while(sdadc->ISR & SDADC_ISR_INITRDY) ;

	sdadc->CR2 |= SDADC_CR2_STARTCALIB;
	while(sdadc->ISR & SDADC_ISR_CALIBIP) ;

}

void SDADC_InitDma(struct SDADC *hSdadc)
{
	/* DMA Channel configuration */
	if(!(RCC->AHBENR & RCC_AHBENR_DMA2EN)) {
		RCC->AHBENR |= RCC_AHBENR_DMA2EN;
	}

	SDADC_TypeDef *sdadc = hSdadc->sdadc;
	DMA_Channel_TypeDef *dma;
	uint16_t *dmaBufPtr;

	if(sdadc == SDADC1) {
		dma = DMA2_Channel3;
		dmaBufPtr = dmaBuffer[0];
	} else if(sdadc == SDADC2) {
		dma = DMA2_Channel4;
		dmaBufPtr = dmaBuffer[1];
	} else if(sdadc == SDADC3) {
		dma = DMA2_Channel5;
		dmaBufPtr = dmaBuffer[2];
	} else {
		dma = 0;
		dmaBufPtr = 0;
	}

	dma->CPAR = (unsigned int)&(sdadc->JDATAR);
	dma->CMAR = (unsigned int)dmaBufPtr;
	dma->CCR |= DMA_CCR_MINC | DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0 | DMA_CCR_CIRC;

	dma->CNDTR = SDADC_AVERAGE_FACTOR * hSdadc->numChannels;

	dma->CCR |= DMA_CCR_EN;

	/* SDADC configuration */
	SDADC_Init(hSdadc);

	sdadc->JCHGR = hSdadc->activeChannels;

	/* TODO Maybe use timers instead of continuous mode */
	sdadc->CR1 |= SDADC_CR1_JDMAEN;
	sdadc->CR2 |= SDADC_CR2_JSWSTART | SDADC_CR2_JCONT;
}

unsigned int SDADC_Read(struct SDADC *hSdadc, unsigned int channel)
{
	SDADC_TypeDef *sdadc = hSdadc->sdadc;

	sdadc->CR2 &= ~SDADC_CR2_RCH_Msk;
	sdadc->CR2 |= channel << SDADC_CR2_RCH_Pos;

	sdadc->CR2 |= SDADC_CR2_RSWSTART;
	while(!(sdadc->ISR & SDADC_ISR_REOCF)) ;

	return sdadc->RDATAR;
}

unsigned int SDADC_ReadDma(struct SDADC *hSdadc, unsigned int channel)
{
	const uint16_t *dmaBuf = get_dma_buffer(hSdadc->sdadc);
	return (uint16_t)(dmaBuf[channel] + 0x8000U);
}

unsigned int SDADC_ReadDmaAverage(struct SDADC *hSdadc, unsigned int channel)
{
	const uint16_t *dmaBuf = get_dma_buffer(hSdadc->sdadc);
	unsigned int sum = 0;

	for(unsigned int i = 0; i < SDADC_AVERAGE_FACTOR; ++i) {
		const unsigned int index = i * (hSdadc->numChannels) + channel;
		sum += dmaBuf[index];
	}

	return (uint16_t)(sum / SDADC_AVERAGE_FACTOR + 0x8000U);
}
