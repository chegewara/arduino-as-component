#include "hal/usb_hal.h"
#include "soc/usb_periph.h"
#include "driver/periph_ctrl.h"
#include "driver/gpio.h"
#include "esp32s2/rom/gpio.h"
#include "esp_log.h"

#include "esptinyusb.h"
#include "usbd.h"
#include "tusb.h"

#define TAG __func__

xTaskHandle EspTinyUSB::usbTaskHandle;
bool EspTinyUSB::enableCDC;
bool EspTinyUSB::enableMSC;
bool EspTinyUSB::enableMIDI;
bool EspTinyUSB::enableHID;
bool EspTinyUSB::enableVENDOR;
bool EspTinyUSB::enableDFU;

uint16_t EspTinyUSB::_VID = 0x303a;
uint16_t EspTinyUSB::_PID = 0x0005;

static esp_tud_mount_cb _mount_cb = nullptr;
static esp_tud_umount_cb _umount_cb = nullptr;
static esp_tud_suspend_cb _suspend_cb = nullptr;
static esp_tud_resume_cb _resume_cb = nullptr;

static bool isEnabled = false;

static void esptinyusbtask(void *p)
{
    (void)p;
    ESP_LOGD(TAG, "USB tud_task created");
    while (1)
    {
        // tinyusb device task
        tud_task();
    }
}

EspTinyUSB::EspTinyUSB(bool extPhy)
{
    if (!isEnabled)
    {
        periph_module_reset((periph_module_t)PERIPH_USB_MODULE);
        periph_module_enable((periph_module_t)PERIPH_USB_MODULE);

        // Hal init
        usb_hal_context_t hal = {
            .use_external_phy = extPhy};
        usb_hal_init(&hal);
        /* usb_periph_iopins currently configures USB_OTG as USB Device.
        * Introduce additional parameters in usb_hal_context_t when adding support
        * for USB Host.
        */
        for (const usb_iopin_dsc_t *iopin = usb_periph_iopins; iopin->pin != -1; ++iopin)
        {
            if ((iopin->ext_phy_only == 0))
            {
                gpio_pad_select_gpio(iopin->pin);
                if (iopin->is_output)
                {
                    gpio_matrix_out(iopin->pin, iopin->func, false, false);
                }
                else
                {
                    gpio_matrix_in(iopin->pin, iopin->func, false);
                    gpio_pad_input_enable(iopin->pin);
                }
                gpio_pad_unhold(iopin->pin);
            }
        }
        gpio_set_drive_capability((gpio_num_t)USBPHY_DP_NUM, GPIO_DRIVE_CAP_3);
        gpio_set_drive_capability((gpio_num_t)USBPHY_DM_NUM, GPIO_DRIVE_CAP_3);
        isEnabled = true;
    }
}

bool EspTinyUSB::begin()
{
    USBdevice.enableDFU = true;

    getDeviceDescriptor();
    getDeviceStringDescriptor();
    getConfigurationDescriptor();

    if (tusb_inited()) {
        return  true;
    }

    if(tusb_init()) {
        ESP_LOGE("TAG", "failed to init");
        return false;
    }

    if (usbTaskHandle != nullptr) {
        return true;
    }
    
    return xTaskCreate(esptinyusbtask, "espUSB", 2 * 1024, NULL, 22, &usbTaskHandle) == pdTRUE;
}

void EspTinyUSB::registerDeviceCallbacks(esp_tud_mount_cb mount_cb, esp_tud_umount_cb umount_cb,
                                        esp_tud_suspend_cb suspend_cb, esp_tud_resume_cb resume_cb)
{
    if (mount_cb)
    {
        _mount_cb = mount_cb;
    }
    
    if (umount_cb)
    {
        _umount_cb = umount_cb;
    }
    
    if (suspend_cb)
    {
        _suspend_cb = suspend_cb;
    }
    
    if (resume_cb)
    {
        _resume_cb = resume_cb;
    }
}


void EspTinyUSB::deviceID(uint16_t vid, uint16_t pid)
{
    _VID = vid;
    _PID = pid;
}

void EspTinyUSB::deviceID(uint16_t* vid, uint16_t* pid)
{
    *vid = _VID;
    *pid = _PID;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    ESP_LOGI(TAG, "%s", __func__);
    if (_mount_cb != nullptr)
    {
        _mount_cb();
    }
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    ESP_LOGI(TAG, "%s", __func__);
    if (_umount_cb != nullptr)
    {
        _umount_cb();
    }
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    ESP_LOGI(TAG, "%s", __func__);
    if (_suspend_cb != nullptr)
    {
        _suspend_cb(remote_wakeup_en);
    }
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    ESP_LOGI(TAG, "%s", __func__);
    if (_resume_cb != nullptr)
    {
        _resume_cb();
    }
}
