#ifndef USBJOYSTICK_H
#define USBJOYSTICK_H

#include "USBHID.h"

#define REPORT_ID_JOYSTICK  4

class USBJoystick: public USBHID {
   public:
     USBJoystick(uint16_t vendor_id = 0x1234, uint16_t product_id = 0x0100, uint16_t product_release = 0x0001):
         USBHID(0, 0, vendor_id, product_id, product_release, false)
         {
             connect();
         };

         bool update(int16_t x_l, int16_t y_l, uint8_t buttons_l, int16_t x_r, int16_t y_r, uint8_t buttons_r);

         virtual uint8_t * reportDesc();
};

#endif