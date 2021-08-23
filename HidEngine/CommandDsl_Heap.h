/*
  The MIT License (MIT)

  Copyright (c) 2021 ogatatsu.

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

#include "Command.h"
#include "HidEngine.h"

namespace hidpg
{
  // CommandDsl Dynamic Memory Allocation virsion.

  using CommandPtr = Command *;

  static inline Command *NK(KeyCode key_code) { return (new NormalKey(key_code)); }

  static inline Command *MO(Modifiers modifiers) { return (new ModifierKey(modifiers)); }

  static inline Command *CK(Modifiers modifiers, KeyCode key_code) { return (new CombinationKey(modifiers, key_code)); }

  static inline Command *MT(Modifiers modifiers, Command *command) { return (new ModifierTap(modifiers, command)); }

  static inline Command *OSM(Modifiers modifiers) { return (new OneShotModifier(modifiers)); }

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

  static inline Command *LT(uint8_t layer_number, Command *command) { return (new LayerTap(&Layer, layer_number, command)); }
  static inline Command *LT1(uint8_t layer_number, Command *command) { return (new LayerTap(&Layer1, layer_number, command)); }
  static inline Command *LT2(uint8_t layer_number, Command *command) { return (new LayerTap(&Layer2, layer_number, command)); }

  static inline Command *TL(uint8_t layer_number) { return (new ToggleLayer(&Layer, layer_number)); }
  static inline Command *TL1(uint8_t layer_number) { return (new ToggleLayer(&Layer1, layer_number)); }
  static inline Command *TL2(uint8_t layer_number) { return (new ToggleLayer(&Layer2, layer_number)); }

  static inline Command *SL(uint8_t layer_number) { return (new SwitchLayer(&Layer, layer_number)); }
  static inline Command *SL1(uint8_t layer_number) { return (new SwitchLayer(&Layer1, layer_number)); }
  static inline Command *SL2(uint8_t layer_number) { return (new SwitchLayer(&Layer2, layer_number)); }

  static inline Command *OSL(uint8_t layer_number) { return (new OneShotLayer(&Layer, layer_number)); }
  static inline Command *OSL1(uint8_t layer_number) { return (new OneShotLayer(&Layer1, layer_number)); }
  static inline Command *OSL2(uint8_t layer_number) { return (new OneShotLayer(&Layer2, layer_number)); }

  static inline Command *TAP(Command *command, uint8_t n_times, uint16_t tap_speed_ms = HID_ENGINE_TAP_SPEED_MS) { return (new Tap(command, n_times, tap_speed_ms)); }

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

  static inline Command *ToH(Command *tap_command, unsigned int ms, Command *hold_command) { return (new TapOrHold(tap_command, ms, hold_command)); }

  static inline Command *CC(ConsumerControlCode usage_code) { return (new ConsumerControl(usage_code)); }

  static inline Command *SC(SystemControlCode usage_code) { return (new SystemControl(usage_code)); }

  static inline Command *MS_MOV(int16_t x, int16_t y) { return (new MouseMove(x, y)); }

  static inline Command *MS_SPD(int percent) { return (new MouseSpeed(percent)); }

  static inline Command *MS_SCR(int8_t scroll, int8_t horiz) { return (new MouseScroll(scroll, horiz)); }

  static inline Command *MS_CLK(MouseButtons buttons) { return (new MouseClick(buttons)); }

  static inline Command *RD_CLK() { return (new RadialClick()); }

  static inline Command *RD_ROT(int16_t deci_degree)
  {
    deci_degree = constrain(deci_degree, -3600, 3600);
    return (new RadialRotate(deci_degree));
  }

  static inline Command *IF(bool (*func)(), Command *true_command, Command *false_command) { return (new If(func, true_command, false_command)); }

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

  static inline Command *NOP() { return (new NoOperation()); }

  static inline Command *SEQ_MODE() { return (new Internal::HidEngineClass::SequenceMode); }

  static inline Command *TRC(uint8_t track_id) { return (new Internal::HidEngineClass::Tracking(track_id)); }

  static inline Command *TRT(uint8_t track_id, Command *command) { return (new Internal::HidEngineClass::TrackTap(track_id, command)); }

  // nullptr alias (_ * 7)
  #define _______ (static_cast<Command *>(nullptr))

} // namespace hidpg
