// Copyright 2020 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "esptinyusb.h"
#include "usb_descriptors.h"
#include "usbd.h"
#include "tusb.h"
#include "esp_log.h"

#define USB_TUSB_PID (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | _PID_MAP(MIDI, 3))

char* EspTinyUSB::descriptor_str_config[11];
uint8_t* EspTinyUSB::descriptor_config = NULL;
uint8_t* EspTinyUSB::descriptor_config_if = NULL;

char EspTinyUSB::_DESC_MANUFACTURER_STRING[32] = "Espressif";
char EspTinyUSB::_DESC_PRODUCT_STRING[32] = "chege webUSB";
char EspTinyUSB::_DESC_SERIAL_STRING[32] = "12345678";
char EspTinyUSB::langId[2] = {0x09, 0x04};

char EspTinyUSB::_DESC_CDC_STRING[32] = "test";
char EspTinyUSB::_DESC_MSC_STRING[32] = "test";
char EspTinyUSB::_DESC_HID_STRING[32] = "test";
char EspTinyUSB::_DESC_VENDOR_STRING[32] = "vendor string";
char EspTinyUSB::_DESC_MIDI_STRING[32] = "test";


uint8_t *EspTinyUSB::getConfigurationDescriptor()
{
    int CONFIG_TOTAL_LEN = TUD_CONFIG_DESC_LEN + (int)enableCDC * TUD_CDC_DESC_LEN + (int)enableMSC * TUD_MSC_DESC_LEN +
                       (int)enableHID * TUD_HID_DESC_LEN + (int)enableVENDOR * TUD_VENDOR_DESC_LEN + 
                       (int)enableMIDI * TUD_MIDI_DESC_LEN + (int)enableDFU * TUD_DFU_RT_DESC_LEN;


    uint8_t count = (int)enableCDC + (int)enableCDC + (int)enableMSC + (int)enableHID + (int)enableVENDOR + (int)enableMIDI + (int)enableDFU;
    int total = 0;
    uint8_t ifIdx = 1;
    static uint8_t desc_configuration[500] = {0};
    // interface count, string index, total length, attribute, power in mA
    uint8_t dcd[TUD_CONFIG_DESC_LEN] = {TUD_CONFIG_DESCRIPTOR(1, count, 0, CONFIG_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 500)};
    memcpy(&desc_configuration[0], dcd, sizeof(dcd));
    total += sizeof(dcd);

#if CFG_TUD_CDC
    if (enableCDC)
    {
        // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
        uint8_t cdc[TUD_CDC_DESC_LEN] = {TUD_CDC_DESCRIPTOR(ifIdx, 4, 0x81, 8, 0x02, 0x82, 64)};
        memcpy(&desc_configuration[total], cdc, sizeof(cdc));
        total += sizeof(cdc);
        ifIdx += 2;
    }
#endif
#if CFG_TUD_MSC
    if(enableMSC) {
        #define EPNUM_MSC     0x03
        // Interface number, string index, EP Out & EP In address, EP size
        uint8_t msc[] = {TUD_MSC_DESCRIPTOR(ifIdx++, 5, EPNUM_MSC, 0x80 | EPNUM_MSC, 64)}; // highspeed 512
        memcpy(&desc_configuration[total], msc, sizeof(msc));
        total += sizeof(msc);
    }
#endif
#if CFG_TUD_HID
    if(enableHID) {
        uint8_t const desc_hid_report[] = {TUD_HID_REPORT_DESC_GENERIC_INOUT(CFG_TUD_HID_BUFSIZE)};
        // Interface number, string index, protocol, report descriptor len, EP In address, size & polling interval
        uint8_t hid[] = {TUD_HID_DESCRIPTOR(ifIdx++, 6, 0, sizeof(desc_hid_report), 0x84, 16, 10)};
        memcpy(&desc_configuration[total], hid, sizeof(hid));
        total += sizeof(hid);
    }
#endif
#if CFG_TUD_VENDOR
    if (enableVENDOR)
    {
    #define EPNUM_VENDOR  0x06
        // Interface number, string index, EP Out & IN address, EP size
        uint8_t vendor[] = {TUD_VENDOR_DESCRIPTOR(ifIdx++, 7, EPNUM_VENDOR, 0x80 | EPNUM_VENDOR, 64)};
        memcpy(&desc_configuration[total], vendor, sizeof(vendor));
        total += sizeof(vendor);
    }
#endif
#if CFG_TUD_MIDI
    if(enableMIDI) {
        #define EPNUM_MIDI   0x07
        // Interface number, string index, EP Out & EP In address, EP size
        uint8_t midi[] = {TUD_MIDI_DESCRIPTOR(ifIdx++, 8, EPNUM_MIDI, 0x80 | EPNUM_MIDI, (CFG_TUSB_RHPORT0_MODE & OPT_MODE_HIGH_SPEED) ? 512 : 64)};
        memcpy(&desc_configuration[total], midi, sizeof(midi));
        total += sizeof(midi);
    }
#endif

#if CFG_TUD_DFU_RT
    if(enableDFU) {
        #define EPNUM_DFU   0x08
        // Interface number, string index, attributes, detach timeout, transfer size
        uint8_t dfu[] = {TUD_DFU_RT_DESCRIPTOR(ifIdx++, 9, 0x0d, 1000, 4096)};
        memcpy(&desc_configuration[total], dfu, sizeof(dfu));
        ESP_LOG_BUFFER_HEX("", dfu, sizeof(dfu));
        total += sizeof(dfu);
    }
#endif

    if(EspTinyUSB::descriptor_config_if != NULL) {
        free(EspTinyUSB::descriptor_config_if);
    }
    EspTinyUSB::descriptor_config_if = (uint8_t*)calloc(1, total);
    memcpy(EspTinyUSB::descriptor_config_if, desc_configuration, total);

    return desc_configuration;
}

char** EspTinyUSB::getDeviceStringDescriptor()
{
    USBdevice.descriptor_str_config[0] = USBdevice.langId;      // 0: is supported language is English (0x0409)
    USBdevice.descriptor_str_config[1] = USBdevice._DESC_MANUFACTURER_STRING; // 1: Manufacturer
    USBdevice.descriptor_str_config[2] = USBdevice._DESC_PRODUCT_STRING;      // 2: Product
    USBdevice.descriptor_str_config[3] = USBdevice._DESC_SERIAL_STRING;       // 3: Serials, should use chip ID

    if (enableCDC)
        USBdevice.descriptor_str_config[4] = _DESC_CDC_STRING; // 4: CDC Interface

    if (enableMSC)
        USBdevice.descriptor_str_config[5] = _DESC_MSC_STRING; // 5: MSC Interface

    if (enableHID)
        USBdevice.descriptor_str_config[6] = _DESC_HID_STRING; // 6: HIDs

    if (enableVENDOR)
        USBdevice.descriptor_str_config[7] = _DESC_VENDOR_STRING; // 7: Vendor

    if (enableMIDI)
        USBdevice.descriptor_str_config[8] = _DESC_MIDI_STRING; // 8: MIDI

    if (enableDFU)
        USBdevice.descriptor_str_config[9] = _DESC_MIDI_STRING; // 9: DFU

    return USBdevice.descriptor_str_config;
}

tusb_desc_device_t EspTinyUSB::getDeviceDescriptor()
{
    /**** Kconfig driven Descriptor ****/
    tusb_desc_device_t descriptor_config = {
        .bLength = sizeof(descriptor_config),
        .bDescriptorType = TUSB_DESC_DEVICE,
        .bcdUSB = 0x0210,

        .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
        .idVendor = USBdevice._VID,
        .idProduct = USBdevice._PID,

        .bcdDevice = 0x100, // TODO add

        .iManufacturer = 0x01,
        .iProduct = 0x02,
        .iSerialNumber = 0x03,

        .bNumConfigurations = 0x01
    };

    if (enableCDC) {
        // Use Interface Association Descriptor (IAD) for CDC
        // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
        descriptor_config.bDeviceClass = TUSB_CLASS_MISC;
        descriptor_config.bDeviceSubClass = MISC_SUBCLASS_COMMON;
        descriptor_config.bDeviceProtocol = MISC_PROTOCOL_IAD;
    } else {
        descriptor_config.bDeviceClass = 0x00;
        descriptor_config.bDeviceSubClass = 0x00;
        descriptor_config.bDeviceProtocol = 0x00;
    }

    if(EspTinyUSB::descriptor_config != NULL) {
        free(EspTinyUSB::descriptor_config);
    }
    EspTinyUSB::descriptor_config = (uint8_t *)calloc(1, sizeof(descriptor_config));

    memcpy(EspTinyUSB::descriptor_config, &descriptor_config, sizeof(descriptor_config));

    return descriptor_config;
}

// =============================================================================
// Driver functions
// =============================================================================
/**
 * @brief Invoked when received GET DEVICE DESCRIPTOR.
 * Application returns pointer to descriptor
 *
 * @return uint8_t const*
 */
uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const*)EspTinyUSB::descriptor_config;
}

/**
 * @brief Invoked when received GET CONFIGURATION DESCRIPTOR.
 * Descriptor contents must exist long enough for transfer to complete
 *
 * @param index
 * @return uint8_t const* Application return pointer to descriptor
 */
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index; // for multiple configurations
    // ESP_LOG_BUFFER_HEX_LEVEL(__func__, EspTinyUSB::descriptor_config_if, 100, ESP_LOG_VERBOSE);
    return (uint8_t const*)EspTinyUSB::descriptor_config_if;
}

static uint16_t _desc_str[32];

/**
 * @brief Invoked when received GET STRING DESCRIPTOR request.
 * Application returns pointer to descriptor, whose contents must exist long
 * enough for transfer to complete
 *
 * @param index
 * @return uint16_t const*
 */
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    uint8_t chr_count = 0;
    ESP_LOGV("TAG", "index => %d", index);
    ESP_LOG_BUFFER_HEX_LEVEL(__func__, EspTinyUSB::descriptor_str_config[index], strlen(EspTinyUSB::descriptor_str_config[index]), ESP_LOG_VERBOSE);
    if (index == 0)
    {
        memcpy(&_desc_str[1], EspTinyUSB::descriptor_str_config[0], 2);
        chr_count = 1;
    }
    else
    {
        // Convert ASCII string into UTF-16
        if (index >= sizeof(EspTinyUSB::descriptor_str_config) /
                         sizeof(EspTinyUSB::descriptor_str_config[0]))
        {
            return NULL;
        }
        const char *str = EspTinyUSB::descriptor_str_config[index];
        // Cap at max char
        chr_count = strlen(str);
        if (chr_count > 31)
        {
            chr_count = 31;
        }
        for (uint8_t i = 0; i < chr_count; i++)
        {
            _desc_str[1 + i] = str[i];
        }
    }

    // first byte is len, second byte is string type
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

    return _desc_str;
}

