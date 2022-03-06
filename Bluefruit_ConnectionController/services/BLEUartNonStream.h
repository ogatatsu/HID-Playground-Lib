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

#include "BLECharacteristic.h"
#include "BLEService.h"

namespace hidpg
{

  class BLEUartNonStream : public BLEService
  {
  public:
    using rx_hdl_callback_t = void (*)(uint16_t conn_hdl, uint8_t *data, uint16_t len);
    using notify_hdl_callback_t = void (*)(uint16_t conn_hdl, bool enabled);

    using rx_callback_t = void (*)(uint8_t *data, uint16_t len);
    using notify_callback_t = void (*)(bool enabled);

    BLEUartNonStream();

    virtual err_t begin(void);

    bool notifyEnabled(void);
    bool notifyEnabled(uint16_t conn_hdl);

    void setRxCallback(rx_callback_t fp);
    void setNotifyCallback(notify_callback_t fp);

    void setRxCallback(rx_hdl_callback_t fp);
    void setNotifyCallback(notify_hdl_callback_t fp);

    uint16_t write(const uint8_t *content, uint16_t len);
    uint16_t write(uint16_t conn_hdl, const uint8_t *content, uint16_t len);

  protected:
    BLECharacteristic _txd;
    BLECharacteristic _rxd;

    // Callbacks
    rx_hdl_callback_t _rx_hdl_cb;
    notify_hdl_callback_t _notify_hdl_cb;
    rx_callback_t _rx_cb;
    notify_callback_t _notify_cb;

    // Static Method for callbacks
    static void bleuart_rxd_cb(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len);
    static void bleuart_txd_cccd_cb(uint16_t conn_hdl, BLECharacteristic *chr, uint16_t value);
  };

} // namespace hidpg
