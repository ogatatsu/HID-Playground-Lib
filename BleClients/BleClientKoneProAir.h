/*
  The MIT License (MIT)

  Copyright (c) 2020 ogatatsu.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#pragma once

#include "BLEClientCharacteristic.h"
#include "BLEClientService.h"

namespace hidpg
{

#pragma pack(1)
  struct kone_pro_air_mouse_report_t
  {
    uint8_t buttons;
    int16_t x;
    int16_t y;
    int8_t wheel;
  };
#pragma pack()

  class BleClientKoneProAir : public BLEClientService
  {
  public:
    // Callback Signatures
    using mouse_callback_t = void (*)(kone_pro_air_mouse_report_t *report);

    BleClientKoneProAir();

    virtual bool begin();
    virtual bool discover(uint16_t conn_handle);

    // Mouse API
    bool enableMouse();
    bool disableMouse();

    // Report callback
    void setMouseReportCallback(mouse_callback_t fp);

  protected:
    mouse_callback_t _mouse_cb;
    BLEClientCharacteristic _mouse_input;

    void _handle_mouse_input(uint8_t *data, uint16_t len);
    static void mouse_client_notify_cb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len);
  };

} // namespace hidpg
