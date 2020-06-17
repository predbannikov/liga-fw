#include "fifo.h"

int FIFO_Available(struct FIFO *fifo)
{
	if(fifo->head > fifo->tail)
		return fifo->head - fifo->tail;
	else if(fifo->head < fifo->tail)
		return fifo->size - fifo->tail + fifo->head;
	else
		return 0;
}

void FIFO_Put(struct FIFO *fifo, unsigned char ch)
{
	if(FIFO_Available(fifo) < fifo->size) {
		fifo->buffer[fifo->head] = ch;
		++(fifo->head);

		if(fifo->head == fifo->size) {
			fifo->head = 0;
		}
	}
}

void FIFO_Get(struct FIFO *fifo, unsigned char *ch)
{
	if(FIFO_Available(fifo)) {
		*ch = fifo->buffer[fifo->tail];
		++(fifo->tail);

		if(fifo->tail == fifo->size) {
			fifo->tail = 0;
		}
	}
}

void FIFO_Flush(struct FIFO *fifo)
{
	fifo->head = 0;
	fifo->tail = 0;
}
