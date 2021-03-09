#include "usbd_cdc_if.h"
#include "slcan.h"

// This takes 4k of RAM.
#define NUM_RX_BUFS 8
#define RX_BUF_SIZE 512 // I think this must be 512k to match usbd_conf.c's LL RX
static volatile uint8_t rxbuf[NUM_RX_BUFS][RX_BUF_SIZE];
static volatile uint32_t rxbuf_msglen[NUM_RX_BUFS];
static volatile uint8_t rxbuf_head = 0;
static volatile uint8_t rxbuf_tail = 0;

// TXBuf pingpong
#define NUM_TX_BUFS 2
#define TX_BUF_SIZE  256
static uint8_t txbuf[NUM_TX_BUFS][TX_BUF_SIZE];
static uint8_t txbuf_selected = 0;


// Externs
extern USBD_HandleTypeDef hUsbDeviceFS;


// Private prototypes
static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);
void cdc_process(void);


USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, txbuf[txbuf_selected], 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, rxbuf[rxbuf_head]);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:

    break;

    case CDC_GET_LINE_CODING:
        pbuf[0] = (uint8_t)(115200);
	pbuf[1] = (uint8_t)(115200 >> 8);
	pbuf[2] = (uint8_t)(115200 >> 16);
	pbuf[3] = (uint8_t)(115200 >> 24);
	pbuf[4] = 0; // stop bits (1)
	pbuf[5] = 0; // parity (none)
	pbuf[6] = 8; // number of bits (8)
        break;

    case CDC_SET_CONTROL_LINE_STATE:

    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will block any OUT packet reception on USB endpoint
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result
  *         in receiving more data while previous ones are still not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */


static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
	// TODO: Put bytes in circular buffer that is processed
	// in the main loop. Ensure that frames are processed
	// in-order.

	// TODO: Maybe just setrxbuffer to a different buf. Pingpong or
	// just use that as the queue. need to respect that this is an ISR,
	// use head/tail pointer.

	// Save off length
	rxbuf_msglen[rxbuf_head] = *Len;
	rxbuf_head = (rxbuf_head + 1) % NUM_RX_BUFS;

	// Start listening on next buffer. Previous buffer will be processed in main loop.
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, rxbuf[rxbuf_head]);
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);

}



uint8_t slcan_str[SLCAN_MTU];
uint8_t slcan_str_index = 0;

void cdc_process(void)
{
	// TODO: Check for buffer overflow

	if(rxbuf_tail != rxbuf_head)
	{
		//  Process one whole buffer
		for (uint32_t i = 0; i < rxbuf_msglen[rxbuf_tail]; i++)
		{
		   if (rxbuf[rxbuf_tail][i] == '\r')
		   {
			   int8_t result = slcan_parse_str(slcan_str, slcan_str_index);

			   // Success
			   //if(result == 0)
			   //    CDC_Transmit_FS("\n", 1);
			   // Failure
			   //else
			   //    CDC_Transmit_FS("\a", 1);

			   slcan_str_index = 0;
		   }
		   else
		   {
			   slcan_str[slcan_str_index++] = rxbuf[rxbuf_tail][i];
		   }
		}

		// Move on to next buffer
		rxbuf_tail = (rxbuf_tail + 1) % NUM_RX_BUFS;
	}
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
    uint8_t result = USBD_OK;

    // NEW: Check if port is busy
//    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
//    if (hcdc->TxState != 0){
//      return USBD_BUSY;
//    }
    
    if(Len > TX_BUF_SIZE)
    {
    	return 0;
    }

    // Copy data into buffer
    for (uint32_t i=0; i < Len; i++)
    {
    	txbuf[txbuf_selected][i] = Buf[i];
    }

    // Set transmit buffer and start TX
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, txbuf[txbuf_selected], Len);
    result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);

    txbuf_selected = !txbuf_selected;

/*
    for (i = 0; i < 1 + (Len/8); i++) {
	USBD_CDC_SetTxBuffer(hUsbDeviceFS, UserTxBufferFS + (8*i), 8);
	do {
	    result = USBD_CDC_TransmitPacket(hUsbDeviceFS);
	} while (result != HAL_BUSY);
    }
*/
    /* USER CODE END 8 */
    return result;
}
