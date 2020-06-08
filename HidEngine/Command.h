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
    uint8_t press(uint8_t accumulation = 1);
    void release();
    void setParent(Command *parent) { _parent = parent; }
    Command *getParent() { return _parent; }

  protected:
    bool isLastPressed();
    virtual uint8_t onPress(uint8_t accumulation) { return 1; }
    virtual void onRelease() {}
    virtual void onDifferentRootCommandPress() {}
    void addEventListener_DifferentRootCommandPress();

  private:
    // keymap(グローバル変数)の定義で特定のコマンドがnewされたときにaddEventListener_DifferentRootCommandPress()が呼ばれる
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
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;

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
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;

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
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;

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
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;

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
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;

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
    Layering(Command *commands[LAYER_SIZE]);

  protected:
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;

  private:
    Command **const _commands;
    Command *_running_command;
  };

  using CommandPtr = Command *;

  template <size_t N>
  static Command *LY(const CommandPtr (&arr)[N])
  {
    static_assert(N <= LAYER_SIZE, "");

    Command **arg = new Command *[LAYER_SIZE] {};
    for (size_t i = 0; i < N; i++)
    {
      arg[i] = arr[i];
    }
    return (new Layering(arg));
  }

  //------------------------------------------------------------------+
  // LayerTap
  //------------------------------------------------------------------+
  class LayerTap : public Command
  {
  public:
    LayerTap(uint8_t layer_number, Command *command);

  protected:
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;

  private:
    const uint8_t _layer_number;
    Command *_command;
  };

  static inline Command *LT(uint8_t layer_number, Command *command) { return (new LayerTap(layer_number, command)); }

  //------------------------------------------------------------------+
  // ToggleLayer
  //------------------------------------------------------------------+
  class ToggleLayer : public Command
  {
  public:
    ToggleLayer(uint8_t layer_number);

  protected:
    uint8_t onPress(uint8_t accumulation) override;

  private:
    const uint8_t _layer_number;
  };

  static inline Command *TL(uint8_t layer_number) { return (new ToggleLayer(layer_number)); }

  //------------------------------------------------------------------+
  // SwitchLayer
  //------------------------------------------------------------------+
  class SwitchLayer : public Command
  {
  public:
    SwitchLayer(uint8_t layer_number);

  protected:
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;

  private:
    const uint8_t _layer_number;
  };

  static inline Command *SL(uint8_t layer_number) { return (new SwitchLayer(layer_number)); }

  //------------------------------------------------------------------+
  // OneShotLayer
  //------------------------------------------------------------------+
  class OneShotLayer : public Command
  {
  public:
    OneShotLayer(uint8_t layer_number);

  protected:
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;

  private:
    const uint8_t _layer_number;
    bool _chained_osl[LAYER_SIZE];
  };

  static inline Command *OSL(uint8_t layer_number) { return (new OneShotLayer(layer_number)); }

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
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;
    void onTimer() override;
    void onDifferentRootCommandPress() override;

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
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;
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
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;

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
    MouseMove(int8_t x, int8_t y);

  protected:
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;

  private:
    class Mover : public TimerMixin
    {
    public:
      Mover();
      void setXY(int8_t x, int8_t y);
      void unsetXY(int8_t x, int8_t y);

    protected:
      void onTimer() override;

    private:
      void calcXY(int8_t &x, int8_t &y);

      int _x;
      int _y;
      uint8_t _count;
    };

    static Mover _mover;

    const int8_t _x;
    const int8_t _y;
  };

  static inline Command *MS_MOV(int8_t x, int8_t y) { return (new MouseMove(x, y)); }

  //------------------------------------------------------------------+
  // MouseSpeed
  //------------------------------------------------------------------+
  class MouseSpeed : public Command
  {
  public:
    MouseSpeed(int16_t percent);

  protected:
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;

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
    uint8_t onPress(uint8_t accumulation) override;

  private:
    const int8_t _scroll;
    const int8_t _horiz;
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
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;

  private:
    const MouseButton _button;
  };

  static inline Command *MS_CLK(MouseButton button) { return (new MouseClick(button)); }

  //------------------------------------------------------------------+
  // Macro
  //------------------------------------------------------------------+
  class Macro : public Command, public TimerMixin
  {
  public:
    class MacroCommand
    {
    public:
      virtual unsigned int run() = 0;
    };

    Macro(MacroCommand **m_commands, size_t len);

  protected:
    uint8_t onPress(uint8_t accumulation) override;
    void onTimer() override;

  private:
    MacroCommand **const _m_commands;
    const size_t _len;
    unsigned int _count;
    bool _is_running;
  };

  class MC_DownKey : public Macro::MacroCommand
  {
  public:
    MC_DownKey(KeyCode key_code);
    unsigned int run() override;

  private:
    const KeyCode _key_code;
  };

  class MC_UpKey : public Macro::MacroCommand
  {
  public:
    MC_UpKey(KeyCode key_code);
    unsigned int run() override;

  private:
    const KeyCode _key_code;
  };

  class MC_DownModifier : public Macro::MacroCommand
  {
  public:
    MC_DownModifier(Modifier modifier);
    unsigned int run() override;

  private:
    const Modifier _modifier;
  };

  class MC_UpModifier : public Macro::MacroCommand
  {
  public:
    MC_UpModifier(Modifier modifier);
    unsigned int run() override;

  private:
    const Modifier _modifier;
  };

  class MC_TapKey : public Macro::MacroCommand
  {
  public:
    MC_TapKey(KeyCode key_code);
    unsigned int run() override;

  private:
    const KeyCode _key_code;
  };

  class MC_Wait : public Macro::MacroCommand
  {
  public:
    MC_Wait(unsigned int delay);
    unsigned int run() override;

  private:
    const unsigned int _delay;
  };

  using MacroCommandPtr = Macro::MacroCommand *;

  template <size_t N>
  static Command *MACRO(const MacroCommandPtr (&arr)[N])
  {
    Macro::MacroCommand **arg = new Macro::MacroCommand *[N];
    for (size_t i = 0; i < N; i++)
    {
      arg[i] = arr[i];
    }
    return (new Macro(arg, N));
  }

  static inline Macro::MacroCommand *DN(KeyCode key_code) { return (new MC_DownKey(key_code)); }
  static inline Macro::MacroCommand *DN(Modifier modifier) { return (new MC_DownModifier(modifier)); }
  static inline Macro::MacroCommand *UP(KeyCode key_code) { return (new MC_UpKey(key_code)); }
  static inline Macro::MacroCommand *UP(Modifier modifier) { return (new MC_UpModifier(modifier)); }
  static inline Macro::MacroCommand *TP(KeyCode key_code) { return (new MC_TapKey(key_code)); }
  static inline Macro::MacroCommand *WT(unsigned int delay) { return (new MC_Wait(delay)); }

  //------------------------------------------------------------------+
  // If
  //------------------------------------------------------------------+
  class If : public Command
  {
  public:
    If(bool (*func)(), Command *true_command, Command *false_command);

  protected:
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;

  private:
    bool (*const _func)();
    Command *const _true_command;
    Command *const _false_command;
    Command *_running_command;
  };

  static inline Command *IF(bool (*func)(), Command *true_command, Command *false_command) { return (new If(func, true_command, false_command)); }

  //------------------------------------------------------------------+
  // Double
  //------------------------------------------------------------------+
  class Double : public Command
  {
  public:
    Double(Command *command1, Command *command2);

  protected:
    uint8_t onPress(uint8_t accumulation) override;
    void onRelease() override;

  private:
    Command *const _command1;
    Command *const _command2;
  };

  static inline Command *DBL(Command *command1, Command *command2) { return (new Double(command1, command2)); }

  //------------------------------------------------------------------+
  // Other Command
  //------------------------------------------------------------------+

// No Operation
#define NOP (new Command)

// Transparent (_ * 7)
#define _______ (static_cast<Command *>(nullptr))

} // namespace hidpg
