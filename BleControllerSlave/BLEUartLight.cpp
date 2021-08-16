/**************************************************************************/
/*!
    @file     BLEUart.cpp
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

#include "BLEUartLight.h"
#include "bluefruit.h"
#include "utility/TimeoutTimer.h"

namespace hidpg::Internal
{

  // Constructor
  BLEUartLight::BLEUartLight()
      : BLEService(BLEUART_UUID_SERVICE), _txd(BLEUART_UUID_CHR_TXD), _rxd(BLEUART_UUID_CHR_RXD), _rx_cb(nullptr), _notify_cb(nullptr)
  {
  }

  // Callback when received new data
  void BLEUartLight::bleuart_rxd_cb(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len)
  {
    BLEUartLight &svc = (BLEUartLight &)chr->parentService();

#if CFG_DEBUG >= 2
    LOG_LV2("BLEUART", "RX: ");
    PRINT_BUFFER(data, len);
#endif

    // invoke user callback
    if (svc._rx_cb)
    {
      svc._rx_cb(conn_hdl, data, len);
    }
  }

  void BLEUartLight::bleuart_txd_cccd_cb(uint16_t conn_hdl, BLECharacteristic *chr, uint16_t value)
  {
    BLEUartLight &svc = (BLEUartLight &)chr->parentService();

    if (svc._notify_cb)
    {
      svc._notify_cb(conn_hdl, value & BLE_GATT_HVX_NOTIFICATION);
    }
  }

  void BLEUartLight::setRxCallback(rx_callback_t fp)
  {
    _rx_cb = fp;
  }

  void BLEUartLight::setNotifyCallback(notify_callback_t fp)
  {
    _notify_cb = fp;
    _txd.setCccdWriteCallback(fp ? BLEUartLight::bleuart_txd_cccd_cb : NULL);
  }

  err_t BLEUartLight::begin(void)
  {
    // Invoke base class begin()
    VERIFY_STATUS(BLEService::begin());

    uint16_t max_mtu = Bluefruit.getMaxMtu(BLE_GAP_ROLE_PERIPH);

    // Add TXD Characteristic
    _txd.setProperties(CHR_PROPS_NOTIFY);
    // TODO enable encryption when bonding is enabled
    _txd.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
    _txd.setMaxLen(max_mtu);
    _txd.setUserDescriptor("TXD");
    VERIFY_STATUS(_txd.begin());

    // Add RXD Characteristic
    _rxd.setProperties(CHR_PROPS_WRITE | CHR_PROPS_WRITE_WO_RESP);
    _rxd.setWriteCallback(BLEUartLight::bleuart_rxd_cb);

    // TODO enable encryption when bonding is enabled
    _rxd.setPermission(SECMODE_NO_ACCESS, SECMODE_OPEN);
    _rxd.setMaxLen(max_mtu);
    _rxd.setUserDescriptor("RXD");
    VERIFY_STATUS(_rxd.begin());

    return ERROR_NONE;
  }

  bool BLEUartLight::notifyEnabled(void)
  {
    return this->notifyEnabled(Bluefruit.connHandle());
  }

  bool BLEUartLight::notifyEnabled(uint16_t conn_hdl)
  {
    return _txd.notifyEnabled(conn_hdl);
  }

  uint16_t BLEUartLight::write(const uint8_t *content, uint16_t len)
  {
    return this->write(Bluefruit.connHandle(), content, len);
  }

  uint16_t BLEUartLight::write(uint16_t conn_hdl, const uint8_t *content, uint16_t len)
  {
    BLEConnection *conn = Bluefruit.Connection(conn_hdl);
    VERIFY(conn, 0);

    // skip if not enabled
    if (!notifyEnabled(conn_hdl))
    {
      return 0;
    }

    uint16_t max_payload = conn->getMtu() - 3;
    len = min(max_payload, len);
    return _txd.notify(conn_hdl, content, len) ? len : 0;
  }

} // namespace hidpg::Internal
