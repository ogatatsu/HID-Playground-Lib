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

#include "BLEClientBTTB179Hid.h"
#include "bluefruit.h"

namespace hidpg
{

  BLEClientBTTB179Hid::BLEClientBTTB179Hid(void)
      : BLEClientService(UUID16_SVC_HUMAN_INTERFACE_DEVICE),
        _trackball_cb(nullptr),
        _trackball_input(UUID16_CHR_REPORT)
  {
  }

  bool BLEClientBTTB179Hid::begin()
  {
    // Invoke base class begin()
    BLEClientService::begin();

    _trackball_input.begin(this);

    // set notify callback
    _trackball_input.setNotifyCallback(trackball_client_notify_cb);

    return true;
  }

  void BLEClientBTTB179Hid::setTrackballReportCallback(trackball_callback_t fp)
  {
    _trackball_cb = fp;
  }

  bool BLEClientBTTB179Hid::discover(uint16_t conn_handle)
  {
    // Call Base class discover
    VERIFY(BLEClientService::discover(conn_handle));
    _conn_hdl = BLE_CONN_HANDLE_INVALID; // make as invalid until we found all chars

    // Discover all characteristics
    Bluefruit.Discovery.discoverCharacteristic(conn_handle, _trackball_input);

    VERIFY(_trackball_input.discovered());

    _conn_hdl = conn_handle;
    return true;
  }

  //------------------------------------------------------------------+
  // Trackball
  //------------------------------------------------------------------+
  bool BLEClientBTTB179Hid::enableTrackball()
  {
    return _trackball_input.enableNotify();
  }

  bool BLEClientBTTB179Hid::disableTrackball()
  {
    return _trackball_input.disableNotify();
  }

  void BLEClientBTTB179Hid::_handle_trackball_input(uint8_t *data, uint16_t len)
  {
    if (_trackball_cb && (len == sizeof(trackball_report_t)))
    {
      trackball_report_t *report = (trackball_report_t *)data;
      _trackball_cb(report->buttons, report->x, report->y, report->wheel, report->pan);
    }
  }

  void BLEClientBTTB179Hid::trackball_client_notify_cb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
  {
    BLEClientBTTB179Hid &svc = (BLEClientBTTB179Hid &)chr->parentService();
    svc._handle_trackball_input(data, len);
  }

} // namespace hidpg
