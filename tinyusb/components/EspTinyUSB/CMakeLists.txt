set(USB_SRCS
  tinyusb/src/device/usbd.c
  tinyusb/src/class/vendor/vendor_device.c
  tinyusb/src/class/cdc/cdc_device.c
  tinyusb/src/common/tusb_fifo.c
  tinyusb/src/portable/espressif/esp32s2/dcd_esp32s2.c
  tinyusb/src/device/usbd_control.c
  tinyusb/src/tusb.c
  tinyusb/src/class/msc/msc_device.c
  tinyusb/src/class/hid/hid_device.c
  tinyusb/src/class/midi/midi_device.c
  tinyusb/src/class/dfu/dfu_rt_device.c
)


set(includedirs
  tinyusb/src
  tinyusb/src/class
  tinyusb/src/device
  tinyusb/src/class/cdc
  tinyusb/src/class/dfu
  tinyusb/src/class/vendor

)

idf_component_register(SRCS ${USB_SRCS}  "webusb.cpp" "esptinyusb.cpp" "usb_descriptors.cpp" "cdcdevice.cpp" 
                    REQUIRES arduino
                    INCLUDE_DIRS . ${includedirs} )

target_compile_options(${COMPONENT_TARGET} PUBLIC
  "-DUSE_TINYUSB=1"
)
