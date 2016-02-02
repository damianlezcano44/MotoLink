
#include "ch.h"
#include "hal.h"
#include "common.h"
#include <string.h>

/*===========================================================================*/
/* USB related stuff.                                                        */
/*===========================================================================*/

extern const USBConfig usbcfg;
extern SerialUSBConfig serusbcfg1;
extern SerialUSBConfig serusbcfg2;
extern SerialUSBDriver SDU1;
extern SerialUSBDriver SDU2;

extern bool doKLineInit;

extern SerialConfig uart1Cfg;
extern SerialConfig uart2Cfg;
extern SerialConfig uart3Cfg;

bool usbConnected(void);
bool usb_lld_connect_bus(USBDriver *usbp);
bool usb_lld_disconnect_bus(USBDriver *usbp);
