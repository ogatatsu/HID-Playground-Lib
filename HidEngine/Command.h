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
    virtual void onBeforeInput() {}
    void addEventListener_BeforeInput();

  private:
    // keymap(グローバル変数)の定義で特定のコマンドがnewされたときにaddEventListener_BeforeInput()が呼ばれる
    // _listenerListはその内部で使用するので単純なstatic変数にすると初期化順序が問題になる。
    // https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use
    static LinkedList<Command *> &_listener_list()
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

  static inline Command *NK(KeyCode key_code) { return (new NormalKey(key_code)); }

  //------------------------------------------------------------------+
  // ModifierKey
  //------------------------------------------------------------------+
  class ModifierKey : public Command
  {
  public:
    ModifierKey(Modifier modifier);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const Modifier _modifier;
  };

  static inline Command *MO(Modifier modifier) { return (new ModifierKey(modifier)); }

  //------------------------------------------------------------------+
  // CombinationKey
  //------------------------------------------------------------------+
  class CombinationKey : public Command
  {
  public:
    CombinationKey(Modifier modifier, KeyCode key_code);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const Modifier _modifier;
    const KeyCode _key_code;
  };

  static inline Command *CK(Modifier modifier, KeyCode key_code) { return (new CombinationKey(modifier, key_code)); }

  //------------------------------------------------------------------+
  // ModifierTap
  //------------------------------------------------------------------+
  class ModifierTap : public Command
  {
  public:
    ModifierTap(Modifier modifier, Command *command);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const Modifier _modifier;
    Command *_command;
  };

  static inline Command *MT(Modifier modifier, Command *command) { return (new ModifierTap(modifier, command)); }

  //------------------------------------------------------------------+
  // OneShotModifier
  //------------------------------------------------------------------+
  class OneShotModifier : public Command
  {
  public:
    OneShotModifier(Modifier modifier);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const Modifier _modifier;
  };

  static inline Command *OSM(Modifier modifier) { return (new OneShotModifier(modifier)); }

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

  using CommandPtr = Command *;

  template <size_t N>
  static Command *LY1(const CommandPtr (&arr)[N])
  {
    static_assert(N <= HID_ENGINE_LAYER_SIZE, "");

    Command **arg = new Command *[HID_ENGINE_LAYER_SIZE] {};
    for (size_t i = 0; i < N; i++)
    {
      arg[i] = arr[i];
    }
    return (new Layering(&Layer1, arg));
  }

  template <size_t N>
  static Command *LY2(const CommandPtr (&arr)[N])
  {
    static_assert(N <= HID_ENGINE_LAYER_SIZE, "");

    Command **arg = new Command *[HID_ENGINE_LAYER_SIZE] {};
    for (size_t i = 0; i < N; i++)
    {
      arg[i] = arr[i];
    }
    return (new Layering(&Layer2, arg));
  }

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

  static inline Command *LT1(uint8_t layer_number, Command *command) { return (new LayerTap(&Layer1, layer_number, command)); }
  static inline Command *LT2(uint8_t layer_number, Command *command) { return (new LayerTap(&Layer2, layer_number, command)); }

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

  static inline Command *TL1(uint8_t layer_number) { return (new ToggleLayer(&Layer1, layer_number)); }
  static inline Command *TL2(uint8_t layer_number) { return (new ToggleLayer(&Layer2, layer_number)); }

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

  static inline Command *SL1(uint8_t layer_number) { return (new SwitchLayer(&Layer1, layer_number)); }
  static inline Command *SL2(uint8_t layer_number) { return (new SwitchLayer(&Layer2, layer_number)); }

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
    bool _chained_osl[HID_ENGINE_LAYER_SIZE];
  };

  static inline Command *OSL1(uint8_t layer_number) { return (new OneShotLayer(&Layer1, layer_number)); }
  static inline Command *OSL2(uint8_t layer_number) { return (new OneShotLayer(&Layer2, layer_number)); }

  // ------------------------------------------------------------------+
  // Tap
  // ------------------------------------------------------------------+
  class Tap : public Command
  {
  public:
    Tap(Command *command, uint8_t n_times);

  protected:
    void onPress(uint8_t n_times) override;

  private:
    Command *_command;
    const uint8_t _n_times;
  };

  static inline Command *TAP(Command *command, uint8_t n_times) { return (new Tap(command, n_times)); }

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

    TapDance(Pair pairs[], size_t len);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;
    void onTimer() override;
    void onBeforeInput() override;

  private:
    enum class State
    {
      Unexecuted,
      Unfixed,
      Tap_or_NextCommand,
      FixedToHold,
    };

    Pair *const _pairs;
    Command *_running_command;
    const size_t _len;
    size_t _count;
    State _state;
  };

  template <size_t N>
  static Command *TD(const TapDance::Pair (&arr)[N])
  {
    TapDance::Pair *arg = new TapDance::Pair[N];
    for (size_t i = 0; i < N; i++)
    {
      arg[i].tap_command = arr[i].tap_command;
      arg[i].hold_command = arr[i].hold_command;
    }
    return (new TapDance(arg, N));
  }

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
    enum class State
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

  static inline Command *ToH(Command *tap_command, unsigned int ms, Command *hold_command) { return (new TapOrHold(tap_command, ms, hold_command)); }

  //------------------------------------------------------------------+
  // ConsumerControll
  //------------------------------------------------------------------+
  class ConsumerControll : public Command
  {
  public:
    ConsumerControll(UsageCode usage_code);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const UsageCode _usage_code;
  };

  static inline Command *CC(UsageCode usage_code) { return (new ConsumerControll(usage_code)); }

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

  static inline Command *MS_MOV(int16_t x, int16_t y) { return (new MouseMove(x, y)); }

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

  static inline Command *MS_SPD(int percent) { return (new MouseSpeed(percent)); }

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
    uint8_t _n_times;
  };

  static inline Command *MS_SCR(int8_t scroll, int8_t horiz) { return (new MouseScroll(scroll, horiz)); }

  //------------------------------------------------------------------+
  // MouseClick
  //------------------------------------------------------------------+
  class MouseClick : public Command
  {
  public:
    MouseClick(MouseButton button);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const MouseButton _button;
  };

  static inline Command *MS_CLK(MouseButton button) { return (new MouseClick(button)); }

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

  static inline Command *IF(bool (*func)(), Command *true_command, Command *false_command) { return (new If(func, true_command, false_command)); }

  //------------------------------------------------------------------+
  // Multi
  //------------------------------------------------------------------+
  class Multi : public Command
  {
  public:
    Multi(Command *commands[], size_t len);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    Command **const _commands;
    const size_t _len;
  };

  template <size_t N>
  static Command *MLT(const CommandPtr (&arr)[N])
  {
    Command **arg = new Command *[N] {};

    for (size_t i = 0; i < N; i++)
    {
      arg[i] = arr[i];
    }
    return (new Multi(arg, N));
  }

  //------------------------------------------------------------------+
  // Other Command
  //------------------------------------------------------------------+

  // No Operation
  static inline Command *NOP() { return (new Command); }

  // Transparent (_ * 7)
  #define _______ (static_cast<Command *>(nullptr))

} // namespace hidpg
