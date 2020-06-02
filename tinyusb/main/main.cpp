#include "Arduino.h"
#include "webusb.h"
#include "cdcusb.h"
#include "mscusb.h"
#include "hal/usb_hal.h"
#include "soc/usb_periph.h"
#include "driver/periph_ctrl.h"
#include "driver/gpio.h"
#include "esp32s2/rom/gpio.h"

// #define WEB
#define CDC
// #define MSC
extern "C" uint8_t *msc_disk;

#ifdef WEB
WebUSB USBSerial;
#endif
#ifdef CDC
CDCusb USBSerial1;
#endif
EspTinyUSB dev;

void conCB(bool isCon)
{
    Serial.printf("connection state changed, new state %s\n", isCon? "connected" : "disconnected");
}
// extern "C" void init_disk();

void setup() {
    Serial.begin(115200);
    // msc_disk = (uint8_t*)calloc(1, 2*10*512);
    // if(msc_disk == nullptr) Serial.println("ERROR");
//   Serial.printf( "SIZE: %d", (sizeof(msc_disk)/sizeof(msc_disk[0])));

dev.begin();
    // init_disk();
    // EspTinyUSB::deviceID(0xcafe, 0x9876);
#ifdef WEB
    if(!USBSerial.begin())
        Serial.println("Failed to start webUSB stack");
#endif

#ifdef CDC
    if(!USBSerial1.begin())
        Serial.println("Failed to start CDC USB stack");
#endif

#ifdef WEB
    // USBSerial.onConnect(conCB);
#endif
#ifdef CDC
    USBSerial1.onConnect(conCB);
#endif
}

char buff[10] = {0};
void loop() {
#ifdef WEB
    // while (USBSerial.peek())
    // {
    //     // Serial.println(USBSerial.write(buff[0]));
    //     // Serial.print(USBSerial.read());
    // }
#endif
#ifdef CDC
    while (USBSerial1.read((uint8_t*)buff, 10))
    {
        USBSerial1.write(buff[0]);
        Serial.print(buff);
    }
#endif
    if(Serial.available())
    {
#ifdef WEB
        USBSerial.write(Serial.peek());
#endif
#ifdef CDC
        USBSerial1.write(Serial.peek());
#endif
        Serial.write(Serial.read());
    }
}


void tud_dfu_rt_reboot_to_dfu(void){
    Serial.println("DFU callback, switch to DFU mode");
}