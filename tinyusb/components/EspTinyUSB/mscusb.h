
#include "esptinyusb.h"

#pragma once


class MSCusb : public EspTinyUSB, Stream
{
    MSCusb();
};