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
  Command::Command() : _parent(nullptr), _state(State::Released)
  {
  }

  void Command::press(uint8_t n_times)
  {
    if (_state == State::Released && CommandHook::_isHooked(*this) == false)
    {
      _state = State::Notified;
      BeforeOtherCommandPressEventListener::_notifyOtherCommandPress(*this);
    }

    if (CommandHook::_tryHookPress(*this) == false)
    {
      if (_state == State::Notified && CommandHook::_isHooked(*this) == false)
      {
        _state = State::Pressed;
        onPress(n_times);
      }
    }
  }

  uint8_t Command::release()
  {
    uint8_t result = 1;

    if (CommandHook::_tryHookRelease(*this) == false)
    {
      if (_state == State::Pressed && CommandHook::_isHooked(*this) == false)
      {
        _state = State::Released;
        result = onRelease();
      }
    }

    return result;
  }

  //------------------------------------------------------------------+
  // BeforeOtherCommandPressEventListener
  //------------------------------------------------------------------+
  BeforeOtherCommandPressEventListener::BeforeOtherCommandPressEventListener(Command *command) : _command(command), _is_listen(false)
  {
  }

  bool BeforeOtherCommandPressEventListener::startListenBeforeOtherCommandPress()
  {
    if (_is_listen)
    {
      return false;
    }

    _listener_list().push_back(*this);
    _is_listen = true;

    return true;
  }

  bool BeforeOtherCommandPressEventListener::stopListenBeforeOtherCommandPress()
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

  void BeforeOtherCommandPressEventListener::_notifyOtherCommandPress(Command &press_command)
  {
    for (BeforeOtherCommandPressEventListener &listener : _listener_list())
    {
      if (getRootCommand(listener._command) != getRootCommand(&press_command))
      {
        listener.onBeforeOtherCommandPress(press_command);
      }
    }
  }

  Command *BeforeOtherCommandPressEventListener::getRootCommand(Command *command)
  {
    while (command->getParent() != nullptr)
    {
      command = command->getParent();
    }
    return command;
  }

  //------------------------------------------------------------------+
  // BeforeMouseMoveEventListener
  //------------------------------------------------------------------+
  BeforeMouseMoveEventListener::BeforeMouseMoveEventListener() : _is_listen(false)
  {
  }

  bool BeforeMouseMoveEventListener::startListenBeforeMouseMove()
  {
    if (_is_listen)
    {
      return false;
    }

    _listener_list().push_back(*this);
    _is_listen = true;

    return true;
  }

  bool BeforeMouseMoveEventListener::stopListenBeforeMouseMove()
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

  void BeforeMouseMoveEventListener::_notifyBeforeMouseMove(uint8_t mouse_id, int16_t delta_x, int16_t delta_y)
  {
    for (BeforeMouseMoveEventListener &listener : _listener_list())
    {
      listener.onBeforeMouseMove(mouse_id, delta_x, delta_y);
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

  void BeforeGestureEventListener::_notifyBeforeGesture(uint8_t gesture_id, uint8_t mouse_id)
  {
    for (BeforeGestureEventListener &listener : _listener_list())
    {
      listener.onBeforeGesture(gesture_id, mouse_id);
    }
  }

  //------------------------------------------------------------------+
  // CommandHook
  //------------------------------------------------------------------+
  CommandHook::CommandHook() : _is_hook(false), _hooked_command(nullptr), _state(State::Invalid)
  {
  }

  bool CommandHook::startHook(Command &command)
  {
    if (_is_hook)
    {
      return false;
    }

    for (CommandHook &cmd : _hooker_list())
    {
      if (cmd._hooked_command == &command)
      {
        return false;
      }
    }

    _hooked_command = &command;
    _hooker_list().push_back(*this);
    _is_hook = true;
    _state = State::Invalid;

    return true;
  }

  bool CommandHook::stopHook()
  {
    if (_is_hook == false)
    {
      return false;
    }

    _hooked_command = nullptr;
    auto i_item = List::iterator(*this);
    _hooker_list().erase(i_item);
    _is_hook = false;

    return true;
  }

  bool CommandHook::_tryHookPress(Command &command)
  {
    for (CommandHook &cmd : _hooker_list())
    {
      if (cmd._hooked_command == &command &&
          (cmd._state == State::Invalid || cmd._state == State::Released))
      {
        cmd._state = State::Pressed;
        cmd.onHookPress();
        return true;
      }
    }
    return false;
  }

  bool CommandHook::_tryHookRelease(Command &command)
  {
    for (CommandHook &cmd : _hooker_list())
    {
      if (cmd._hooked_command == &command &&
          (cmd._state == State::Invalid || cmd._state == State::Pressed))
      {
        cmd._state = State::Released;
        cmd.onHookRelease();
        return true;
      }
    }
    return false;
  }

  bool CommandHook::_isHooked(Command &command)
  {
    for (CommandHook &cmd : _hooker_list())
    {
      if (cmd._hooked_command == &command)
      {
        return true;
      }
    }
    return false;
  }

} // namespace hidpg
