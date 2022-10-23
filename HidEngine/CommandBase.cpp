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

#include "CommandBase.h"

namespace hidpg
{

  //------------------------------------------------------------------+
  // Command
  //------------------------------------------------------------------+
  Command::Command() : _is_pressed(false)
  {
  }

  void Command::press(uint8_t n_times)
  {
    if (_is_pressed == false)
    {
      _is_pressed = true;
      onPress(n_times);
    }
  }

  uint8_t Command::release()
  {
    uint8_t result = 1;

    if (_is_pressed)
    {
      _is_pressed = false;
      result = onRelease();
    }

    return result;
  }

  //------------------------------------------------------------------+
  // BeforeOtherKeyPressEventListener
  //------------------------------------------------------------------+
  BeforeOtherKeyPressEventListener::BeforeOtherKeyPressEventListener(Command *command) : _command(command), _is_listen(false)
  {
  }

  bool BeforeOtherKeyPressEventListener::startListenBeforeOtherKeyPress()
  {
    if (_is_listen)
    {
      return false;
    }

    _listener_list().push_back(*this);
    _is_listen = true;

    return true;
  }

  bool BeforeOtherKeyPressEventListener::stopListenBeforeOtherKeyPress()
  {
    if (_is_listen == false)
    {
      return false;
    }

    auto i_item = List::iterator(*this);
    _listener_list().erase(i_item);
    _is_listen = false;

    return true;
  }

  void BeforeOtherKeyPressEventListener::_notifyBeforeOtherKeyPress(uint8_t key_id)
  {
    for (auto &listener : _listener_list())
    {
      if (listener._command->getKeyId() != key_id)
      {
        listener.onBeforeOtherKeyPress(key_id);
      }
    }
  }

  //------------------------------------------------------------------+
  // BeforeMovePointerEventListener
  //------------------------------------------------------------------+
  BeforeMovePointerEventListener::BeforeMovePointerEventListener() : _is_listen(false)
  {
  }

  bool BeforeMovePointerEventListener::startListenBeforeMovePointer()
  {
    if (_is_listen)
    {
      return false;
    }

    _listener_list().push_back(*this);
    _is_listen = true;

    return true;
  }

  bool BeforeMovePointerEventListener::stopListenBeforeMovePointer()
  {
    if (_is_listen == false)
    {
      return false;
    }

    auto i_item = List::iterator(*this);
    _listener_list().erase(i_item);
    _is_listen = false;

    return true;
  }

  void BeforeMovePointerEventListener::_notifyBeforeMovePointer(PointingDeviceId pointing_device_id, int16_t delta_x, int16_t delta_y)
  {
    for (BeforeMovePointerEventListener &listener : _listener_list())
    {
      listener.onBeforeMovePointer(pointing_device_id, delta_x, delta_y);
    }
  }

  //------------------------------------------------------------------+
  // BeforeRotateEncoderEventListener
  //------------------------------------------------------------------+
  BeforeRotateEncoderEventListener::BeforeRotateEncoderEventListener() : _is_listen(false)
  {
  }

  bool BeforeRotateEncoderEventListener::startListenBeforeRotateEncoder()
  {
    if (_is_listen)
    {
      return false;
    }

    _listener_list().push_back(*this);
    _is_listen = true;

    return true;
  }

  bool BeforeRotateEncoderEventListener::stopListenBeforeRotateEncoder()
  {
    if (_is_listen == false)
    {
      return false;
    }

    auto i_item = List::iterator(*this);
    _listener_list().erase(i_item);
    _is_listen = false;

    return true;
  }

  void BeforeRotateEncoderEventListener::_notifyBeforeRotateEncoder(EncoderId encoder_id, int16_t step)
  {
    for (BeforeRotateEncoderEventListener &listener : _listener_list())
    {
      listener.onBeforeRotateEncoder(encoder_id, step);
    }
  }

  //------------------------------------------------------------------+
  // BeforeGestureEventListener
  //------------------------------------------------------------------+
  BeforeGestureEventListener::BeforeGestureEventListener() : _is_listen(false)
  {
  }

  bool BeforeGestureEventListener::startListenBeforeGesture()
  {
    if (_is_listen)
    {
      return false;
    }

    _listener_list().push_back(*this);
    _is_listen = true;

    return true;
  }

  bool BeforeGestureEventListener::stopListenBeforeGesture()
  {
    if (_is_listen == false)
    {
      return false;
    }

    auto i_item = List::iterator(*this);
    _listener_list().erase(i_item);
    _is_listen = false;

    return true;
  }

  void BeforeGestureEventListener::_notifyBeforeGesture(GestureId gesture_id, PointingDeviceId pointing_device_id)
  {
    for (BeforeGestureEventListener &listener : _listener_list())
    {
      listener.onBeforeGesture(gesture_id, pointing_device_id);
    }
  }

} // namespace hidpg
