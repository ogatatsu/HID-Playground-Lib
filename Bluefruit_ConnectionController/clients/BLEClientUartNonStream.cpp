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

#include "BLEClientUartNonStream.h"
#include "bluefruit.h"

namespace hidpg
{

  BLEClientUartNonStream::BLEClientUartNonStream()
      : BLEClientService(BLEUART_UUID_SERVICE),
        _txd(BLEUART_UUID_CHR_TXD),
        _rxd(BLEUART_UUID_CHR_RXD),
        _rx_cb(nullptr)
  {
  }

  bool BLEClientUartNonStream::begin()
  {
    // Invoke base class begin()
    BLEClientService::begin();

    _rxd.begin(this);
    _txd.begin(this);

    // set up notify callback
    _txd.setNotifyCallback(bleuart_client_notify_cb);

    return true;
  }

  bool BLEClientUartNonStream::enableTXD()
  {
    return _txd.enableNotify();
  }

  bool BLEClientUartNonStream::disableTXD()
  {
    return _txd.disableNotify();
  }

  void BLEClientUartNonStream::setRxCallback(rx_callback_t fp)
  {
    _rx_cb = fp;
  }

  bool BLEClientUartNonStream::discover(uint16_t conn_handle)
  {
    // Call Base class discover
    VERIFY(BLEClientService::discover(conn_handle));
    _conn_hdl = BLE_CONN_HANDLE_INVALID; // make as invalid until we found all chars

    // Discover TXD, RXD characteristics
    VERIFY(2 == Bluefruit.Discovery.discoverCharacteristic(conn_handle, _rxd, _txd));

    _conn_hdl = conn_handle;
    return true;
  }

  void BLEClientUartNonStream::bleuart_client_notify_cb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
  {
    BLEClientUartNonStream &uart_svc = (BLEClientUartNonStream &)chr->parentService();

    // invoke callback
    if (uart_svc._rx_cb)
    {
      uart_svc._rx_cb(data, len);
    }
  }

  uint16_t BLEClientUartNonStream::write(const uint8_t *content, uint16_t len)
  {
    BLEConnection *conn = Bluefruit.Connection(this->connHandle());
    VERIFY(conn, 0);

    uint16_t const max_payload = conn->getMtu() - 3;
    len = min(max_payload, len);

    // write without response
    return _rxd.write(content, len);
  }

} // namespace hidpg
