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

#include "BleClientKoneProAir.h"
#include "bluefruit.h"

namespace hidpg
{

  BleClientKoneProAir::BleClientKoneProAir(void)
      : BLEClientService(UUID16_SVC_HUMAN_INTERFACE_DEVICE),
        _mouse_cb(nullptr),
        _mouse_input(UUID16_CHR_REPORT)
  {
  }

  bool BleClientKoneProAir::begin()
  {
    // Invoke base class begin()
    BLEClientService::begin();

    _mouse_input.begin(this);

    // set notify callback
    _mouse_input.setNotifyCallback(mouse_client_notify_cb);

    return true;
  }

  void BleClientKoneProAir::setMouseReportCallback(mouse_callback_t fp)
  {
    _mouse_cb = fp;
  }

  bool BleClientKoneProAir::discover(uint16_t conn_handle)
  {
    // Call Base class discover
    VERIFY(BLEClientService::discover(conn_handle));
    _conn_hdl = BLE_CONN_HANDLE_INVALID; // make as invalid until we found all chars

    // Discover all characteristics
    Bluefruit.Discovery.discoverCharacteristic(conn_handle, _mouse_input);

    VERIFY(_mouse_input.discovered());

    _conn_hdl = conn_handle;
    return true;
  }

  //------------------------------------------------------------------+
  // Mouse
  //------------------------------------------------------------------+
  bool BleClientKoneProAir::enableMouse()
  {
    return _mouse_input.enableNotify();
  }

  bool BleClientKoneProAir::disableMouse()
  {
    return _mouse_input.disableNotify();
  }

  void BleClientKoneProAir::_handle_mouse_input(uint8_t *data, uint16_t len)
  {
    if (_mouse_cb && (len == sizeof(kone_pro_air_mouse_report_t)))
    {
      _mouse_cb(reinterpret_cast<kone_pro_air_mouse_report_t *>(data));
    }
  }

  void BleClientKoneProAir::mouse_client_notify_cb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
  {
    BleClientKoneProAir &svc = (BleClientKoneProAir &)chr->parentService();
    svc._handle_mouse_input(data, len);
  }

} // namespace hidpg
