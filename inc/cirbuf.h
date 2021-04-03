#ifndef _CIRBUF_H
#define _CIRBUF_H


// Receive buffering: circular buffer FIFO
typedef struct __cirbuf_t
{
	uint8_t head;
	uint8_t tail;
} cirbuf_t;





#endif // _CIRBUF_H
