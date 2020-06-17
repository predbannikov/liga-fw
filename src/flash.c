#include "flash.h"
#include "stm32f3xx.h"

#include <string.h>

#define EEPROM_LENGTH 64 /* Whatever, just don't run out of memory*/
#define EEPROM_BASE  (FLASH_BASE + 0x800 * 0x7f) /* Last page of FLASH memory */
#define EEPROM_BLANK  0xffffffffU

static void flash_unlock(int unlock);
static int flash_erase(void);

static unsigned int buf[EEPROM_LENGTH]; /* Better safe than sorry */

static inline int flash_check_eop(void)
{
	if(FLASH->SR & FLASH_SR_EOP) {
		FLASH->SR |= FLASH_SR_EOP;
		return 1;
	} else
		return 0;
}

static inline void flash_wait_busy(void)
{
	while(FLASH->SR & FLASH_SR_BSY) ;
}

/* Write to flash is only allowed by half-words (16 bit) */
static inline void flash_write_helper(void *dataPtr, void *flashPtr, unsigned int len)
{
	uint16_t *dataPtrHw  = (uint16_t*)dataPtr;
	uint16_t *flashPtrHw = (uint16_t*)flashPtr;

	FLASH->CR |= FLASH_CR_PG;

	for(uint32_t i = 0; i < len * 2; ++i) {
		flashPtrHw[i] = dataPtrHw[i];
		flash_wait_busy();
		flash_check_eop();
	}

	FLASH->CR &= ~FLASH_CR_PG;
}

static inline int flash_is_blank(unsigned int *flashPtr, unsigned int len)
{
	for(unsigned int i = 0; i < len; ++i) {
		if(flashPtr[i] != EEPROM_BLANK)
			return 0;
		else
			continue;
	}

	return 1;
}

unsigned int Flash_ReadWord(unsigned int address)
{
	return ((unsigned int*)EEPROM_BASE)[address];
}

float Flash_ReadFloat(unsigned int address)
{
	return ((float*)EEPROM_BASE)[address];
}

void Flash_Read(unsigned int address, unsigned int *dst, unsigned int len)
{
	unsigned int *readAddr = (unsigned int*)EEPROM_BASE + address;
	memcpy(dst, (void*)readAddr, len * sizeof(unsigned int));
}

int Flash_WriteWord(unsigned int address, unsigned int value)
{
	return Flash_Write(address, &value, 1);
}

int Flash_Write(unsigned int address, unsigned int *data, unsigned int len)
{
	if((address + len) > EEPROM_LENGTH)
		return 0;

	unsigned int *flashPtr = (unsigned int*)EEPROM_BASE + address;

	/* Check if the required range is not already set to correct value */
	if(memcmp(data, flashPtr, len * sizeof(unsigned int)) == 0)
		return 0;

	flash_unlock(1);

	if(!flash_is_blank(flashPtr, len)) {
		memcpy(buf, (void*)EEPROM_BASE, EEPROM_LENGTH * sizeof(unsigned int));

		flash_erase();

		for(unsigned int i = 0; i < len; ++i) {
			buf[address + i] = data[i];
		}

		flash_write_helper(buf, (void*)EEPROM_BASE, EEPROM_LENGTH);
	} else {
		flash_write_helper(data, flashPtr, len);
	}

	flash_unlock(0);

	/* Read back flash contents and compare with data */
	if(memcmp(flashPtr, data, len * sizeof(unsigned int)) == 0) return 1;
	else return 0;
}

static void flash_unlock(int unlock)
{
	if(unlock != 0) {
		__disable_irq();
		FLASH->KEYR = FLASH_KEY1;
		FLASH->KEYR = FLASH_KEY2;
		__enable_irq();

	} else {
		FLASH->CR |= FLASH_CR_LOCK;
	}
}

static int flash_erase(void)
{
	FLASH->CR |= FLASH_CR_PER;
	FLASH->AR  = EEPROM_BASE;
	FLASH->CR |= FLASH_CR_STRT;

	flash_wait_busy();

	FLASH->CR &= ~FLASH_CR_PER;

	return flash_check_eop();
}
