#include "Arduino.h"
#include "cdc_device.h"
#include "esptinyusb.h"
#include "cdcusb.h"

static bool web_serial_connected = true;
static CDCusb *_CDCusb = NULL;

CDCusb::CDCusb(uint8_t itf)
{
    itf = itf;
    _CDCusb = this;
}

bool CDCusb::begin()
{
    USBdevice.enableCDC = true;
    return EspTinyUSB::begin();
}

int CDCusb::available()
{
    return tud_cdc_n_available(_itf);
}

int CDCusb::peek()
{
    int pos; 
    uint8_t buffer[1];
    if (web_serial_connected)
    {
        tud_cdc_n_peek(_itf, 0, buffer);
        return buffer[0];
    } else {
        return -1;
    }
}

int CDCusb::read()
{
    return 0;
}

size_t CDCusb::read(uint8_t *buffer, size_t size)
{
    if(1)
    {
        if (tud_cdc_n_available(_itf)) {
            uint32_t count = tud_cdc_n_read(_itf, buffer, size);
            return count;
        }
    }

    return 0;
}

size_t CDCusb::write(uint8_t buffer)
{
    uint8_t c = buffer;
    if(web_serial_connected) {
        uint32_t d = tud_cdc_n_write(_itf, &c, 1);
        tud_cdc_n_write_flush(_itf);
        return d;
    } else {
        return 0;
    }
}

size_t CDCusb::write(const uint8_t *buffer, size_t size)
{
    if(web_serial_connected) {
        uint32_t d = tud_cdc_n_write(_itf, buffer, size);
        tud_cdc_n_write_flush(_itf);
        return d;
    } else {
        return 0;
    }
}

void CDCusb::flush()
{
    tud_cdc_n_read_flush(_itf);
    tud_cdc_n_write_flush(_itf);
}

CDCusb::operator bool() const
{
    return web_serial_connected;
}

void CDCusb::onConnect(usb_connected_cb cb)
{
    _connected_cb = cb;
}

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
  if (dtr && rts)
  {
    tud_cdc_write_str("Welcome to tinyUSB CDC example!!!\r\n");
  }
}

