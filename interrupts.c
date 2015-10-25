/******************************************************************************/
/*Files to Include                                                            */
/******************************************************************************/

#include <xc.h>         /* XC8 General Include File */
#include "usb_config.h"
#include <usb/usb.h>

extern unsigned short timeout;
extern long seconds;

/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/

/* High-priority service */

void interrupt high_isr(void)
{
/*
    if (TMR2IE && TMR2IF)
    {
#asm
        BSF TMR1H, 7
#endasm
        TMR2IF=0;
        seconds++;
        if (timeout > 0)
        {
            timeout--;
        }
    }
*/
    #if defined(USB_INTERRUPT)
        USBDeviceTasks();
    #endif
}
