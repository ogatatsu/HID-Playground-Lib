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

#include "class/hid/hid_device.h"

namespace hidpg
{

  // clang-format off

  // Keyboard Report Descriptor
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

  // Radial Controller Report Descriptor
  // https://docs.microsoft.com/en-us/windows-hardware/design/component-guidelines/radial-controller-sample-report-descriptors
  #define TUD_HID_REPORT_DESC_RADIAL_CONTROLLER(...) \
    0x05, 0x01,                      /* USAGE_PAGE (Generic Desktop)          */ \
    0x09, 0x0e,                      /* USAGE (System Multi-Axis Controller)  */ \
    0xa1, 0x01,                      /* COLLECTION (Application)              */ \
    __VA_ARGS__                      /*   REPORT_ID (Radial Controller)       */ \
    0x05, 0x0d,                      /*   USAGE_PAGE (Digitizers)             */ \
    0x09, 0x21,                      /*   USAGE (Puck)                        */ \
    0xa1, 0x00,                      /*   COLLECTION (Physical)               */ \
    0x05, 0x09,                      /*     USAGE_PAGE (Buttons)              */ \
    0x09, 0x01,                      /*     USAGE (Button 1)                  */ \
    0x95, 0x01,                      /*     REPORT_COUNT (1)                  */ \
    0x75, 0x01,                      /*     REPORT_SIZE (1)                   */ \
    0x15, 0x00,                      /*     LOGICAL_MINIMUM (0)               */ \
    0x25, 0x01,                      /*     LOGICAL_MAXIMUM (1)               */ \
    0x81, 0x02,                      /*     INPUT (Data,Var,Abs)              */ \
    0x05, 0x01,                      /*     USAGE_PAGE (Generic Desktop)      */ \
    0x09, 0x37,                      /*     USAGE (Dial)                      */ \
    0x95, 0x01,                      /*     REPORT_COUNT (1)                  */ \
    0x75, 0x0f,                      /*     REPORT_SIZE (15)                  */ \
    0x55, 0x0f,                      /*     UNIT_EXPONENT (-1)                */ \
    0x65, 0x14,                      /*     UNIT (Degrees, English Rotation)  */ \
    0x36, 0xf0, 0xf1,                /*     PHYSICAL_MINIMUM (-3600)          */ \
    0x46, 0x10, 0x0e,                /*     PHYSICAL_MAXIMUM (3600)           */ \
    0x16, 0xf0, 0xf1,                /*     LOGICAL_MINIMUM (-3600)           */ \
    0x26, 0x10, 0x0e,                /*     LOGICAL_MAXIMUM (3600)            */ \
    0x81, 0x06,                      /*     INPUT (Data,Var,Rel)              */ \
    /* ----------------------------- Optional ------------------------------- */ \
    /*  0x09, 0x30,              */  /*     USAGE (X)                         */ \
    /*  0x75, 0x10,              */  /*     REPORT_SIZE (16)                  */ \
    /*  0x55, 0x0d,              */  /*     UNIT_EXPONENT (-3)                */ \
    /*  0x65, 0x13,              */  /*     UNIT (Inch,EngLinear)             */ \
    /*  0x35, 0x00,              */  /*     PHYSICAL_MINIMUM (0)              */ \
    /*  0x46, 0xc0, 0x5d,        */  /*     PHYSICAL_MAXIMUM (24000)          */ \
    /*  0x15, 0x00,              */  /*     LOGICAL_MINIMUM (0)               */ \
    /*  0x26, 0xff, 0x7f,        */  /*     LOGICAL_MAXIMUM (32767)           */ \
    /*  0x81, 0x02,              */  /*     INPUT (Data,Var,Abs)              */ \
    /*  0x09, 0x31,              */  /*     USAGE (Y)                         */ \
    /*  0x46, 0xb0, 0x36,        */  /*     PHYSICAL_MAXIMUM (14000)          */ \
    /*  0x81, 0x02,              */  /*     INPUT (Data,Var,Abs)              */ \
    /*  0x05, 0x0d,              */  /*     USAGE_PAGE (Digitizers)           */ \
    /*  0x09, 0x48,              */  /*     USAGE (Width)                     */ \
    /*  0x36, 0xb8, 0x0b,        */  /*     PHYSICAL_MINIMUM (3000)           */ \
    /*  0x46, 0xb8, 0x0b,        */  /*     PHYSICAL_MAXIMUM (3000)           */ \
    /*  0x16, 0xb8, 0x0b,        */  /*     LOGICAL_MINIMUM (3000)            */ \
    /*  0x26, 0xb8, 0x0b,        */  /*     LOGICAL_MAXIMUM (3000)            */ \
    /*  0x81, 0x03               */  /*     INPUT (Cnst,Var,Abs)              */ \
    /* ---------------------------------------------------------------------- */ \
    0xc0,                            /*   END_COLLECTION                      */ \
    0xc0                             /* END_COLLECTION                        */ \

  // System Control Report Descriptor
  #define TUD_HID_REPORT_DESC_SYSTEM_CONTROL_FIX(...) \
    HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP           )        ,\
    HID_USAGE      ( HID_USAGE_DESKTOP_SYSTEM_CONTROL )        ,\
    HID_COLLECTION ( HID_COLLECTION_APPLICATION       )        ,\
      /* Report ID if any */\
      __VA_ARGS__ \
      /* 2 bit system power control */ \
      HID_LOGICAL_MIN  ( 1                                   ) ,\
      HID_LOGICAL_MAX  ( 3                                   ) ,\
      HID_REPORT_COUNT ( 1                                   ) ,\
      HID_REPORT_SIZE  ( 2                                   ) ,\
      HID_USAGE        ( HID_USAGE_DESKTOP_SYSTEM_POWER_DOWN ) ,\
      HID_USAGE        ( HID_USAGE_DESKTOP_SYSTEM_SLEEP      ) ,\
      HID_USAGE        ( HID_USAGE_DESKTOP_SYSTEM_WAKE_UP    ) ,\
      HID_INPUT        ( HID_DATA | HID_ARRAY | HID_ABSOLUTE ) ,\
      /* 6 bit padding */ \
      HID_REPORT_COUNT ( 1                                   ) ,\
      HID_REPORT_SIZE  ( 6                                   ) ,\
      HID_INPUT        ( HID_CONSTANT                        ) ,\
    HID_COLLECTION_END \

  enum
  {
    REPORT_ID_KEYBOARD = 1,
    REPORT_ID_CONSUMER_CONTROL,
    REPORT_ID_MOUSE,
    REPORT_ID_RADIAL_CONTROLLER,
    REPORT_ID_SYSTEM_CONTROL,
  };

#pragma pack(1)
  struct hid_mouse_report_ex_t
  {
    uint8_t buttons;
    int16_t x;
    int16_t y;
    int8_t wheel;
    int8_t pan;
  };

  struct hid_radial_controller_report_t
  {
    bool button : 1;
    int16_t dial : 15;
  };
#pragma pack()

  // clang-format on

  extern uint8_t const hid_report_descriptor[260];

} // namespace hidpg
