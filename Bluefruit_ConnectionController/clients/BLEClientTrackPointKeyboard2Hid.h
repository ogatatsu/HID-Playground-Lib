/*
  The MIT License (MIT)

  Copyright (c) 2022 ogatatsu.

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

  class BLEClientTrackPointKeyboard2Hid : public BLEClientService
  {
  public:
    // Callback Signatures
    using keyboard_callback_t = void (*)(uint8_t modifiers, uint8_t key_codes[6]);
    using trackpoint_callback_t = void (*)(uint8_t buttons, int8_t x, int8_t y, int8_t wheel);
    using consumer_callback_t = void (*)(uint16_t usage_code);

    BLEClientTrackPointKeyboard2Hid();

    bool begin() override;
    bool discover(uint16_t conn_handle) override;

    bool enableKeyboard();
    bool disableKeyboard();

    bool enableTrackpoint();
    bool disableTrackpoint();

    bool enableConsumer();
    bool disableConsumer();

    // Report callback
    void setKeyboardReportCallback(keyboard_callback_t fp);
    void setTrackpointReportCallback(trackpoint_callback_t fp);
    void setConsumerReportCallback(consumer_callback_t fp);

  private:
    keyboard_callback_t _keyboard_cb;
    trackpoint_callback_t _trackpoint_cb;
    consumer_callback_t _consumer_cb;

    BLEClientCharacteristic _keyboard_input;
    BLEClientCharacteristic _trackpoint_input;
    BLEClientCharacteristic _consumer_input;

    void _handle_keyboard_input(uint8_t *data, uint16_t len);
    void _handle_trackpoint_input(uint8_t *data, uint16_t len);
    void _handle_consumer_input(uint8_t *data, uint16_t len);

    static void keyboard_client_notify_cb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len);
    static void trackpoint_client_notify_cb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len);
    static void consumer_client_notify_cb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len);

#pragma pack(1)
    struct keyboard_report_t
    {
      uint8_t modifiers;
      uint8_t reserved;
      uint8_t key_codes[6];
    };

    struct trackpoint_report_t
    {
      uint8_t buttons;
      int8_t x;
      int8_t y;
      int8_t wheel;
    };

    struct consumer_report_t
    {
      uint16_t usage_code;
    };
#pragma pack()
  };

} // namespace hidpg
