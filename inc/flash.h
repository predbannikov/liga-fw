#ifndef FLASH_H_
#define FLASH_H_

unsigned int Flash_ReadWord(unsigned int address);
float Flash_ReadFloat(unsigned int address);
void Flash_Read(unsigned int address, unsigned int *dst, unsigned int len);

int Flash_WriteWord(unsigned int address, unsigned int value);
int Flash_Write(unsigned int address, unsigned int *data, unsigned int len);

#endif /* FLASH_H_ */
