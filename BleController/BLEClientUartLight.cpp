/**************************************************************************/
/*!
    @file     BLEClientUart.cpp
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

#include "BLEClientUartLight.h"
#include "bluefruit.h"

namespace hidpg
{

  void bleuart_central_notify_cb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len);

  BLEClientUartLight::BLEClientUartLight()
      : BLEClientService(BLEUART_UUID_SERVICE), _txd(BLEUART_UUID_CHR_TXD), _rxd(BLEUART_UUID_CHR_RXD), _rx_cb(nullptr)
  {
  }

  bool BLEClientUartLight::begin(void)
  {
    // Invoke base class begin()
    BLEClientService::begin();

    _rxd.begin(this);
    _txd.begin(this);

    // set up notify callback
    _txd.setNotifyCallback(bleuart_central_notify_cb);

    return true;
  }

  bool BLEClientUartLight::enableTXD(void)
  {
    return _txd.enableNotify();
  }

  bool BLEClientUartLight::disableTXD(void)
  {
    return _txd.disableNotify();
  }

  void BLEClientUartLight::setRxCallback(rx_callback_t fp)
  {
    _rx_cb = fp;
  }

  bool BLEClientUartLight::discover(uint16_t conn_handle)
  {
    // Call Base class discover
    VERIFY(BLEClientService::discover(conn_handle));
    _conn_hdl = BLE_CONN_HANDLE_INVALID; // make as invalid until we found all chars

    // Discover TXD, RXD characteristics
    VERIFY(2 == Bluefruit.Discovery.discoverCharacteristic(conn_handle, _rxd, _txd));

    _conn_hdl = conn_handle;
    return true;
  }

  void BLEClientUartLight::disconnect(void)
  {
    BLEClientService::disconnect();
  }

  void bleuart_central_notify_cb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
  {
    BLEClientUartLight &uart_svc = (BLEClientUartLight &)chr->parentService();

    // invoke callback
    if (uart_svc._rx_cb)
    {
      uart_svc._rx_cb(uart_svc.connHandle(), data, len);
    }
  }

  uint16_t BLEClientUartLight::write(const uint8_t *content, uint16_t len)
  {
    BLEConnection *conn = Bluefruit.Connection(this->connHandle());
    VERIFY(conn, 0);

    uint16_t const max_payload = conn->getMtu() - 3;
    len = min(max_payload, len);

    // write without response
    return _rxd.write(content, len);
  }

} // namespace hidpg
