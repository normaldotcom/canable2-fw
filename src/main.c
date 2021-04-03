//
// CANable Flex firmware
//

#include "stm32g4xx.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "can.h"
#include "slcan.h"
#include "system.h"
#include "led.h"


int main(void)
{
    // Initialize peripherals
    system_init();
    can_init();
    led_init();
    usb_init();

    // Poweron blink sequence
    led_blue_blink(2);

    // Storage for status and received message buffer
    FDCAN_RxHeaderTypeDef rx_msg_header;
    uint8_t rx_msg_data[8] = {0}; // FIXME upsize for FD
    uint8_t msg_buf[SLCAN_MTU];


    while(1)
    {
        led_process();
        can_process();
        cdc_process();

        // Message has been received, pull it from the buffer
        if(is_can_msg_pending(FDCAN_RX_FIFO0))
        {
			// If message received from bus, parse the frame
			if (can_rx(&rx_msg_header, rx_msg_data) == HAL_OK)
			{
				uint16_t msg_len = slcan_parse_frame((uint8_t *)&msg_buf, &rx_msg_header, rx_msg_data);

				// Transmit message via USB-CDC
				if(msg_len>0)
				{
					cdc_transmit(msg_buf, msg_len);
				}
			}
        }


    }
}

