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
#include "LinkedList.h"
#include "TimerMixin.h"
#include <stddef.h>

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

    static void _notifyBeforeMouseMove();

  protected:
    bool isLastPressed();
    virtual void onPress(uint8_t n_times) {}
    virtual uint8_t onRelease() { return 1; }
    virtual void onBeforeDifferentRootCommandPress() {}
    virtual void onBeforeMouseMove() {}
    void addEventListener_BeforeDifferentRootCommandPress();
    void addEventListener_BeforeMouseMove();

  private:
    // keymap(グローバル変数)の定義で特定のコマンドがnewされたときにaddEventListener_BeforeInput()が呼ばれる
    // _listenerListはその内部で使用するので単純なstatic変数にすると初期化順序が問題になる。
    // https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use
    static LinkedList<Command *> &_bdrcp_listener_list()
    {
      static LinkedList<Command *> *list = new LinkedList<Command *>;
      return *list;
    };

    static LinkedList<Command *> &_bmm_listener_list()
    {
      static LinkedList<Command *> *list = new LinkedList<Command *>;
      return *list;
    };

    static Command *_last_pressed_command;

    Command *_parent;
    bool _prev_state;
  };

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
  // ModifierTap
  //------------------------------------------------------------------+
  class ModifierTap : public Command
  {
  public:
    ModifierTap(Modifiers modifiers, Command *command);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const Modifiers _modifiers;
    Command *_command;
  };

  //------------------------------------------------------------------+
  // OneShotModifier
  //------------------------------------------------------------------+
  class OneShotModifier : public Command
  {
  public:
    OneShotModifier(Modifiers modifiers);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const Modifiers _modifiers;
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
  // LayerTap
  //------------------------------------------------------------------+
  class LayerTap : public Command
  {
  public:
    LayerTap(LayerClass *layer, uint8_t layer_number, Command *command);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    LayerClass *_layer;
    const uint8_t _layer_number;
    Command *_command;
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
  // OneShotLayer
  //------------------------------------------------------------------+
  class OneShotLayer : public Command
  {
  public:
    OneShotLayer(LayerClass *layer, uint8_t layer_number);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    LayerClass *_layer;
    const uint8_t _layer_number;
    layer_bitmap_t _chained_osl;
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

  //------------------------------------------------------------------+
  // TapDance
  //------------------------------------------------------------------+
  class TapDance : public Command, public TimerMixin
  {
  public:
    struct Pair
    {
      Command *tap_command;
      Command *hold_command;
    };

    TapDance(Pair pairs[], int8_t len, bool confirm_command_with_mouse_move);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;
    void onTimer() override;
    void onBeforeDifferentRootCommandPress() override;
    void onBeforeMouseMove() override;
    void onBeforeInput();

  private:
    enum class State : uint8_t
    {
      Unexecuted,
      Unfixed,
      Tap_or_NextCommand,
      FixedToHold,
    };

    Pair *const _pairs;
    Command *_running_command;
    const int8_t _len;
    int8_t _idx_count;
    State _state;
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
      Unfixed,
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

} // namespace hidpg
