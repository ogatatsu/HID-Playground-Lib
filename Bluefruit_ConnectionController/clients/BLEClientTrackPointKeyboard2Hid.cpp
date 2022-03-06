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

#include "BLEClientTrackPointKeyboard2Hid.h"
#include "bluefruit.h"

namespace hidpg
{

  BLEClientTrackPointKeyboard2Hid::BLEClientTrackPointKeyboard2Hid(void)
      : BLEClientService(UUID16_SVC_HUMAN_INTERFACE_DEVICE),
        _keyboard_cb(nullptr),
        _trackpoint_cb(nullptr),
        _consumer_cb(nullptr),
        _keyboard_input(UUID16_CHR_REPORT),
        _trackpoint_input(UUID16_CHR_REPORT),
        _consumer_input(UUID16_CHR_REPORT)
  {
  }

  bool BLEClientTrackPointKeyboard2Hid::begin()
  {
    // Invoke base class begin()
    BLEClientService::begin();

    _keyboard_input.begin(this);
    _trackpoint_input.begin(this);
    _consumer_input.begin(this);

    // set notify callback
    _keyboard_input.setNotifyCallback(keyboard_client_notify_cb);
    _trackpoint_input.setNotifyCallback(trackpoint_client_notify_cb);
    _consumer_input.setNotifyCallback(consumer_client_notify_cb);

    return true;
  }

  void BLEClientTrackPointKeyboard2Hid::setKeyboardReportCallback(keyboard_callback_t fp)
  {
    _keyboard_cb = fp;
  }

  void BLEClientTrackPointKeyboard2Hid::setTrackpointReportCallback(trackpoint_callback_t fp)
  {
    _trackpoint_cb = fp;
  }

  void BLEClientTrackPointKeyboard2Hid::setConsumerReportCallback(consumer_callback_t fp)
  {
    _consumer_cb = fp;
  }

  bool BLEClientTrackPointKeyboard2Hid::discover(uint16_t conn_handle)
  {
    // Call Base class discover
    VERIFY(BLEClientService::discover(conn_handle));
    _conn_hdl = BLE_CONN_HANDLE_INVALID; // make as invalid until we found all chars

    // Discover all characteristics
    Bluefruit.Discovery.discoverCharacteristic(conn_handle, _keyboard_input, _trackpoint_input, _consumer_input);

    VERIFY(_keyboard_input.discovered() && _trackpoint_input.discovered() && _consumer_input.discovered());

    _conn_hdl = conn_handle;
    return true;
  }

  //------------------------------------------------------------------+
  // Keyboard
  //------------------------------------------------------------------+
  bool BLEClientTrackPointKeyboard2Hid::enableKeyboard()
  {
    return _keyboard_input.enableNotify();
  }

  bool BLEClientTrackPointKeyboard2Hid::disableKeyboard()
  {
    return _keyboard_input.disableNotify();
  }

  void BLEClientTrackPointKeyboard2Hid::_handle_keyboard_input(uint8_t *data, uint16_t len)
  {
    if (_keyboard_cb && (len == sizeof(keyboard_report_t)))
    {
      keyboard_report_t *report = (keyboard_report_t *)data;
      _keyboard_cb(report->modifiers, report->key_codes);
    }
  }

  void BLEClientTrackPointKeyboard2Hid::keyboard_client_notify_cb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
  {
    BLEClientTrackPointKeyboard2Hid &svc = (BLEClientTrackPointKeyboard2Hid &)chr->parentService();
    svc._handle_keyboard_input(data, len);
  }

  //------------------------------------------------------------------+
  // TrackPoint
  //------------------------------------------------------------------+
  bool BLEClientTrackPointKeyboard2Hid::enableTrackpoint()
  {
    return _trackpoint_input.enableNotify();
  }

  bool BLEClientTrackPointKeyboard2Hid::disableTrackpoint()
  {
    return _trackpoint_input.disableNotify();
  }

  void BLEClientTrackPointKeyboard2Hid::_handle_trackpoint_input(uint8_t *data, uint16_t len)
  {
    if (_trackpoint_cb && (len == sizeof(trackpoint_report_t)))
    {
      trackpoint_report_t *report = (trackpoint_report_t *)data;
      _trackpoint_cb(report->buttons, report->x, report->y, report->wheel);
    }
  }

  void BLEClientTrackPointKeyboard2Hid::trackpoint_client_notify_cb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
  {
    BLEClientTrackPointKeyboard2Hid &svc = (BLEClientTrackPointKeyboard2Hid &)chr->parentService();
    svc._handle_trackpoint_input(data, len);
  }

  //------------------------------------------------------------------+
  // Consumer
  //------------------------------------------------------------------+
  bool BLEClientTrackPointKeyboard2Hid::enableConsumer()
  {
    return _consumer_input.enableNotify();
  }

  bool BLEClientTrackPointKeyboard2Hid::disableConsumer()
  {
    return _consumer_input.disableNotify();
  }

  void BLEClientTrackPointKeyboard2Hid::_handle_consumer_input(uint8_t *data, uint16_t len)
  {
    if (_consumer_cb && (len == sizeof(consumer_report_t)))
    {
      consumer_report_t *report = (consumer_report_t *)data;
      _consumer_cb(report->usage_code);
    }
  }

  void BLEClientTrackPointKeyboard2Hid::consumer_client_notify_cb(BLEClientCharacteristic *chr, uint8_t *data, uint16_t len)
  {
    BLEClientTrackPointKeyboard2Hid &svc = (BLEClientTrackPointKeyboard2Hid &)chr->parentService();
    svc._handle_consumer_input(data, len);
  }

} // namespace hidpg
