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

#include "BLEUartNonStream.h"
#include "bluefruit.h"

namespace hidpg
{

  // Constructor
  BLEUartNonStream::BLEUartNonStream()
      : BLEService(BLEUART_UUID_SERVICE),
        _txd(BLEUART_UUID_CHR_TXD),
        _rxd(BLEUART_UUID_CHR_RXD),
        _rx_cb(nullptr),
        _notify_cb(nullptr)
  {
  }

  // Callback when received new data
  void BLEUartNonStream::bleuart_rxd_cb(uint16_t conn_hdl, BLECharacteristic *chr, uint8_t *data, uint16_t len)
  {
    BLEUartNonStream &svc = (BLEUartNonStream &)chr->parentService();

    // invoke user callback
    if (svc._rx_hdl_cb)
    {
      svc._rx_hdl_cb(conn_hdl, data, len);
    }
    if (svc._rx_cb)
    {
      svc._rx_cb(data, len);
    }
  }

  void BLEUartNonStream::bleuart_txd_cccd_cb(uint16_t conn_hdl, BLECharacteristic *chr, uint16_t value)
  {
    BLEUartNonStream &svc = (BLEUartNonStream &)chr->parentService();

    if (svc._notify_hdl_cb)
    {
      svc._notify_hdl_cb(conn_hdl, value & BLE_GATT_HVX_NOTIFICATION);
    }
    if (svc._notify_cb)
    {
      svc._notify_cb(value & BLE_GATT_HVX_NOTIFICATION);
    }
  }

  void BLEUartNonStream::setRxCallback(rx_callback_t fp)
  {
    _rx_cb = fp;
  }

  void BLEUartNonStream::setRxCallback(rx_hdl_callback_t fp)
  {
    _rx_hdl_cb = fp;
  }

  void BLEUartNonStream::setNotifyCallback(notify_callback_t fp)
  {
    _notify_cb = fp;
    _txd.setCccdWriteCallback(fp ? BLEUartNonStream::bleuart_txd_cccd_cb : NULL);
  }

  void BLEUartNonStream::setNotifyCallback(notify_hdl_callback_t fp)
  {
    _notify_hdl_cb = fp;
    _txd.setCccdWriteCallback(fp ? BLEUartNonStream::bleuart_txd_cccd_cb : NULL);
  }

  err_t BLEUartNonStream::begin(void)
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
    _rxd.setWriteCallback(BLEUartNonStream::bleuart_rxd_cb);

    // TODO enable encryption when bonding is enabled
    _rxd.setPermission(SECMODE_NO_ACCESS, SECMODE_OPEN);
    _rxd.setMaxLen(max_mtu);
    _rxd.setUserDescriptor("RXD");
    VERIFY_STATUS(_rxd.begin());

    return ERROR_NONE;
  }

  bool BLEUartNonStream::notifyEnabled(void)
  {
    return this->notifyEnabled(Bluefruit.connHandle());
  }

  bool BLEUartNonStream::notifyEnabled(uint16_t conn_hdl)
  {
    return _txd.notifyEnabled(conn_hdl);
  }

  uint16_t BLEUartNonStream::write(const uint8_t *content, uint16_t len)
  {
    return this->write(Bluefruit.connHandle(), content, len);
  }

  uint16_t BLEUartNonStream::write(uint16_t conn_hdl, const uint8_t *content, uint16_t len)
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

} // namespace hidpg
