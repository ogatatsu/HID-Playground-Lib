/**************************************************************************/
/*!
    @file     BLEHidAdafruit.cpp
    @author   hathach (tinyusb.org)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2018, Adafruit Industries (adafruit.com)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/**************************************************************************/

#include "BLEHid.h"

#include "class/hid/hid_device.h"

namespace hidpg
{

  // clang-format off

  // Keyboard Report Descriptor Template
  #define TUD_HID_REPORT_DESC_KEYBOARD_FIX(...) \
    HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP     )                    ,\
    HID_USAGE      ( HID_USAGE_DESKTOP_KEYBOARD )                    ,\
    HID_COLLECTION ( HID_COLLECTION_APPLICATION )                    ,\
      /* Report ID if any */\
      __VA_ARGS__ \
      /* 8 bits Modifier Keys (Shfit, Control, Alt) */ \
      HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD )                     ,\
        HID_USAGE_MIN    ( 224                                    )  ,\
        HID_USAGE_MAX    ( 231                                    )  ,\
        HID_LOGICAL_MIN  ( 0                                      )  ,\
        HID_LOGICAL_MAX  ( 1                                      )  ,\
        HID_REPORT_COUNT ( 8                                      )  ,\
        HID_REPORT_SIZE  ( 1                                      )  ,\
        HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE )  ,\
        /* 8 bit reserved */ \
        HID_REPORT_COUNT ( 1                                      )  ,\
        HID_REPORT_SIZE  ( 8                                      )  ,\
        HID_INPUT        ( HID_CONSTANT                           )  ,\
      /* 6-byte Keycodes */ \
      HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD )                     ,\
        HID_USAGE_MIN    ( 0                                   )     ,\
        HID_USAGE_MAX_N  ( 255, 2                              )     ,\
        HID_LOGICAL_MIN  ( 0                                   )     ,\
        HID_LOGICAL_MAX_N( 255, 2                              )     ,\
        HID_REPORT_COUNT ( 6                                   )     ,\
        HID_REPORT_SIZE  ( 8                                   )     ,\
        HID_INPUT        ( HID_DATA | HID_ARRAY | HID_ABSOLUTE )     ,\
      /* 5-bit LED Indicator Kana | Compose | ScrollLock | CapsLock | NumLock */ \
      HID_USAGE_PAGE  ( HID_USAGE_PAGE_LED                   )       ,\
        HID_USAGE_MIN    ( 1                                       ) ,\
        HID_USAGE_MAX    ( 5                                       ) ,\
        HID_REPORT_COUNT ( 5                                       ) ,\
        HID_REPORT_SIZE  ( 1                                       ) ,\
        HID_OUTPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE  ) ,\
        /* led padding */ \
        HID_REPORT_COUNT ( 1                                       ) ,\
        HID_REPORT_SIZE  ( 3                                       ) ,\
        HID_OUTPUT       ( HID_CONSTANT                            ) ,\
    HID_COLLECTION_END \

  // Mouse Report Descriptor
  #define TUD_HID_REPORT_DESC_MOUSE_EX(...) \
    HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP      )                   ,\
    HID_USAGE      ( HID_USAGE_DESKTOP_MOUSE     )                   ,\
    HID_COLLECTION ( HID_COLLECTION_APPLICATION  )                   ,\
      /* Report ID if any */\
      __VA_ARGS__ \
      HID_USAGE      ( HID_USAGE_DESKTOP_POINTER )                   ,\
      HID_COLLECTION ( HID_COLLECTION_PHYSICAL   )                   ,\
        HID_USAGE_PAGE  ( HID_USAGE_PAGE_BUTTON  )                   ,\
          HID_USAGE_MIN   ( 1                                      ) ,\
          HID_USAGE_MAX   ( 5                                      ) ,\
          HID_LOGICAL_MIN ( 0                                      ) ,\
          HID_LOGICAL_MAX ( 1                                      ) ,\
          /* Left, Right, Middle, Backward, Forward buttons */ \
          HID_REPORT_COUNT( 5                                      ) ,\
          HID_REPORT_SIZE ( 1                                      ) ,\
          HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ) ,\
          /* 3 bit padding */ \
          HID_REPORT_COUNT( 1                                      ) ,\
          HID_REPORT_SIZE ( 3                                      ) ,\
          HID_INPUT       ( HID_CONSTANT                           ) ,\
        HID_USAGE_PAGE  ( HID_USAGE_PAGE_DESKTOP )                   ,\
          /* X, Y position [-32768, 32767] */ \
          HID_USAGE       ( HID_USAGE_DESKTOP_X                    ) ,\
          HID_USAGE       ( HID_USAGE_DESKTOP_Y                    ) ,\
          HID_LOGICAL_MIN_N ( 0x8000, 2                            ) ,\
          HID_LOGICAL_MAX_N ( 0x7fff, 2                            ) ,\
          HID_REPORT_COUNT( 2                                      ) ,\
          HID_REPORT_SIZE ( 16                                     ) ,\
          HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_RELATIVE ) ,\
          /* Verital wheel scroll [-127, 127] */ \
          HID_USAGE       ( HID_USAGE_DESKTOP_WHEEL                )  ,\
          HID_LOGICAL_MIN ( 0x81                                   )  ,\
          HID_LOGICAL_MAX ( 0x7f                                   )  ,\
          HID_REPORT_COUNT( 1                                      )  ,\
          HID_REPORT_SIZE ( 8                                      )  ,\
          HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_RELATIVE )  ,\
        HID_USAGE_PAGE  ( HID_USAGE_PAGE_CONSUMER ), \
        /* Horizontal wheel scroll [-127, 127] */ \
          HID_USAGE_N     ( HID_USAGE_CONSUMER_AC_PAN, 2           ), \
          HID_LOGICAL_MIN ( 0x81                                   ), \
          HID_LOGICAL_MAX ( 0x7f                                   ), \
          HID_REPORT_COUNT( 1                                      ), \
          HID_REPORT_SIZE ( 8                                      ), \
          HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_RELATIVE ), \
      HID_COLLECTION_END                                            , \
    HID_COLLECTION_END \

  enum
  {
    REPORT_ID_KEYBOARD = 1,
    REPORT_ID_CONSUMER_CONTROL,
    REPORT_ID_MOUSE,
  };

  uint8_t const hid_report_descriptor[] =
  {
    TUD_HID_REPORT_DESC_KEYBOARD_FIX( HID_REPORT_ID(REPORT_ID_KEYBOARD) ),
    TUD_HID_REPORT_DESC_CONSUMER( HID_REPORT_ID(REPORT_ID_CONSUMER_CONTROL) ),
    TUD_HID_REPORT_DESC_MOUSE_EX( HID_REPORT_ID(REPORT_ID_MOUSE) )
  };
  // clang-format on

#pragma pack(1)
  struct hid_mouse_report_ex_t
  {
    uint8_t buttons;
    int16_t x;
    int16_t y;
    int8_t wheel;
    int8_t pan;
  };
#pragma pack()

  BLEHid::BLEHid() : BLEHidGeneric(3, 1, 0), _kbd_led_cb1(nullptr), _kbd_led_cb2(nullptr)
  {
  }

  err_t BLEHid::begin()
  {
    uint16_t input_len[] = {sizeof(hid_keyboard_report_t), sizeof(hid_consumer_control_report_t), sizeof(hid_mouse_report_ex_t)};
    uint16_t output_len[] = {1};

    setReportLen(input_len, output_len, NULL);
    enableKeyboard(true);
    enableMouse(true);
    setReportMap(hid_report_descriptor, sizeof(hid_report_descriptor));

    VERIFY_STATUS(BLEHidGeneric::begin());

    return ERROR_NONE;
  }

  void BLEHid::keyboard_output_cb(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len)
  {
    BLEHid &svc = (BLEHid &)chr->parentService();

    if (svc._kbd_led_cb1 != nullptr)
      svc._kbd_led_cb1(data[0]);

    if (svc._kbd_led_cb2 != nullptr)
      svc._kbd_led_cb2(conn_hdl, data[0]);
  }

  void BLEHid::setKeyboardLedCallback(kbd_led_cb1_t cb)
  {
    _kbd_led_cb1 = cb;

    // Report mode
    this->setOutputReportCallback(REPORT_ID_KEYBOARD, cb ? keyboard_output_cb : NULL);
    // Boot mode
    _chr_boot_keyboard_output->setWriteCallback(cb ? keyboard_output_cb : NULL);
  }

  void BLEHid::setKeyboardLedCallback(kbd_led_cb2_t cb)
  {
    _kbd_led_cb2 = cb;

    this->setOutputReportCallback(REPORT_ID_KEYBOARD, cb ? keyboard_output_cb : NULL);

    _chr_boot_keyboard_output->setWriteCallback(cb ? keyboard_output_cb : NULL);
  }

  bool BLEHid::keyboardReport(uint16_t conn_hdl, uint8_t modifier, uint8_t key_codes[6])
  {
    hid_keyboard_report_t report;
    report.modifier = modifier;
    memcpy(report.keycode, key_codes, 6);

    if (isBootMode())
    {
      return bootKeyboardReport(conn_hdl, &report, sizeof(hid_keyboard_report_t));
    }
    else
    {
      return inputReport(conn_hdl, REPORT_ID_KEYBOARD, &report, sizeof(hid_keyboard_report_t));
    }
  }

  bool BLEHid::keyboardReport(uint8_t modifier, uint8_t key_codes[6])
  {
    return keyboardReport(BLE_CONN_HANDLE_INVALID, modifier, key_codes);
  }

  bool BLEHid::consumerReport(uint16_t conn_hdl, uint16_t usage_code)
  {
    return inputReport(conn_hdl, REPORT_ID_CONSUMER_CONTROL, &usage_code, sizeof(usage_code));
  }

  bool BLEHid::consumerReport(uint16_t usage_code)
  {
    return consumerReport(BLE_CONN_HANDLE_INVALID, usage_code);
  }

  bool BLEHid::mouseReport(uint16_t conn_hdl, uint8_t buttons, int16_t x, int16_t y, int8_t wheel, int8_t horiz)
  {
    if (isBootMode())
    {
      hid_mouse_report_t report;
      report.buttons = buttons;
      report.x = constrain(x, -127, 127);
      report.y = constrain(y, -127, 127);
      report.wheel = wheel;
      report.pan = horiz;
      return bootMouseReport(conn_hdl, &report, sizeof(hid_mouse_report_t));
    }
    else
    {
      hid_mouse_report_ex_t report;
      report.buttons = buttons;
      report.x = x;
      report.y = y;
      report.wheel = wheel;
      report.pan = horiz;
      return inputReport(conn_hdl, REPORT_ID_MOUSE, &report, sizeof(hid_mouse_report_ex_t));
    }
  }

  bool BLEHid::mouseReport(uint8_t buttons, int16_t x, int16_t y, int8_t wheel, int8_t horiz)
  {
    return mouseReport(BLE_CONN_HANDLE_INVALID, buttons, x, y, wheel, horiz);
  }

  bool BLEHid::waitReady(uint16_t conn_hdl)
  {
    BLEConnection *conn = Bluefruit.Connection(conn_hdl);

    if (conn != nullptr)
    {
      if (conn->getHvnPacket() == false)
        return false;
      conn->releaseHvnPacket();
    }
    return true;
  }

  bool BLEHid::waitReady()
  {
    uint8_t conn_hdl = Bluefruit.connHandle();
    return waitReady(conn_hdl);
  }

} // namespace hidpg
