#ifndef FIFO_H_
#define FIFO_H_

struct FIFO
{
	unsigned char *buffer;
	int head, tail, size;
};

int  FIFO_Available(struct FIFO *fifo);
void FIFO_Put(struct FIFO *fifo, unsigned char ch);
void FIFO_Get(struct FIFO *fifo, unsigned char *ch);
void FIFO_Flush(struct FIFO *fifo);

#endif /* FIFO_H_ */
