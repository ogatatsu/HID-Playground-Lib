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
  static Command *LY(const CommandPtr (&arr)[N])
  {
    static_assert(N <= HID_ENGINE_LAYER_SIZE, "");

    Command **arg = new Command *[HID_ENGINE_LAYER_SIZE] {};
    for (size_t i = 0; i < N; i++)
    {
      arg[i] = arr[i];
    }
    return (new Layering(&Layer, arg));
  }

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

  static inline Command *LT(uint8_t layer_number, Command *command) { return (new LayerTap(&Layer, layer_number, command)); }
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

  static inline Command *TL(uint8_t layer_number) { return (new ToggleLayer(&Layer, layer_number)); }
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

  static inline Command *SL(uint8_t layer_number) { return (new SwitchLayer(&Layer, layer_number)); }
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
    layer_bitmap_t _chained_osl;
  };

  static inline Command *OSL(uint8_t layer_number) { return (new OneShotLayer(&Layer, layer_number)); }
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

  template <int8_t N>
  static Command *TD(const TapDance::Pair (&arr)[N], bool confirm_command_with_mouse_move = false)
  {
    TapDance::Pair *arg = new TapDance::Pair[N];
    for (int i = 0; i < N; i++)
    {
      arg[i].tap_command = arr[i].tap_command;
      arg[i].hold_command = arr[i].hold_command;
    }
    return (new TapDance(arg, N, confirm_command_with_mouse_move));
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

  static inline Command *ToH(Command *tap_command, unsigned int ms, Command *hold_command) { return (new TapOrHold(tap_command, ms, hold_command)); }

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

  static inline Command *CC(ConsumerControlCode usage_code) { return (new ConsumerControl(usage_code)); }

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

  static inline Command *SC(SystemControlCode usage_code) { return (new SystemControl(usage_code)); }

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
    const uint8_t _max_n_times;
    uint8_t _actual_n_times;
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
  // RadialClick
  //------------------------------------------------------------------+
  class RadialClick : public Command
  {
  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;
  };

  static inline Command *RD_CLK() { return (new RadialClick()); }

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

  static inline Command *RD_ROT(int16_t deci_degree)
  {
    deci_degree = constrain(deci_degree, -3600, 3600);
    return (new RadialRotate(deci_degree));
  }

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
    Multi(Command *commands[], uint8_t len);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    Command **const _commands;
    const uint8_t _len;
  };

  template <uint8_t N>
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

  static inline Command *NOP() { return (new NoOperation()); }

  //------------------------------------------------------------------+
  // Other Command
  //------------------------------------------------------------------+

  // Transparent (_ * 7)
  #define _______ (static_cast<Command *>(nullptr))

} // namespace hidpg
