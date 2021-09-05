#ifndef __USBD_CDC_IF_H__
#define __USBD_CDC_IF_H__

#include "usbd_cdc.h"


// This takes 4k of RAM.
#define NUM_RX_BUFS 8
#define RX_BUF_SIZE CDC_DATA_FS_MAX_PACKET_SIZE // Size of RX buffer item


// CDC transmit buffering
#define TX_LINBUF_SIZE 64 // Set to 64 for max single packet size
#define USBTXQUEUE_LEN 2048 // Number of bytes allocated


// Transmit buffering: circular buffer FIFO
typedef struct usbtxbuf_
{
	uint8_t data[USBTXQUEUE_LEN]; // Data buffer
	uint32_t head; // Head pointer
	uint32_t tail; // Tail pointer
} usbtx_buf_t;


// Receive buffering: circular buffer FIFO
typedef struct _usbrx_buf_
{
	// Receive buffering: circular buffer FIFO
	uint8_t buf[NUM_RX_BUFS][RX_BUF_SIZE];
	uint32_t msglen[NUM_RX_BUFS];
	uint32_t head;
	uint32_t tail;

} usbrx_buf_t;


extern USBD_CDC_ItfTypeDef USBD_Interface_fops_FS;


// Prototypes
void cdc_transmit(uint8_t* buf, uint16_t len);
void cdc_process(void);


#endif
