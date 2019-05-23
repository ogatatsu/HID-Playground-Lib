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
#include "LinkedList.h"
#include "TimerMixin.h"
#include "keycode.h"
#include <stddef.h>

namespace hidpg
{

/*------------------------------------------------------------------*/
class Command
{
public:
  Command();
  uint8_t press(uint8_t accrued = 1);
  void release();
  void setParent(Command *parent) { _parent = parent; }
  Command *getParent() { return _parent; }

protected:
  bool isLastPressed();
  virtual uint8_t onPress(uint8_t accrued) { return 1; }
  virtual void onRelease() {}
  virtual void onDifferentRootCommandPress() {}
  void addEventListener_DifferentRootCommandPress();

private:
  // keymap(グローバル変数)の定義で特定のコマンドがnewされたときにaddEventListener_DifferentRootCommandPress()が呼ばれる
  // _listenerListはその内部で使用するので単純なstatic変数にすると初期化順序が問題になる。
  // https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use
  static LinkedList<Command *> &_listenerList()
  {
    static LinkedList<Command *> *listenerList = new LinkedList<Command *>;
    return *listenerList;
  };
  static Command *_lastPressedCommand;

  Command *_parent;
  bool _prevState;
};

/*------------------------------------------------------------------*/
class NormalKey : public Command
{
public:
  NormalKey(Keycode keycode);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  const Keycode _keycode;
};

/*------------------------------------------------------------------*/
class ModifierKey : public Command
{
public:
  ModifierKey(Modifier modifier);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  const Modifier _modifier;
};

/*------------------------------------------------------------------*/
class CombinationKey : public Command
{
public:
  CombinationKey(Modifier modifier, Keycode keycode);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  const Modifier _modifier;
  const Keycode _keycode;
};

/*------------------------------------------------------------------*/
class ModifierTap : public Command
{
public:
  ModifierTap(Modifier modifier, Command *command);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  const Modifier _modifier;
  Command *_command;
};

/*------------------------------------------------------------------*/
class OneShotModifier : public Command
{
public:
  OneShotModifier(Modifier modifier);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  const Modifier _modifier;
};

/*------------------------------------------------------------------*/
class Layering : public Command
{
public:
  Layering(Command *commands[LAYER_SIZE]);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  Command **const _commands;
  Command *_runningCommand;
};

/*------------------------------------------------------------------*/
class LayerTap : public Command
{
public:
  LayerTap(uint8_t layerNumber, Command *command);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  const uint8_t _layerNumber;
  Command *_command;
};

/*------------------------------------------------------------------*/
class ToggleLayer : public Command
{
public:
  ToggleLayer(uint8_t layerNumber);

protected:
  uint8_t onPress(uint8_t accrued) override;

private:
  const uint8_t _layerNumber;
};

/*------------------------------------------------------------------*/
class SwitchLayer : public Command
{
public:
  SwitchLayer(uint8_t layerNumber);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  const uint8_t _layerNumber;
};

/*------------------------------------------------------------------*/
class OneShotLayer : public Command
{
public:
  OneShotLayer(uint8_t layerNumber);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  const uint8_t _layerNumber;
  bool _chainedOSL[LAYER_SIZE];
};

/*------------------------------------------------------------------*/
class TapDance : public Command, public TimerMixin
{
public:
  struct Pair
  {
    Command *tapCommand;
    Command *holdCommand;
  };

  TapDance(Pair pairs[], size_t len);

protected:
  uint8_t onPress(uint8_t accrued) override;
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
  Command *_runningCommand;
  const size_t _len;
  size_t _count;
  State _state;
};

/*------------------------------------------------------------------*/
class TapOrHold : public Command, public TimerMixin
{
public:
  TapOrHold(Command *tapCommand, unsigned int ms, Command *holdCommand);

protected:
  uint8_t onPress(uint8_t accrued) override;
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
  Command *const _tapCommand;
  Command *const _holdCommand;
};

/*------------------------------------------------------------------*/
class ConsumerControll : public Command
{
public:
  ConsumerControll(UsageCode usageCode);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  const UsageCode _usageCode;
};

/*------------------------------------------------------------------*/
class MouseMove : public Command
{
public:
  MouseMove(int8_t x, int8_t y);

protected:
  uint8_t onPress(uint8_t accrued) override;
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

/*------------------------------------------------------------------*/
class MouseSpeed : public Command
{
public:
  MouseSpeed(int16_t percent);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  const int16_t _percent;
};

/*------------------------------------------------------------------*/
class MouseScroll : public Command
{
public:
  MouseScroll(int8_t scroll, int8_t horiz);

protected:
  uint8_t onPress(uint8_t accrued) override;

private:
  const int8_t _scroll;
  const int8_t _horiz;
};

/*------------------------------------------------------------------*/
class MouseClick : public Command
{
public:
  MouseClick(MouseButton button);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  const MouseButton _button;
};

/*------------------------------------------------------------------*/
class Macro : public Command, public TimerMixin
{
public:
  class MacroCommand
  {
  public:
    virtual unsigned int apply() = 0;
  };

  class DownKey : public MacroCommand
  {
  public:
    DownKey(Keycode keycode);
    unsigned int apply() override;

  private:
    const Keycode _keycode;
  };

  class UpKey : public MacroCommand
  {
  public:
    UpKey(Keycode keycode);
    unsigned int apply() override;

  private:
    const Keycode _keycode;
  };

  class DownModifier : public MacroCommand
  {
  public:
    DownModifier(Modifier modifier);
    unsigned int apply() override;

  private:
    const Modifier _modifier;
  };

  class UpModifier : public MacroCommand
  {
  public:
    UpModifier(Modifier modifier);
    unsigned int apply() override;

  private:
    const Modifier _modifier;
  };

  class Wait : public MacroCommand
  {
  public:
    Wait(unsigned int delay);
    unsigned int apply() override;

  private:
    const unsigned int _delay;
  };

  Macro(MacroCommand **mcommands, size_t len);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onTimer() override;

private:
  MacroCommand **const _mcommands;
  const size_t _len;
  unsigned int _count;
  bool _isRunning;
};

/*------------------------------------------------------------------*/
class If : public Command
{
public:
  If(bool (*func)(), Command *trueCommand, Command *falseCommand);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  bool (*const _func)();
  Command *const _trueCommand;
  Command *const _falseCommand;
  Command *_runningCommand;
};

/*------------------------------------------------------------------*/
class Double : public Command
{
public:
  Double(Command *command1, Command *command2);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  Command *const _command1;
  Command *const _command2;
};

/*------------------------------------------------------------------*/
/*  define short name command
 *------------------------------------------------------------------*/
// No Operation
#define NOP (new Command)

// Normal Key
static inline Command *NK(Keycode keycode) { return (new NormalKey(keycode)); }

// Modifier Key
static inline Command *MO(Modifier modifier) { return (new ModifierKey(modifier)); }

// Combination Key
static inline Command *CK(Modifier modifier, Keycode keycode) { return (new CombinationKey(modifier, keycode)); }

// Modifier or Tap
static inline Command *MT(Modifier modifier, Command *command) { return (new ModifierTap(modifier, command)); }

// Oneshot Modifier
static inline Command *OSM(Modifier modifier) { return (new OneShotModifier(modifier)); }

// Layering
using CommandPtr = Command *;

template <size_t N>
static Command *L(const CommandPtr (&arr)[N])
{
  static_assert(N <= LAYER_SIZE, "");

  Command **arg = new Command *[LAYER_SIZE] {};
  for (size_t i = 0; i < N; i++)
  {
    arg[i] = arr[i];
  }
  return (new Layering(arg));
}

// Transparent (_ * 7)
#define _______ (static_cast<Command *>(nullptr))

// Layer or Tap
static inline Command *LT(uint8_t layerNumber, Command *command) { return (new LayerTap(layerNumber, command)); }

// Toggle Layer (alternate)
static inline Command *TL(uint8_t layerNumber) { return (new ToggleLayer(layerNumber)); }

// Switch Layer (momentary)
static inline Command *SL(uint8_t layerNumber) { return (new SwitchLayer(layerNumber)); }

// Oneshot Layer
static inline Command *OSL(uint8_t layerNumber) { return (new OneShotLayer(layerNumber)); }

// Tap Dance
template <size_t N>
static Command *TD(const TapDance::Pair (&arr)[N])
{
  TapDance::Pair *arg = new TapDance::Pair[N];
  for (size_t i = 0; i < N; i++)
  {
    arg[i].tapCommand = arr[i].tapCommand;
    arg[i].holdCommand = arr[i].holdCommand;
  }
  return (new TapDance(arg, N));
}

// Tap or Hold
static inline Command *ToH(Command *tapCommand, unsigned int ms, Command *holdCommand) { return (new TapOrHold(tapCommand, ms, holdCommand)); }

// Consumer Controll
static inline Command *CC(UsageCode usageCode) { return (new ConsumerControll(usageCode)); }

// Mouse Move
static inline Command *MS_MOV(int8_t x, int8_t y) { return (new MouseMove(x, y)); }

// Mouse Speed
static inline Command *MS_SPD(int percent) { return (new MouseSpeed(percent)); }

// Mouse Scroll
static inline Command *MS_SCR(int8_t scroll, int8_t horiz) { return (new MouseScroll(scroll, horiz)); }

// Mouse Click
static inline Command *MS_CLK(MouseButton button) { return (new MouseClick(button)); }

// Macro
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

static inline Macro::MacroCommand *D(Keycode keycode) { return (new Macro::DownKey(keycode)); }
static inline Macro::MacroCommand *D(Modifier modifier) { return (new Macro::DownModifier(modifier)); }
static inline Macro::MacroCommand *U(Keycode keycode) { return (new Macro::UpKey(keycode)); }
static inline Macro::MacroCommand *U(Modifier modifier) { return (new Macro::UpModifier(modifier)); }
static inline Macro::MacroCommand *W(unsigned int delay) { return (new Macro::Wait(delay)); }

// If
static inline Command *IF(bool (*func)(), Command *trueCommand, Command *falseCommand) { return (new If(func, trueCommand, falseCommand)); }

// Double
static inline Command *DBL(Command *command1, Command *command2) { return (new Double(command1, command2)); }

} // namespace hidpg
