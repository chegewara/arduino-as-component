#include "esptinyusb.h"

#pragma once
enum
{
  VENDOR_REQUEST_WEBUSB = 1,
  VENDOR_REQUEST_MICROSOFT = 2
};
// typedef void (*webusb_connected_cb)(bool);
class WebUSB : public EspTinyUSB, Stream
{
public:
  WebUSB(uint8_t itf = 3);
  bool begin();
  int available(void);
  int peek(void);
  int read(void);
  size_t read(uint8_t *buffer, size_t size);
  void flush(void);
  size_t write(uint8_t);
  size_t write(const uint8_t *buffer, size_t size);

  operator bool() const;

  usb_connected_cb _connected_cb;
  void onConnect(usb_connected_cb cb);
  void landingPageURI(String url, bool ssl);

private:
  // uint8_t _itf;
  friend bool tud_vendor_control_request_cb(uint8_t rhport, tusb_control_request_t const *request);
  friend bool tud_vendor_control_complete_cb(uint8_t rhport, tusb_control_request_t const *request);
  uint8_t* _url;

};
