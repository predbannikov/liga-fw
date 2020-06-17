#include "usart.h"
#include "modbus.h"

#define DATA_OFFSET_FC03 3
#define DATA_OFFSET_FC06 4
#define DATA_OFFSET_FC16 7

#define TX_HEADER_SIZE 3
#define RX_HEADER_SIZE 6

#define CRC_SIZE  2
#define WORD_SIZE 2

enum ModbusCommand {
	ReadMultipleReg  = 0x03,
	WriteSingleReg   = 0x06,
	WriteMultipleReg = 0x10
};

enum ModbusOffset {
	Address = 0,
	FuncCode,
	StartAddrHi,
	StartAddrLo,
	CountHi,
	CountLo,
	NumBytes
};

static void processCode03(struct Modbus *modbus);
static void processCode06(struct Modbus *modbus);
static void processCode16(struct Modbus *modbus);

static void throwException(struct Modbus *modbus);
static int checkCRC(unsigned char *frame, int length);
static uint16_t makeCRC(unsigned char *data, int len);

void Modbus_Init(struct Modbus *modbus, unsigned char address)
{
	modbus->address = address;

	USART_Init(modbus->usart);

	modbus->usart->port->RTOR = 22;
	modbus->usart->port->CR2 |= USART_CR2_RTOEN;
	modbus->usart->port->CR1 |= USART_CR1_RTOIE;
}

int Modbus_ProcessFrame(struct Modbus *modbus)
{
	struct USART *usart = modbus->usart;
	if(usart->port->ISR & USART_ISR_RTOF) {
		usart->port->ICR |= USART_ICR_RTOCF;

		/* A frame was received */
		modbus->frameLength = USART_Available(usart);
		USART_Gets(usart, modbus->frame, modbus->frameLength);
		if(modbus->frame[Address] != modbus->address)
			return 0;
		else if(checkCRC(modbus->frame, modbus->frameLength) == 0)
			return 0;

		/* Address & CRC checks out */
		switch(modbus->frame[FuncCode]) {
		case ReadMultipleReg:
			processCode03(modbus);
			break;
		case WriteSingleReg:
			processCode06(modbus);
			break;
		case WriteMultipleReg:
			processCode16(modbus);
			break;
		default:
			throwException(modbus);
		}

		return 1;
	}
	return 0;
}

unsigned char Modbus_ServerAddress(struct Modbus *modbus)
{
	return modbus->address;
}

static void processCode03(struct Modbus *modbus)
{
	const int startaddr = modbus->frame[StartAddrLo];
	const int count = modbus->frame[CountLo];

	for(int i = 0; i < count; ++i) {
		const int srcOffset = WORD_SIZE * (i + startaddr);
		const int destOffset = (WORD_SIZE * i) + DATA_OFFSET_FC03;

		modbus->frame[destOffset] = modbus->data[srcOffset + 1];
		modbus->frame[destOffset + 1] = modbus->data[srcOffset];
	}

	modbus->frame[2] = (unsigned char)(count * WORD_SIZE);

	uint16_t crc = makeCRC(modbus->frame, TX_HEADER_SIZE + count * WORD_SIZE);

	USART_Puts(modbus->usart, modbus->frame, TX_HEADER_SIZE + count * WORD_SIZE);
	USART_Puts(modbus->usart, (unsigned char*)&crc, CRC_SIZE);
}

static void processCode06(struct Modbus *modbus)
{
	const int startaddr = modbus->frame[StartAddrLo];
	const int srcOffset = DATA_OFFSET_FC06;
	const int destOffset = WORD_SIZE * startaddr;

	modbus->data[destOffset + 1] = modbus->frame[srcOffset];
	modbus->data[destOffset] = modbus->frame[srcOffset + 1];

	uint16_t crc = makeCRC(modbus->frame, RX_HEADER_SIZE);
	USART_Puts(modbus->usart, modbus->frame, RX_HEADER_SIZE);
	USART_Puts(modbus->usart, (unsigned char*)&crc, CRC_SIZE);
}

static void processCode16(struct Modbus *modbus)
{
	const int startaddr = modbus->frame[StartAddrLo];
	const int count = modbus->frame[CountLo];

	for(int i = 0; i < count; ++i) {
		const int srcOffset = (WORD_SIZE * i) + DATA_OFFSET_FC16;
		const int destOffset = WORD_SIZE * (i + startaddr);

		modbus->data[destOffset + 1] = modbus->frame[srcOffset];
		modbus->data[destOffset] = modbus->frame[srcOffset + 1];
	}

	uint16_t crc = makeCRC(modbus->frame, RX_HEADER_SIZE);
	USART_Puts(modbus->usart, modbus->frame, RX_HEADER_SIZE);
	USART_Puts(modbus->usart, (unsigned char*)&crc, CRC_SIZE);
}

static void throwException(struct Modbus *modbus)
{
	unsigned char err[] = {modbus->address, (unsigned char)(modbus->frame[FuncCode] + 0x80), 0x01};
	uint16_t crc = makeCRC(err, TX_HEADER_SIZE);

	USART_Puts(modbus->usart, err, TX_HEADER_SIZE);
	USART_Puts(modbus->usart, (unsigned char*)&crc, CRC_SIZE);
}

static int checkCRC(unsigned char *frame, int length)
{
	const uint16_t crcCalculated = makeCRC(frame, length - CRC_SIZE);
	const uint16_t crcReceived = *(uint16_t*)(frame + length - CRC_SIZE);
	return crcCalculated == crcReceived;
}

static uint16_t makeCRC(unsigned char *data, int len)
{
	uint16_t result = 0xffff;

	if(len > 0) {
		for(int i = 0; i < len; ++i) {
			result = (uint16_t)(result ^ data[i]);
			for(int j = 0; j < 8; ++j) {
				unsigned char msb = result & 0x0001;
				result >>= 1;
				if(msb)
					result ^= 0xa001;
			}
		}
	}

	return result;
}
