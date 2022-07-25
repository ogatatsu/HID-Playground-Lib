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
#include <stddef.h>

#define BEFORE_DIFFERENT_ROOT_COMMAND_PRESS_EVENT_LISTENER_LINK_ID 0
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

  //------------------------------------------------------------------+
  // BeforeDifferentRootCommandPressEventListener
  //------------------------------------------------------------------+
  typedef etl::bidirectional_link<BEFORE_DIFFERENT_ROOT_COMMAND_PRESS_EVENT_LISTENER_LINK_ID> BeforeDifferentRootCommandPressEventListenerLink;

  class BeforeDifferentRootCommandPressEventListener : public BeforeDifferentRootCommandPressEventListenerLink
  {
  public:
    BeforeDifferentRootCommandPressEventListener(Command *command);
    static void _notifyBeforeDifferentRootCommandPress(Command &press_command);

  protected:
    bool startListenBeforeDifferentRootCommandPress();
    bool stopListenBeforeDifferentRootCommandPress();
    virtual void onBeforeDifferentRootCommandPress(Command &command) = 0;

  private:
    static Command *getRootCommand(Command *command);

    // keymap(グローバル変数)の定義で特定のコマンドがnewされたときにコンストラクタ内でstartListenBeforeDifferentRootCommandPress()が呼ばれる、
    // _listener_listはその内部で使用するので単純なstatic変数にすると初期化順序が問題となる可能性がある。
    // https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use
    static etl::intrusive_list<BeforeDifferentRootCommandPressEventListener, BeforeDifferentRootCommandPressEventListenerLink> &_listener_list()
    {
      static etl::intrusive_list<BeforeDifferentRootCommandPressEventListener, BeforeDifferentRootCommandPressEventListenerLink> list;
      return list;
    };

    Command *_command;
    bool _is_listen;
  };

  //------------------------------------------------------------------+
  // BeforeMouseMoveEventListener
  //------------------------------------------------------------------+
  typedef etl::bidirectional_link<BEFORE_MOUSE_MOVE_EVENT_LISTENER_LINK_ID> BeforeMouseMoveEventListenerLink;

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
    // Construct On First Use Idiom
    static etl::intrusive_list<BeforeMouseMoveEventListener, BeforeMouseMoveEventListenerLink> &_listener_list()
    {
      static etl::intrusive_list<BeforeMouseMoveEventListener, BeforeMouseMoveEventListenerLink> list;
      return list;
    };

    bool _is_listen;
  };

  //------------------------------------------------------------------+
  // BeforeGestureEventListener
  //------------------------------------------------------------------+
  typedef etl::bidirectional_link<BEFORE_GESTURE_EVENT_LISTENER_LINK_ID> BeforeGestureEventListenerLink;

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
    // Construct On First Use Idiom
    static etl::intrusive_list<BeforeGestureEventListener, BeforeGestureEventListenerLink> &_listener_list()
    {
      static etl::intrusive_list<BeforeGestureEventListener, BeforeGestureEventListenerLink> list;
      return list;
    };

    bool _is_listen;
  };

  //------------------------------------------------------------------+
  // CommandHook
  //------------------------------------------------------------------+
  typedef etl::bidirectional_link<COMMAND_HOOK_LINK_ID> CommandHookLink;

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
    // Construct On First Use Idiom
    static etl::intrusive_list<CommandHook, CommandHookLink> &_hooker_list()
    {
      static etl::intrusive_list<CommandHook, CommandHookLink> list;
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
      Layering(LayerClass *layer, Command *commands[HID_ENGINE_LAYER_SIZE]);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      LayerClass *_layer;
      Command **const _commands;
      Command *_running_command;
    };

    //------------------------------------------------------------------+
    // ToggleLayer
    //------------------------------------------------------------------+
    class ToggleLayer : public Command
    {
    public:
      ToggleLayer(LayerClass *layer, uint8_t layer_number);

    protected:
      void onPress(uint8_t n_times) override;

    private:
      LayerClass *_layer;
      const uint8_t _layer_number;
    };

    //------------------------------------------------------------------+
    // SwitchLayer
    //------------------------------------------------------------------+
    class SwitchLayer : public Command
    {
    public:
      SwitchLayer(LayerClass *layer, uint8_t layer_number);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      LayerClass *_layer;
      const uint8_t _layer_number;
    };

    //------------------------------------------------------------------+
    // UpDefaultLayer
    //------------------------------------------------------------------+
    class UpDefaultLayer : public Command
    {
    public:
      UpDefaultLayer(LayerClass *layer, uint8_t i);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      LayerClass *_layer;
      const uint8_t _i;
    };

    // ------------------------------------------------------------------+
    // Tap
    // ------------------------------------------------------------------+
    class Tap : public Command
    {
    public:
      Tap(Command *command, uint8_t n_times, uint16_t tap_speed_ms);

    protected:
      void onPress(uint8_t n_times) override;

    private:
      Command *_command;
      const uint8_t _n_times;
      const uint16_t _tap_speed_ms;
    };

    // ------------------------------------------------------------------+
    // TapWhenReleased
    // ------------------------------------------------------------------+
    class TapWhenReleased : public Command
    {
    public:
      TapWhenReleased(Command *command, uint8_t n_times, uint16_t tap_speed_ms);

    protected:
      uint8_t onRelease() override;

    private:
      Command *_command;
      const uint8_t _n_times;
      const uint16_t _tap_speed_ms;
    };

    //------------------------------------------------------------------+
    // TapDance
    //------------------------------------------------------------------+
    class TapDance : public Command,
                     public TimerMixin,
                     public BeforeDifferentRootCommandPressEventListener,
                     public CommandHook
    {
    public:
      struct Pair
      {
        Command *tap_command;
        Command *hold_command;
      };

      TapDance(Pair pairs[], uint8_t len, TapHoldBehavior behavior);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;
      void onTimer() override;
      void onBeforeDifferentRootCommandPress(Command &command) override;
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
        Tap_or_NextCommand,
        FixedToHold,
      };

      Command *_running_command;
      Command *_hooked_command;
      Pair *const _pairs;
      const uint8_t _len;
      const TapHoldBehavior _behavior;
      int16_t _idx_count;
      State _state;
    };

    //------------------------------------------------------------------+
    // TapDanceDetermineWithMouseMove
    //------------------------------------------------------------------+
    class TapDanceDetermineWithMouseMove : public Command,
                                           public TimerMixin,
                                           public BeforeDifferentRootCommandPressEventListener,
                                           public BeforeMouseMoveEventListener,
                                           public CommandHook
    {
    public:
      struct Pair
      {
        Command *tap_command;
        Command *hold_command;
      };

      TapDanceDetermineWithMouseMove(Pair pairs[], uint8_t len, uint8_t mouse_ids[], uint8_t mouse_ids_len, uint16_t determine_threshold, TapHoldBehavior behavior);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    protected:
      void onTimer() override;
      void onBeforeDifferentRootCommandPress(Command &command) override;
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
        Tap_or_NextCommand,
        FixedToHold,
      };

      Command *_running_command;
      Command *_hooked_command;
      Pair *const _pairs;
      const uint8_t _len;
      uint8_t *_mouse_ids;
      const uint8_t _mouse_ids_len;
      const TapHoldBehavior _behavior;
      int16_t _idx_count;
      State _state;
      const uint16_t _determine_threshold;
      int32_t _delta_x_sum;
      int32_t _delta_y_sum;
    };

    //------------------------------------------------------------------+
    // TapOrHold
    //------------------------------------------------------------------+
    class TapOrHold : public Command, public TimerMixin
    {
    public:
      TapOrHold(Command *tap_command, unsigned int ms, Command *hold_command);

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

      const unsigned int _ms;
      State _state;
      Command *const _tap_command;
      Command *const _hold_command;
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
      class Mover : public TimerMixin
      {
      public:
        Mover();
        void setXY(int16_t x, int16_t y);
        void unsetXY(int16_t x, int16_t y);

      protected:
        void onTimer() override;

      private:
        void calcXY(int16_t &x, int16_t &y);

        int _total_x;
        int _total_y;
        uint8_t _count;
      };

      static Mover _mover;

      const int16_t _x;
      const int16_t _y;
    };

    //------------------------------------------------------------------+
    // MouseSpeed
    //------------------------------------------------------------------+
    class MouseSpeed : public Command
    {
    public:
      MouseSpeed(int16_t percent);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      const int16_t _percent;
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
      OnceEvery(uint32_t ms, Command *command);
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      uint32_t _ms;
      Command *_command;
      uint32_t _last_press_millis;
      bool _has_pressed;
      uint8_t _n_times;
    };

    //------------------------------------------------------------------+
    // TapSpacing
    //------------------------------------------------------------------+
    class TapSpacing : public Command
    {
    public:
      TapSpacing(uint32_t reference_ms, Command *command);
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      uint32_t _reference_ms;
      Command *_command;
      uint32_t _last_press_millis;
      bool _has_pressed;
      uint8_t _n_times;
    };

    //------------------------------------------------------------------+
    // If
    //------------------------------------------------------------------+
    class If : public Command
    {
    public:
      If(bool (*func)(), Command *true_command, Command *false_command);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      bool (*const _func)();
      Command *const _true_command;
      Command *const _false_command;
      Command *_running_command;
    };

    //------------------------------------------------------------------+
    // Multi
    //------------------------------------------------------------------+
    class Multi : public Command
    {
    public:
      Multi(Command *commands[], uint8_t len);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      Command **const _commands;
      const uint8_t _len;
    };

    //------------------------------------------------------------------+
    // NoOperation
    //------------------------------------------------------------------+
    class NoOperation : public Command
    {

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      uint8_t _n_times;
    };

  } // namespace Internal

} // namespace hidpg
