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

#pragma once

#include "etl/intrusive_links.h"
#include "etl/intrusive_list.h"
#include "gsl/gsl-lite.hpp"

#define BEFORE_OTHER_COMMAND_PRESS_EVENT_LISTENER_LINK_ID 0
#define BEFORE_MOUSE_MOVE_EVENT_LISTENER_LINK_ID 1
#define BEFORE_GESTURE_EVENT_LISTENER_LINK_ID 2
#define COMMAND_HOOK_LINK_ID 3

namespace hidpg
{

  //------------------------------------------------------------------+
  // Command
  //------------------------------------------------------------------+
  class Command
  {
  public:
    Command();
    void press(uint8_t n_times = 1);
    uint8_t release();
    void setParent(Command *parent) { _parent = parent; }
    Command *getParent() { return _parent; }

  protected:
    virtual void onPress(uint8_t n_times) {}
    virtual uint8_t onRelease() { return 1; }

  private:
    enum class State
    {
      Notified,
      Pressed,
      Released,
    };

    Command *_parent;
    State _state;
  };

  using CommandPtr = Command *;
  using NotNullCommandPtr = gsl::not_null<Command *>;

  //------------------------------------------------------------------+
  // BeforeOtherCommandPressEventListener
  //------------------------------------------------------------------+
  using BeforeOtherCommandPressEventListenerLink = etl::bidirectional_link<BEFORE_OTHER_COMMAND_PRESS_EVENT_LISTENER_LINK_ID>;

  class BeforeOtherCommandPressEventListener : public BeforeOtherCommandPressEventListenerLink
  {
  public:
    BeforeOtherCommandPressEventListener(Command *command);
    static void _notifyOtherCommandPress(Command &press_command);

  protected:
    bool startListenBeforeOtherCommandPress();
    bool stopListenBeforeOtherCommandPress();
    virtual void onBeforeOtherCommandPress(Command &command) = 0;

  private:
    using List = etl::intrusive_list<BeforeOtherCommandPressEventListener, BeforeOtherCommandPressEventListenerLink>;

    static Command *getRootCommand(Command *command);

    // keymap(グローバル変数)の定義で特定のコマンドがnewされたときにコンストラクタ内でstartListenBeforeOtherCommandPress()が呼ばれる、
    // _listener_listはその内部で使用するので単純なstatic変数にすると初期化順序が問題となる可能性がある。
    // https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use
    static List &_listener_list()
    {
      static List list;
      return list;
    };

    Command *_command;
    bool _is_listen;
  };

  //------------------------------------------------------------------+
  // BeforeMouseMoveEventListener
  //------------------------------------------------------------------+
  using BeforeMouseMoveEventListenerLink = etl::bidirectional_link<BEFORE_MOUSE_MOVE_EVENT_LISTENER_LINK_ID>;

  class BeforeMouseMoveEventListener : public BeforeMouseMoveEventListenerLink
  {
  public:
    BeforeMouseMoveEventListener();
    static void _notifyBeforeMouseMove(uint8_t mouse_id, int16_t delta_x, int16_t delta_y);

  protected:
    bool startListenBeforeMouseMove();
    bool stopListenBeforeMouseMove();
    virtual void onBeforeMouseMove(uint8_t mouse_id, int16_t delta_x, int16_t delta_y) = 0;

  private:
    using List = etl::intrusive_list<BeforeMouseMoveEventListener, BeforeMouseMoveEventListenerLink>;

    // Construct On First Use Idiom
    static List &_listener_list()
    {
      static List list;
      return list;
    };

    bool _is_listen;
  };

  //------------------------------------------------------------------+
  // BeforeGestureEventListener
  //------------------------------------------------------------------+
  using BeforeGestureEventListenerLink = etl::bidirectional_link<BEFORE_GESTURE_EVENT_LISTENER_LINK_ID>;

  class BeforeGestureEventListener : public BeforeGestureEventListenerLink
  {
  public:
    BeforeGestureEventListener();
    static void _notifyBeforeGesture(uint8_t gesture_id, uint8_t mouse_id);

  protected:
    bool startListenBeforeGesture();
    bool stopListenBeforeGesture();
    virtual void onBeforeGesture(uint8_t gesture_id, uint8_t mouse_id) = 0;

  private:
    using List = etl::intrusive_list<BeforeGestureEventListener, BeforeGestureEventListenerLink>;

    // Construct On First Use Idiom
    static List &_listener_list()
    {
      static List list;
      return list;
    };

    bool _is_listen;
  };

  //------------------------------------------------------------------+
  // CommandHook
  //------------------------------------------------------------------+
  using CommandHookLink = etl::bidirectional_link<COMMAND_HOOK_LINK_ID>;

  class CommandHook : public CommandHookLink
  {
  public:
    CommandHook();
    static bool _tryHookPress(Command &command);
    static bool _tryHookRelease(Command &command);
    static bool _isHooked(Command &command);

  protected:
    bool startHook(Command &command);
    bool stopHook();
    virtual void onHookPress() = 0;
    virtual void onHookRelease() = 0;

  private:
    using List = etl::intrusive_list<CommandHook, CommandHookLink>;

    // Construct On First Use Idiom
    static List &_hooker_list()
    {
      static List list;
      return list;
    };

    enum class State
    {
      Invalid,
      Pressed,
      Released,
    };

    bool _is_hook;
    Command *_hooked_command;
    State _state;
  };

  enum class HoldTapBehavior
  {
    HoldPreferred,
    Balanced,
  };

}
