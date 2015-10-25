
#include <stdio.h>
#include <usb/usb.h>
#include <usb/usb_device_cdc.h>

#include "system.h"
#include "usb_comm.h"

/*********************************************************************
* Function:     void APP_CustomHIDInitialize(void);
*               (legacy USBCBInitEP();)
*
* Overview:     Initializes the Custom HID demo code
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_CustomCDCInitialize()
{
    CDCInitEP();
}

void usb_send(const char *format, ...)
{
    //char *usb_msg;
    static unsigned char usb_msg[CDC_DATA_OUT_EP_SIZE];

    va_list args;
    va_start(args,__format);
    sprintf(usb_msg,format,args);
    if (( USBGetDeviceState() < CONFIGURED_STATE ) || ( USBIsDeviceSuspended() == true ))
    {
        return;
    }
    else
    {
        if (mUSBUSARTIsTxTrfReady())
        {
            putUSBUSART(usb_msg,strlen(usb_msg));
        }
        CDCTxService();
    }

}

