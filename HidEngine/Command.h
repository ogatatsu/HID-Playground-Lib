/*
  The MIT License (MIT)

  Copyright (c) 2019 ogatatsu.

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

#include "HidEngine_config.h"
#include "KeyCode.h"
#include "Layer.h"
#include "TimerMixin.h"
#include "etl/intrusive_links.h"
#include "etl/intrusive_list.h"
#include "etl/optional.h"
#include "etl/span.h"
#include "gsl/gsl-lite.hpp"
#include <stddef.h>

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
      Pressed,
      Released,
    };

    Command *_parent;
    State _state;
    bool _notified;
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

  enum class TapHoldBehavior
  {
    HoldPreferred,
    Balanced,
  };

  namespace Internal
  {

    //------------------------------------------------------------------+
    // NormalKey
    //------------------------------------------------------------------+
    class NormalKey : public Command
    {
    public:
      NormalKey(KeyCode key_code);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      const KeyCode _key_code;
    };

    //------------------------------------------------------------------+
    // ModifierKey
    //------------------------------------------------------------------+
    class ModifierKey : public Command
    {
    public:
      ModifierKey(Modifiers modifiers);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      const Modifiers _modifiers;
    };

    //------------------------------------------------------------------+
    // CombinationKey
    //------------------------------------------------------------------+
    class CombinationKey : public Command
    {
    public:
      CombinationKey(Modifiers modifiers, KeyCode key_code);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      const Modifiers _modifiers;
      const KeyCode _key_code;
    };

    //------------------------------------------------------------------+
    // Layering
    //------------------------------------------------------------------+
    class Layering : public Command
    {
    public:
      Layering(LayerClass &layer, etl::span<CommandPtr> commands);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      LayerClass &_layer;
      const etl::span<CommandPtr> _commands;
      Command *_running_command;
    };

    //------------------------------------------------------------------+
    // ToggleLayer
    //------------------------------------------------------------------+
    class ToggleLayer : public Command
    {
    public:
      ToggleLayer(LayerClass &layer, uint8_t layer_number);

    protected:
      void onPress(uint8_t n_times) override;

    private:
      LayerClass &_layer;
      const uint8_t _layer_number;
    };

    //------------------------------------------------------------------+
    // SwitchLayer
    //------------------------------------------------------------------+
    class SwitchLayer : public Command
    {
    public:
      SwitchLayer(LayerClass &layer, uint8_t layer_number);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      LayerClass &_layer;
      const uint8_t _layer_number;
    };

    //------------------------------------------------------------------+
    // UpDefaultLayer
    //------------------------------------------------------------------+
    class UpDefaultLayer : public Command
    {
    public:
      UpDefaultLayer(LayerClass &layer, uint8_t i);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      LayerClass &_layer;
      const uint8_t _i;
    };

    // ------------------------------------------------------------------+
    // Tap
    // ------------------------------------------------------------------+
    class Tap : public Command
    {
    public:
      Tap(NotNullCommandPtr command, uint8_t n_times, uint16_t tap_speed_ms);

    protected:
      void onPress(uint8_t n_times) override;

    private:
      const NotNullCommandPtr _command;
      const uint8_t _n_times;
      const uint16_t _tap_speed_ms;
    };

    // ------------------------------------------------------------------+
    // TapWhenReleased
    // ------------------------------------------------------------------+
    class TapWhenReleased : public Command
    {
    public:
      TapWhenReleased(NotNullCommandPtr command, uint8_t n_times, uint16_t tap_speed_ms);

    protected:
      uint8_t onRelease() override;

    private:
      const NotNullCommandPtr _command;
      const uint8_t _n_times;
      const uint16_t _tap_speed_ms;
    };

    //------------------------------------------------------------------+
    // TapDance
    //------------------------------------------------------------------+
    class TapDance : public Command,
                     public TimerMixin,
                     public BeforeOtherCommandPressEventListener,
                     public BeforeMouseMoveEventListener,
                     public CommandHook
    {
    public:
      struct Pair
      {
        Pair(NotNullCommandPtr hold_command, CommandPtr tap_command = nullptr) : hold_command(hold_command), tap_command(tap_command) {}
        const NotNullCommandPtr hold_command;
        const CommandPtr tap_command;
      };

      TapDance(etl::span<Pair> pairs, etl::span<uint8_t> mouse_ids, uint16_t move_threshold, TapHoldBehavior behavior);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    protected:
      void onTimer() override;
      void onBeforeOtherCommandPress(Command &command) override;
      void onBeforeMouseMove(uint8_t mouse_id, int16_t delta_x, int16_t delta_y) override;
      void onHookPress() override;
      void onHookRelease() override;

    private:
      void processTap();
      void processHoldPress();
      void processHoldRelease();

      enum class State : uint8_t
      {
        Unexecuted,
        Pressed,
        Hook,
        TapOrNextCommand,
        DecidedToHold,
      };

      Command *_running_command;
      Command *_hooked_command;
      const etl::span<Pair> _pairs;
      const etl::span<uint8_t> _mouse_ids;
      const uint16_t _move_threshold;
      const TapHoldBehavior _behavior;
      int16_t _delta_x_sum;
      int16_t _delta_y_sum;
      size_t _idx_count;
      State _state;
    };

    //------------------------------------------------------------------+
    // TapOrHold
    //------------------------------------------------------------------+
    class TapOrHold : public Command, public TimerMixin
    {
    public:
      TapOrHold(NotNullCommandPtr tap_command, unsigned int ms, NotNullCommandPtr hold_command);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;
      void onTimer() override;

    private:
      enum class State : uint8_t
      {
        Unexecuted,
        Pressed,
        FixedToHold,
      };

      const NotNullCommandPtr _tap_command;
      const NotNullCommandPtr _hold_command;
      const unsigned int _ms;
      State _state;
    };

    //------------------------------------------------------------------+
    // ConsumerControl
    //------------------------------------------------------------------+
    class ConsumerControl : public Command
    {
    public:
      ConsumerControl(ConsumerControlCode usage_code);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      const ConsumerControlCode _usage_code;
    };

    //------------------------------------------------------------------+
    // SystemControl
    //------------------------------------------------------------------+
    class SystemControl : public Command
    {
    public:
      SystemControl(SystemControlCode usage_code);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      const SystemControlCode _usage_code;
    };

    //------------------------------------------------------------------+
    // MouseMove
    //------------------------------------------------------------------+
    class MouseMove : public Command
    {
    public:
      MouseMove(int16_t x, int16_t y);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      const int16_t _x;
      const int16_t _y;
      const uint8_t _max_n_times;
      uint8_t _actual_n_times;
    };

    //------------------------------------------------------------------+
    // MouseScroll
    //------------------------------------------------------------------+
    class MouseScroll : public Command
    {
    public:
      MouseScroll(int8_t scroll, int8_t horiz);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      const int8_t _scroll;
      const int8_t _horiz;
      const uint8_t _max_n_times;
      uint8_t _actual_n_times;
    };

    //------------------------------------------------------------------+
    // MouseClick
    //------------------------------------------------------------------+
    class MouseClick : public Command
    {
    public:
      MouseClick(MouseButtons buttons);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      const MouseButtons _buttons;
    };

    //------------------------------------------------------------------+
    // RadialClick
    //------------------------------------------------------------------+
    class RadialClick : public Command
    {
    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;
    };

    //------------------------------------------------------------------+
    // RadialRotate
    //------------------------------------------------------------------+
    class RadialRotate : public Command
    {
    public:
      RadialRotate(int16_t deci_degree);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      const int16_t _deci_degree;
      const uint8_t _max_n_times;
      uint8_t _actual_n_times;
    };

    //------------------------------------------------------------------+
    // OnceEvery
    //------------------------------------------------------------------+
    class OnceEvery : public Command
    {
    public:
      OnceEvery(NotNullCommandPtr command, uint32_t ms);
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      const NotNullCommandPtr _command;
      const uint32_t _ms;
      uint32_t _last_press_millis;
      bool _has_pressed;
    };

    //------------------------------------------------------------------+
    // NTimesEvery
    //------------------------------------------------------------------+
    class NTimesEvery : public Command
    {
    public:
      NTimesEvery(NotNullCommandPtr command, uint32_t _ms);
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      const NotNullCommandPtr _command;
      const uint32_t _ms;
      uint32_t _last_press_millis;
      bool _has_pressed;
    };

    //------------------------------------------------------------------+
    // If
    //------------------------------------------------------------------+
    class If : public Command
    {
    public:
      If(bool (*func)(), NotNullCommandPtr true_command, NotNullCommandPtr false_command);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      bool (*const _func)();
      const NotNullCommandPtr _true_command;
      const NotNullCommandPtr _false_command;
      Command *_running_command;
    };

    //------------------------------------------------------------------+
    // Multi
    //------------------------------------------------------------------+
    class Multi : public Command
    {
    public:
      Multi(etl::span<NotNullCommandPtr> commands);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      const etl::span<NotNullCommandPtr> _commands;
    };

    //------------------------------------------------------------------+
    // Toggle
    //------------------------------------------------------------------+
    class Toggle : public Command
    {
    public:
      Toggle(NotNullCommandPtr command);

    protected:
      void onPress(uint8_t n_times) override;

    private:
      const NotNullCommandPtr _command;
      bool _is_pressed;
    };

    //------------------------------------------------------------------+
    // Repeat
    //------------------------------------------------------------------+
    class Repeat : public Command, public TimerMixin
    {
    public:
      Repeat(NotNullCommandPtr command, uint32_t delay_ms, uint32_t interval_ms);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;
      void onTimer() override;

    private:
      const NotNullCommandPtr _command;
      const uint32_t _delay_ms;
      const uint32_t _interval_ms;
    };

    //------------------------------------------------------------------+
    // Cycle
    //------------------------------------------------------------------+
    class Cycle : public Command
    {
    public:
      Cycle(etl::span<NotNullCommandPtr> commands);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      const etl::span<NotNullCommandPtr> _commands;
      size_t _idx;
    };

    //------------------------------------------------------------------+
    // CyclePhaseShift
    //------------------------------------------------------------------+
    class CyclePhaseShift : public Command
    {
    public:
      CyclePhaseShift(etl::span<NotNullCommandPtr> commands);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      const etl::span<NotNullCommandPtr> _commands;
      size_t _idx;
    };

    //------------------------------------------------------------------+
    // NoOperation
    //------------------------------------------------------------------+
    class NoOperation : public Command
    {

    protected:
      uint8_t onRelease() override;
    };

  } // namespace Internal

} // namespace hidpg
