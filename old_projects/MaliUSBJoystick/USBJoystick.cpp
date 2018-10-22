#include "stdint.h"
#include "USBJoystick.h"

bool USBJoystick::update(int16_t x_l, int16_t y_l, uint8_t buttons_l, int16_t x_r, int16_t y_r, uint8_t buttons_r)
{
   HID_REPORT report;
   // Fill the report according to the Joystick Descriptor
   report.data[0] = x_l & 0xff;
   report.data[1] = y_l & 0xff;
   report.data[2] = x_r & 0xff;
   report.data[3] = y_r & 0xff;
   report.data[4] = buttons_l;
   report.data[5] = buttons_r;
   report.length = 6;

   return send(&report);
}

uint8_t * USBJoystick::reportDesc()
{
    static uint8_t reportDescriptor[] =
    {
         USAGE_PAGE(1), 0x01,           // Generic Desktop
         LOGICAL_MINIMUM(1), 0x00,      // Logical_Minimum (0)
         USAGE(1), 0x04,                // Usage (Joystick)

         COLLECTION(1), 0x01,           // Application
           USAGE_PAGE(1), 0x01,            // Generic Desktop
           USAGE(1), 0x01,                 // Usage (Pointer)
           COLLECTION(1), 0x00,            // Physical
             USAGE(1), 0x30,                 // X
             USAGE(1), 0x31,                 // Y
             USAGE(1), 0x33,                 // Rx
             USAGE(1), 0x34,                 // Ry
             LOGICAL_MINIMUM(1), 0x81,       // -127
             LOGICAL_MAXIMUM(1), 0x7f,       // 127
             REPORT_SIZE(1), 0x08,
             REPORT_COUNT(1), 0x04,
             INPUT(1), 0x02,                 // Data, Variable, Absolute         
           END_COLLECTION(0),

           USAGE_PAGE(1), 0x09,            // Buttons
           USAGE_MINIMUM(1), 0x01,         // 1
           USAGE_MAXIMUM(1), 0x10,         // 16 buttons
           LOGICAL_MINIMUM(1), 0x00,       // 0
           LOGICAL_MAXIMUM(1), 0x01,       // 1
           REPORT_SIZE(1), 0x01,
           REPORT_COUNT(1), 0x10,          // 16 buttons
           UNIT_EXPONENT(1), 0x00,         // Unit_Exponent (0)
           UNIT(1), 0x00,                  // Unit (None)
           INPUT(1), 0x02,                 // Data, Variable, Absolute
         END_COLLECTION(0)
    };

    reportLength = sizeof(reportDescriptor);
    return reportDescriptor;
}
