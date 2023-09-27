#ifndef _CAN_H
#define _CAN_H


// Classic CAN / CANFD nominal bitrates
enum can_bitrate {
    CAN_BITRATE_10K = 0,
    CAN_BITRATE_20K,
    CAN_BITRATE_50K,
    CAN_BITRATE_100K,
    CAN_BITRATE_125K,
    CAN_BITRATE_250K,
    CAN_BITRATE_500K,
    CAN_BITRATE_750K,
    CAN_BITRATE_1000K,
	CAN_BITRATE_83_3K,

	CAN_BITRATE_INVALID,
};


// CANFD bitrates
enum can_data_bitrate {
    CAN_DATA_BITRATE_1M = 1,
    CAN_DATA_BITRATE_2M = 2,
    CAN_DATA_BITRATE_4M = 4,
    CAN_DATA_BITRATE_5M = 5,

	CAN_DATA_BITRATE_INVALID,
};


// Bus state
enum can_bus_state {
    OFF_BUS,
    ON_BUS
};


// CAN transmit buffering
#define TXQUEUE_LEN 64 // Number of buffers allocated
#define TXQUEUE_DATALEN 64 // CAN DLC length of data buffers. Must be 64 for canfd.


// Cirbuf structure for CAN TX frames
typedef struct cantxbuf_
{
	uint8_t data[TXQUEUE_LEN][TXQUEUE_DATALEN]; // Data buffer
	FDCAN_TxHeaderTypeDef header[TXQUEUE_LEN]; // Header buffer
	uint16_t head; // Head pointer
	uint16_t tail; // Tail pointer
	uint8_t full; // TODO: Set this when we are full, clear when the tail moves one.
} can_txbuf_t;


// Prototypes
void can_init(void);
void can_enable(void);
void can_disable(void);
void can_set_bitrate(enum can_bitrate bitrate);
void can_set_data_bitrate(enum can_data_bitrate bitrate);
void can_set_silent(uint8_t silent);
void can_set_autoretransmit(uint8_t autoretransmit);
uint32_t can_tx(FDCAN_TxHeaderTypeDef *tx_msg_header, uint8_t *tx_msg_data);
uint32_t can_rx(FDCAN_RxHeaderTypeDef *rx_msg_header, uint8_t *rx_msg_data);
void can_process(void);
uint8_t is_can_msg_pending(uint8_t fifo);
FDCAN_HandleTypeDef* can_gethandle(void);


#endif // _CAN_H
