/* 
 * File:   usb_comm.h
 * Author: nabillo
 *
 * Created on 19 juillet 2015, 13:56
 */

#ifndef USB_COMM_H
#define	USB_COMM_H

void APP_CustomCDCInitialize(void);     // legacy
void APP_CustomCDCTasks(void);          // legacy ProcessIO()
void usb_send(const char *format, ...);

#endif	/* USB_COMM_H */

