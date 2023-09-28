//
// slcan: Parse incoming and generate outgoing slcan messages
//

#include "stm32g4xx_hal.h"
#include <string.h>
#include "can.h"
#include "error.h"
#include "slcan.h"
#include "usbd_cdc_if.h"


#define SLCAN_RET_OK    (uint8_t *)"\x0D"
#define SLCAN_RET_ERR   (uint8_t *)"\x07"
#define SLCAN_RET_LEN   1

// Private variables
char* fw_id = GIT_VERSION " " GIT_REMOTE "\r";


// Private methods
static uint32_t __std_dlc_code_to_hal_dlc_code(uint8_t dlc_code);
static uint8_t __hal_dlc_code_to_std_dlc_code(uint32_t hal_dlc_code);

// FIXME: Pressing enter repeats the previous TX

// Parse an incoming CAN frame into an outgoing slcan message
int32_t slcan_parse_frame(uint8_t *buf, FDCAN_RxHeaderTypeDef *frame_header, uint8_t* frame_data)
{
    // Clear buffer
    for (uint8_t j=0; j < SLCAN_MTU; j++)
        buf[j] = '\0';

    // Start building the slcan message string at idx 0 in buf[]
    uint8_t msg_idx = 0;

    // Handle classic CAN frames
    if(frame_header->FDFormat == FDCAN_CLASSIC_CAN)
    {
        // Add character for frame type
        if (frame_header->RxFrameType == FDCAN_DATA_FRAME)
        {
            buf[msg_idx] = 't';
        } else if (frame_header->RxFrameType == FDCAN_REMOTE_FRAME) {
            buf[msg_idx] = 'r';
        }
    }
    // Handle FD CAN frames
    else
    {
        // FD doesn't support remote frames so this must be a data frame

        // Frame with BRS enabled
        if(frame_header->BitRateSwitch == FDCAN_BRS_ON)
        {
            buf[msg_idx] = 'b';
        }
        // Frame with BRS disabled
        else
        {
            buf[msg_idx] = 'd';
        }
    }

    // Assume standard identifier
    uint8_t id_len = SLCAN_STD_ID_LEN;
    uint32_t tmp = frame_header->Identifier;

    // Check if extended
    if (frame_header->IdType == FDCAN_EXTENDED_ID)
    {
        // Convert first char to upper case for extended frame
        buf[msg_idx] -= 32;
        id_len = SLCAN_EXT_ID_LEN;
        tmp = frame_header->Identifier;
    }
    msg_idx++;

    // Add identifier to buffer
    for(uint8_t j = id_len; j > 0; j--)
    {
        // Add nibble to buffer
        buf[j] = (tmp & 0xF);
        tmp = tmp >> 4;
        msg_idx++;
    }

    // Add DLC to buffer
    buf[msg_idx++] = __hal_dlc_code_to_std_dlc_code(frame_header->DataLength);
    int8_t bytes = hal_dlc_code_to_bytes(frame_header->DataLength);

    // Check bytes value
    if(bytes < 0)
        return -1;
    if(bytes > 64)
        return -1;

    // Add data bytes
    for (uint8_t j = 0; j < bytes; j++)
    {
        buf[msg_idx++] = (frame_data[j] >> 4);
        buf[msg_idx++] = (frame_data[j] & 0x0F);
    }

    // Convert to ASCII (2nd character to end)
    for (uint8_t j = 1; j < msg_idx; j++)
    {
        if (buf[j] < 0xA) {
            buf[j] += 0x30;
        } else {
            buf[j] += 0x37;
        }
    }

    // Add CR for slcan EOL
    buf[msg_idx++] = '\r';

    // Return string length
    return msg_idx;
}


// Parse an incoming slcan command from the USB CDC port
int32_t slcan_parse_str(uint8_t *buf, uint8_t len)
{
    // Set default header. All values overridden below as needed.
    FDCAN_TxHeaderTypeDef frame_header =
    {
        .TxFrameType = FDCAN_DATA_FRAME,
        .FDFormat = FDCAN_CLASSIC_CAN, // default to classic frame
        .IdType = FDCAN_STANDARD_ID, // default to standard ID
        .BitRateSwitch = FDCAN_BRS_OFF, // no bitrate switch
        .ErrorStateIndicator = FDCAN_ESI_ACTIVE, // error active
        .TxEventFifoControl = FDCAN_NO_TX_EVENTS, // don't record tx events
        .MessageMarker = 0, // ?
    };
    uint8_t frame_data[64] = {0};


    // Convert from ASCII (2nd character to end)
    for (uint8_t i = 1; i < len; i++)
    {
        // Lowercase letters
        if(buf[i] >= 'a')
            buf[i] = buf[i] - 'a' + 10;
        // Uppercase letters
        else if(buf[i] >= 'A')
            buf[i] = buf[i] - 'A' + 10;
        // Numbers
        else
            buf[i] = buf[i] - '0';
    }


    // Handle each incoming command
    switch(buf[0])
    {
        // Open channel
        case 'O':
            can_enable();
            cdc_transmit(SLCAN_RET_OK, SLCAN_RET_LEN);
            return 0;

        // Close channel
        case 'C':
            can_disable();
            cdc_transmit(SLCAN_RET_OK, SLCAN_RET_LEN);
            return 0;

        // Set nominal bitrate
        case 'S':

            // Check for valid bitrate
            if(buf[1] >= CAN_BITRATE_INVALID)
            {
                cdc_transmit(SLCAN_RET_ERR, SLCAN_RET_LEN);
                return -1;
            }

            can_set_bitrate(buf[1]);
            cdc_transmit(SLCAN_RET_OK, SLCAN_RET_LEN);
            return 0;

        // Set data bitrate
        case 'Y':

            // Check for valid bitrate
            switch (buf[1]) {
                case CAN_DATA_BITRATE_1M:
                case CAN_DATA_BITRATE_2M:
                case CAN_DATA_BITRATE_4M:
                case CAN_DATA_BITRATE_5M:
                    can_set_data_bitrate(buf[1]);
                    cdc_transmit(SLCAN_RET_OK, SLCAN_RET_LEN);
                    return 0;
                default:
                    // Invalid bitrate
                    cdc_transmit(SLCAN_RET_ERR, SLCAN_RET_LEN);
                    return -1;
            }

        // FIXME: Nonstandard!
        case 'M':
            // Set mode command
            if (buf[1] == 1)
            {
                // Mode 1: silent
                can_set_silent(1);
            } else {
                // Default to normal mode
                can_set_silent(0);
            }
            cdc_transmit(SLCAN_RET_OK, SLCAN_RET_LEN);
            return 0;


        // FIXME: Nonstandard!
        case 'A':
            // Set autoretry command
            if (buf[1] == 1)
            {
                // Mode 1: autoretry enabled (default)
                can_set_autoretransmit(ENABLE);
            } else {
                // Mode 0: autoretry disabled
                can_set_autoretransmit(DISABLE);
            }
            cdc_transmit(SLCAN_RET_OK, SLCAN_RET_LEN);
            return 0;


        // FIXME: Nonstandard!
        case 'V':
        {
            // Report firmware version and remote
            cdc_transmit((uint8_t*)fw_id, strlen(fw_id));
            return 0;
        }

        // FIXME: Nonstandard!
        case 'E':
        {
            // Report error register
            char errstr[64] = {0};
            snprintf_(errstr, 64, "CANable Error Register: %X", (unsigned int)error_reg());
            cdc_transmit((uint8_t*)errstr, strlen(errstr));
            return 0;
        }
        

        // Transmit data frame command
        case 'T':
            frame_header.IdType = FDCAN_EXTENDED_ID;
            break;
        case 't':
            break;

        // Transmit remote frame command
        case 'r':
            frame_header.TxFrameType = FDCAN_REMOTE_FRAME;
            break;
        case 'R':
            frame_header.IdType = FDCAN_EXTENDED_ID;
            frame_header.TxFrameType = FDCAN_REMOTE_FRAME;
            break;



        // CANFD transmit - no BRS
        case 'd':
            frame_header.FDFormat = FDCAN_FD_CAN;
            break;
            //frame_header.BitRateSwitch = FDCAN_BRS_ON
        case 'D':
            frame_header.FDFormat = FDCAN_FD_CAN;
            frame_header.IdType = FDCAN_EXTENDED_ID;
            // Transmit CANFD frame
            break;

        // CANFD transmit - with BRS
        case 'b':
            frame_header.FDFormat = FDCAN_FD_CAN;
            frame_header.BitRateSwitch = FDCAN_BRS_ON;
            break;
            // Fallthrough
        case 'B':
            frame_header.FDFormat = FDCAN_FD_CAN;
            frame_header.BitRateSwitch = FDCAN_BRS_ON;
            frame_header.IdType = FDCAN_EXTENDED_ID;
            break;
            // Transmit CANFD frame
            break;

        case 'X':
            // TODO: Firmware update
            #warning "TODO: Implement firmware update via command"
            break;

        // Invalid command
        default:
            cdc_transmit(SLCAN_RET_ERR, SLCAN_RET_LEN);
            return -1;
    }

    // Start parsing at second byte (skip command byte)
    uint8_t parse_loc = 1;

    // Zero out identifier
    frame_header.Identifier = 0;

    // Default to standard ID
    uint8_t id_len = SLCAN_STD_ID_LEN;

    // Update length if message is extended ID
    if(frame_header.IdType == FDCAN_EXTENDED_ID)
        id_len = SLCAN_EXT_ID_LEN;

    // Iterate through ID bytes
    while(parse_loc <= id_len)
    {
        frame_header.Identifier *= 16;
        frame_header.Identifier += buf[parse_loc++];
    }

    // Attempt to parse DLC and check sanity
    uint8_t dlc_code_raw = buf[parse_loc++];

    // If dlc is too long for an FD frame
    if(frame_header.FDFormat == FDCAN_FD_CAN && dlc_code_raw > 0xF)
    {
        cdc_transmit(SLCAN_RET_ERR, SLCAN_RET_LEN);
        return -1;
    }
    if(frame_header.FDFormat == FDCAN_CLASSIC_CAN && dlc_code_raw > 0x8)
    {
        cdc_transmit(SLCAN_RET_ERR, SLCAN_RET_LEN);
        return -1;
    }

    // Set TX frame DLC according to HAL
    frame_header.DataLength = __std_dlc_code_to_hal_dlc_code(dlc_code_raw);

    // Calculate number of bytes we expect in the message
    int8_t bytes_in_msg = hal_dlc_code_to_bytes(frame_header.DataLength);

    if ((bytes_in_msg < 0) || (bytes_in_msg > 64)) {
        cdc_transmit(SLCAN_RET_ERR, SLCAN_RET_LEN);
        return -1;
    }

    // Parse data
    // TODO: Guard against walking off the end of the string!
    for (uint8_t i = 0; i < bytes_in_msg; i++)
    {
        frame_data[i] = (buf[parse_loc] << 4) + buf[parse_loc+1];
        parse_loc += 2;
    }

    // Transmit the message
    can_tx(&frame_header, frame_data);

    cdc_transmit(SLCAN_RET_OK, SLCAN_RET_LEN);
    return 0;
}


// Convert a FDCAN_data_length_code to number of bytes in a message
int8_t hal_dlc_code_to_bytes(uint32_t hal_dlc_code)
{
    switch(hal_dlc_code)
    {
        case FDCAN_DLC_BYTES_0:
            return 0;
        case FDCAN_DLC_BYTES_1:
            return 1;
        case FDCAN_DLC_BYTES_2:
            return 2;
        case FDCAN_DLC_BYTES_3:
            return 3;
        case FDCAN_DLC_BYTES_4:
            return 4;
        case FDCAN_DLC_BYTES_5:
            return 5;
        case FDCAN_DLC_BYTES_6:
            return 6;
        case FDCAN_DLC_BYTES_7:
            return 7;
        case FDCAN_DLC_BYTES_8:
            return 8;
        case FDCAN_DLC_BYTES_12:
            return 12;
        case FDCAN_DLC_BYTES_16:
            return 16;
        case FDCAN_DLC_BYTES_20:
            return 20;
        case FDCAN_DLC_BYTES_24:
            return 24;
        case FDCAN_DLC_BYTES_32:
            return 32;
        case FDCAN_DLC_BYTES_48:
            return 48;
        case FDCAN_DLC_BYTES_64:
            return 64;
        default:
            return -1;
    }
}

// Convert a standard 0-F CANFD length code to a FDCAN_data_length_code
// TODO: make this a macro
static uint32_t __std_dlc_code_to_hal_dlc_code(uint8_t dlc_code)
{
    return (uint32_t)dlc_code << 16;
}

// Convert a FDCAN_data_length_code to a standard 0-F CANFD length code
// TODO: make this a macro
static uint8_t __hal_dlc_code_to_std_dlc_code(uint32_t hal_dlc_code)
{
    return hal_dlc_code >> 16;
}


