#include "MicroBit.h"
#include "MicroBitUARTService.h"

MicroBit uBit;
MicroBitUARTService *uart;

int connected = 0;

void onConnected(MicroBitEvent e)
{
    uBit.display.scroll("C");
    connected = 1;
    ManagedString end_of_client_message(":");
    
    while(1) {
        ManagedString msg = uart->readUntil(end_of_client_message);
        uBit.display.scroll(msg);
    }
}

void onDisconnected(MicroBitEvent e)
{
    uBit.display.scroll("D");
    connected = 0;
}

void onButtonA(MicroBitEvent e)
{
    if (connected == 0) {
        uBit.display.scroll("NC");
        return;
    }
    uart->send("UPD");
    uBit.display.scroll("UPD");
}

#ifndef MICROBIT_BLE_DFU_SERVICE
#define MICROBIT_BLE_DFU_SERVICE                0
#endif
#ifndef MICROBIT_BLE_EVENT_SERVICE
#define MICROBIT_BLE_EVENT_SERVICE              0
#endif

int main()
{
    uBit.init();

    uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_CONNECTED, onConnected);
    uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_DISCONNECTED, onDisconnected);
    uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, onButtonA);

    uart = new MicroBitUARTService(*uBit.ble, 32, 32); 
    uBit.display.scroll("AVM");

    release_fiber();
}
