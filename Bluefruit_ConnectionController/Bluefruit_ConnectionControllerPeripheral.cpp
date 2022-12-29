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

#include "Bluefruit_ConnectionControllerPeripheral.h"

namespace hidpg
{
  namespace Internal
  {

    BLEPeripheralProfile *Bluefruit_ConnectionControllerPeripheral::_profile;
    BlinkLed *Bluefruit_ConnectionControllerPeripheral::_adv_led = nullptr;
    Bluefruit_ConnectionControllerPeripheral::cannotConnectCallback_t Bluefruit_ConnectionControllerPeripheral::_cannot_connect_cb = nullptr;
    bool Bluefruit_ConnectionControllerPeripheral::_is_running = false;

    void Bluefruit_ConnectionControllerPeripheral::begin()
    {
      Bluefruit.Periph.setConnectCallback(connect_callback);
      Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

      Bluefruit.Advertising.setStopCallback(adv_stop_callback);
      Bluefruit.Advertising.restartOnDisconnect(false);
      Bluefruit.Advertising.setInterval(32, 244); // in unit of 0.625 ms
      Bluefruit.Advertising.setFastTimeout(30);   // number of seconds in fast mode
    }

    void Bluefruit_ConnectionControllerPeripheral::setProfile(BLEPeripheralProfile *profile)
    {
      _profile = profile;
    }

    void Bluefruit_ConnectionControllerPeripheral::setAdvLed(BlinkLed *adv_led)
    {
      _adv_led = adv_led;
      if (_adv_led != nullptr)
      {
        Bluefruit.autoConnLed(false);
      }
    }

    void Bluefruit_ConnectionControllerPeripheral::start()
    {
      ada_callback(nullptr, 0, _start);
    }

    void Bluefruit_ConnectionControllerPeripheral::stop()
    {
      ada_callback(nullptr, 0, _stop);
    }

    void Bluefruit_ConnectionControllerPeripheral::_start()
    {
      if (_is_running)
      {
        return;
      }

      makeAdvData();
      if (startAdv())
      {
        _is_running = true;
      };
    }

    void Bluefruit_ConnectionControllerPeripheral::_stop()
    {
      _is_running = false;

      if (Bluefruit.Advertising.isRunning())
      {
        Bluefruit.Advertising.stop();
      }

      if (Bluefruit.Periph.connected())
      {
        Bluefruit.disconnect(Bluefruit.connHandle());
      }

      if (_adv_led != nullptr)
      {
        _adv_led->off();
      }
    }

    bool Bluefruit_ConnectionControllerPeripheral::isRunning()
    {
      return _is_running;
    }

    bool Bluefruit_ConnectionControllerPeripheral::waitReady()
    {
      uint16_t conn_hdl = Bluefruit.connHandle();
      BLEConnection *conn = Bluefruit.Connection(conn_hdl);

      if (conn != nullptr)
      {
        if (conn->getHvnPacket())
        {
          conn->releaseHvnPacket();
          return true;
        }
      }
      return false;
    }

    void Bluefruit_ConnectionControllerPeripheral::setCannotConnectCallback(cannotConnectCallback_t callback)
    {
      _cannot_connect_cb = callback;
    }

    void Bluefruit_ConnectionControllerPeripheral::makeAdvData()
    {
      Bluefruit.Advertising.clearData();

      // Advertising packet
      Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
      Bluefruit.Advertising.addTxPower();
      Bluefruit.Advertising.addAppearance(_profile->getAppearance());
      Bluefruit.Advertising.addService(_profile->getService());

      // Secondary Scan Response packet (optional)
      // Since there is no room for 'Name' in Advertising packet
      Bluefruit.ScanResponse.addName();
    }

    bool Bluefruit_ConnectionControllerPeripheral::startAdv()
    {
      if (Bluefruit.Advertising.start(60) == false)
      {
        return false;
      };

      if (_adv_led != nullptr)
      {
        _adv_led->blink();
      }

      return true;
    }

    void Bluefruit_ConnectionControllerPeripheral::adv_stop_callback()
    {
      _is_running = false;

      if (_adv_led != nullptr)
      {
        _adv_led->off();
      }

      if (_cannot_connect_cb != nullptr)
      {
        _cannot_connect_cb();
      }
    }

    void Bluefruit_ConnectionControllerPeripheral::connect_callback(uint16_t conn_handle)
    {
      BLEConnection *conn = Bluefruit.Connection(conn_handle);
      conn->requestConnectionParameter(_profile->getConnectionInterval(), _profile->getSlaveLatency(), _profile->getSupervisionTimeout());
      conn->requestPHY();

      if (_adv_led != nullptr)
      {
        _adv_led->off();
      }
    }

    void Bluefruit_ConnectionControllerPeripheral::disconnect_callback(uint16_t conn_handle, uint8_t reason)
    {
      if (_is_running)
      {
        startAdv();
      }
    }

  } // namespace Internal

} // namespace hidpg
