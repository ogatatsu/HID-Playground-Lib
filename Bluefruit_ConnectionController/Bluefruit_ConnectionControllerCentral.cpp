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

#include "Bluefruit_ConnectionControllerCentral.h"

namespace hidpg::Internal
{

  BlinkLed *Bluefruit_ConnectionControllerCentral::_scan_led = nullptr;
  BLECentralProfile **Bluefruit_ConnectionControllerCentral::_profile_list;
  uint8_t Bluefruit_ConnectionControllerCentral::_profile_list_len = 0;
  uint8_t Bluefruit_ConnectionControllerCentral::_unconnected_count = 0;
  bool Bluefruit_ConnectionControllerCentral::_is_running = false;
  Bluefruit_ConnectionControllerCentral::disconnectCallbackCallback_t Bluefruit_ConnectionControllerCentral::_disconnect_callback = nullptr;

  void Bluefruit_ConnectionControllerCentral::begin()
  {
    Bluefruit.Central.setConnectCallback(connect_callback);
    Bluefruit.Central.setDisconnectCallback(disconnect_callback);
    Bluefruit.Security.setSecuredCallback(connection_secured_callback);

    Bluefruit.Scanner.setRxCallback(scan_callback);
    Bluefruit.Scanner.restartOnDisconnect(false);
    Bluefruit.Scanner.setInterval(8, 4); // in unit of 0.625 ms
  }

  void Bluefruit_ConnectionControllerCentral::setScanLed(BlinkLed *scan_led)
  {
    _scan_led = scan_led;
    if (_scan_led != nullptr)
    {
      Bluefruit.autoConnLed(false);
    }
  }

  void Bluefruit_ConnectionControllerCentral::start()
  {
    ada_callback(nullptr, 0, _start);
  }

  void Bluefruit_ConnectionControllerCentral::stop()
  {
    ada_callback(nullptr, 0, _stop);
  }

  void Bluefruit_ConnectionControllerCentral::_start()
  {
    if (_is_running)
    {
      return;
    }

    if (startScan())
    {
      _is_running = true;
    };
  }

  void Bluefruit_ConnectionControllerCentral::_stop()
  {
    _is_running = false;

    if (Bluefruit.Scanner.isRunning())
    {
      Bluefruit.Scanner.stop();
    }

    if (Bluefruit.Central.connected())
    {
      for (size_t i = 0; i < _profile_list_len; i++)
      {
        uint16_t conn_hdl = _profile_list[i]->connHandle();
        BLEConnection *conn = Bluefruit.Connection(conn_hdl);
        if (conn != nullptr)
        {
          conn->disconnect();
        }
      }
    }

    if (_scan_led != nullptr)
    {
      _scan_led->off();
    }
  }

  bool Bluefruit_ConnectionControllerCentral::startScan()
  {
    if (Bluefruit.Scanner.start(0) == false)
    {
      return false;
    }

    if (_scan_led != nullptr)
    {
      _scan_led->blink(_unconnected_count);
    }
    return true;
  }

  bool Bluefruit_ConnectionControllerCentral::isRunning()
  {
    return _is_running;
  }

  void Bluefruit_ConnectionControllerCentral::setDisconnectCallback(disconnectCallbackCallback_t callback)
  {
    _disconnect_callback = callback;
  }

  void Bluefruit_ConnectionControllerCentral::scan_callback(ble_gap_evt_adv_report_t *report)
  {
    for (size_t i = 0; i < _profile_list_len; i++)
    {
      if (_profile_list[i]->canConnect(report))
      {
        Bluefruit.Central.connect(report);
        return;
      }
    }

    Bluefruit.Scanner.resume();
  }

  void Bluefruit_ConnectionControllerCentral::connect_callback(uint16_t conn_handle)
  {
    _unconnected_count--;

    BLEConnection *conn = Bluefruit.Connection(conn_handle);

    for (size_t i = 0; i < _profile_list_len; i++)
    {
      if ((_profile_list[i]->discovered() == false) && _profile_list[i]->discover(conn_handle))
      {
        conn->requestPairing();
        return;
      }
    }

    conn->disconnect();
  }

  void Bluefruit_ConnectionControllerCentral::connection_secured_callback(uint16_t conn_handle)
  {
    BLEConnection *conn = Bluefruit.Connection(conn_handle);

    if (!conn->secured())
    {
      // It is possible that connection is still not secured by this time.
      // This happens (central only) when we try to encrypt connection using stored bond keys
      // but peer reject it (probably it remove its stored key).
      // Therefore we will request an pairing again --> callback again when encrypted
      conn->requestPairing();
    }
    else
    {
      for (size_t i = 0; i < _profile_list_len; i++)
      {
        if (_profile_list[i]->connHandle() == conn_handle)
        {
          _profile_list[i]->enable();
          conn->requestPHY();

          if (_unconnected_count == 0)
          {
            if (_scan_led != nullptr)
            {
              _scan_led->off();
            }
          }
          else
          {
            startScan();
          }

          return;
        }
      }
    }
  }

  void Bluefruit_ConnectionControllerCentral::disconnect_callback(uint16_t conn_handle, uint8_t reason)
  {
    _unconnected_count++;

    if (_is_running == true)
    {
      startScan();
    }
  }

} // namespace hidpg::Internal
