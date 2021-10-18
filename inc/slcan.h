#ifndef _SLCAN_H
#define _SLCAN_H


// Maximum rx buffer len
#define SLCAN_MTU 138 + 1 + 16 // canfd 64 frame plus \r plus some padding
#define SLCAN_STD_ID_LEN 3
#define SLCAN_EXT_ID_LEN 8


// Prototypes
int32_t slcan_parse_frame(uint8_t *buf, FDCAN_RxHeaderTypeDef *frame_header, uint8_t* frame_data);
int32_t slcan_parse_str(uint8_t *buf, uint8_t len);

// TODO: move to helper c file
int8_t hal_dlc_code_to_bytes(uint32_t hal_dlc_code);


#endif // _SLCAN_H
