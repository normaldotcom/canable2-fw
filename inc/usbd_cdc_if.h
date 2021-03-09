#ifndef __USBD_CDC_IF_H__
#define __USBD_CDC_IF_H__

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc.h"

extern USBD_CDC_ItfTypeDef USBD_Interface_fops_FS;

// Prototypes
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);
void cdc_process(void);


#endif
