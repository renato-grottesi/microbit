/**
 * This program implements a complete HID-over-Gatt Profile:
 *  - HID is provided by JoystickService
 *  - Battery Service
 *  - Device Information Service
 */

// MicroBit settings in MicroBitConfig.h located at: 
//   "microbit/microbit-dal/inc/core/MicroBitConfig.h"

#include "MicroBit.h"
#include "ble/services/BatteryService.h"
#include "HIDDeviceInformationService.h"
#include "ble/BLE.h"
#include "HIDServiceBase.h"
#include "JoystickService.h"

MicroBit uBit;
JoystickService *jstServicePtr;

void onConnected(MicroBitEvent)
{
    uBit.display.print("C");
}

void onDisconnected(MicroBitEvent)
{
    uBit.display.print("D");
}

void onButton(MicroBitEvent e)
{
    if(!jstServicePtr)
        return;

    if(!jstServicePtr->isConnected())
    {
        // we haven't connected yet...
        uBit.display.print("W");
    }
    else 
    {
        if (e.source == MICROBIT_ID_BUTTON_A)
        {
            if (e.value == MICROBIT_BUTTON_EVT_UP) 
            {
                jstServicePtr->setButton(JOYSTICK_BUTTON_1, BUTTON_UP);
            }
            else if (e.value == MICROBIT_BUTTON_EVT_DOWN) 
            {
                jstServicePtr->setButton(JOYSTICK_BUTTON_1, BUTTON_DOWN);
            }
        } else if (e.source == MICROBIT_ID_BUTTON_B)
        {
            if (e.value == MICROBIT_BUTTON_EVT_UP) 
            {
                jstServicePtr->setButton(JOYSTICK_BUTTON_2, BUTTON_UP);
            }
            else if (e.value == MICROBIT_BUTTON_EVT_DOWN) 
            {
                jstServicePtr->setButton(JOYSTICK_BUTTON_2, BUTTON_DOWN);
            }
        }

        jstServicePtr->sendCallback();
    }
}

int main()
{
    // Initialise the micro:bit runtime.
    uBit.init();
    // Display a start-up message
    uBit.display.scroll("BBB");
    uBit.display.print("L");
    
    // Application code will go here and in functions outside of main()
    
    uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_CONNECTED, onConnected);
    uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_DISCONNECTED, onDisconnected);  

    PnPID_t pnpID;
    pnpID.vendorID_source = 0x2; // from the USB Implementer's Forum
    pnpID.vendorID = 0x0D28; // NXP
    pnpID.productID = 0x0204; // CMSIS-DAP (well, it's a keyboard but oh well)
    pnpID.productVersion = 0x0100; // v1.0
    new HIDDeviceInformationService(*uBit.ble, "ARM", "m1", "abc", "def", "ghi", "jkl", &pnpID);

    new BatteryService(*uBit.ble, 80);
#if 1
    new MicroBitAccelerometerService(*uBit.ble, uBit.accelerometer); 
#else
    // causes a 020 :( error, aka OOM
    jstServicePtr = new JoystickService(*uBit.ble);
#endif

    // Register to receive events when any buttons are clicked
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_EVT_ANY, onButton);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_EVT_ANY, onButton);
    uBit.display.print("W");

    // end of application code in main()    
    
    // If main exits, there may still be other fibers running or registered event handlers etc.
    // Simply release this fiber, which will mean we enter the scheduler. Worse case, we then
    // sit in the idle task forever, in a power efficient sleep.
    release_fiber();
}