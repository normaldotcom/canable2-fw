#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"


/* USB Device Core handle declaration. */
USBD_HandleTypeDef hUsbDeviceFS;
extern USBD_DescriptorsTypeDef CDC_Desc;


/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */
void usb_init(void)
{
  /* USER CODE BEGIN USB_Device_Init_PreTreatment */
  
  /* USER CODE END USB_Device_Init_PreTreatment */
  
  /* Init Device Library, add supported class and start the library. */
  if (USBD_Init(&hUsbDeviceFS, &CDC_Desc, DEVICE_FS) != USBD_OK) {
    //Error_Handler();
	  while(1);
  }
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK) {
	  while(1);
  }
  if (USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS) != USBD_OK) {
	  while(1);
  }
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK) {
	  while(1);
  }
  /* USER CODE BEGIN USB_Device_Init_PostTreatment */
  
  /* USER CODE END USB_Device_Init_PostTreatment */
}
