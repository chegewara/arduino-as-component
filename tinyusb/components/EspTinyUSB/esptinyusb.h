
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "usb_descriptors.h"

#pragma once
//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

typedef void (*esp_tud_mount_cb)(void);
typedef void (*esp_tud_umount_cb)(void);
typedef void (*esp_tud_suspend_cb)(bool remote_wakeup_en);
typedef void (*esp_tud_resume_cb)(void);
typedef void (*usb_connected_cb)(bool);

class EspTinyUSB
{
public:
    EspTinyUSB(bool extPhy = false);
    bool begin();
    static void registerDeviceCallbacks(esp_tud_mount_cb _mount_cb = nullptr, esp_tud_umount_cb _umount_cb = nullptr,
                                esp_tud_suspend_cb _suspend_cb = nullptr, esp_tud_resume_cb _resume_cb = nullptr);


    static tusb_desc_device_t getDeviceDescriptor();
    static char** getDeviceStringDescriptor();
    static uint8_t* getConfigurationDescriptor();
    static void deviceID(uint16_t, uint16_t);
    static void deviceID(uint16_t*, uint16_t*);


    friend uint8_t const * tud_descriptor_device_cb(void);
    friend uint8_t const * tud_descriptor_configuration_cb(uint8_t index);
    friend uint8_t const *tud_descriptor_device_cb(void);
    friend uint8_t const *tud_descriptor_configuration_cb(uint8_t index);
    friend uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid);
    friend tusb_desc_device_t *tusb_get_active_desc(void);
    friend char **tusb_get_active_str_desc(void);
    friend void tusb_clear_descriptor(void);

protected:
    static char* descriptor_str_config[11];
    static uint8_t* descriptor_config;
    static uint8_t* descriptor_config_if;
    uint8_t _itf;

    static xTaskHandle usbTaskHandle;

    static bool enableCDC;
    static bool enableMSC;
    static bool enableMIDI;
    static bool enableHID;
    static bool enableVENDOR;
    static bool enableDFU;

    static char _DESC_MANUFACTURER_STRING[32];
    static char _DESC_PRODUCT_STRING[32];
    static char _DESC_SERIAL_STRING[32];

    static char _DESC_CDC_STRING[32];
    static char _DESC_MSC_STRING[32];
    static char _DESC_HID_STRING[32];
    static char _DESC_VENDOR_STRING[32];
    static char _DESC_MIDI_STRING[32];

    static char langId[2];
    static uint16_t _VID;
    static uint16_t _PID;
};

extern EspTinyUSB USBdevice;
