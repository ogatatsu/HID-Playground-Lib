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
#include "hash_code.h"
#include <new>

namespace hidpg
{

  using CommandPtr = Command *;

  namespace Internal
  {
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_NormalKey(KeyCode key_code)
    {
      static uint8_t buf[sizeof(NormalKey)];
      return new (buf) NormalKey(key_code);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_ModifierKey(Modifiers modifiers)
    {
      static uint8_t buf[sizeof(ModifierKey)];
      return new (buf) ModifierKey(modifiers);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_CombinationKey(Modifiers modifiers, KeyCode key_code)
    {
      static uint8_t buf[sizeof(CombinationKey)];
      return new (buf) CombinationKey(modifiers, key_code);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_ModifierTap(Modifiers modifiers, Command *command)
    {
      static uint8_t buf[sizeof(ModifierTap)];
      return new (buf) ModifierTap(modifiers, command);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_OneShotModifier(Modifiers modifiers)
    {
      static uint8_t buf[sizeof(OneShotModifier)];
      return new (buf) OneShotModifier(modifiers);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    Command *new_Layering(LayerClass *layer, const CommandPtr (&arr)[N])
    {
      static_assert(N <= HID_ENGINE_LAYER_SIZE, "");

      static Command *arg[HID_ENGINE_LAYER_SIZE] = {};
      static uint8_t buf[sizeof(Layering)];

      for (size_t i = 0; i < N; i++)
      {
        arg[i] = arr[i];
      }
      return new (buf) Layering(layer, arg);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_LayerTap(LayerClass *layer, uint8_t layer_number, Command *command)
    {
      static uint8_t buf[sizeof(LayerTap)];
      return new (buf) LayerTap(layer, layer_number, command);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_ToggleLayer(LayerClass *layer, uint8_t layer_number)
    {
      static uint8_t buf[sizeof(ToggleLayer)];
      return new (buf) ToggleLayer(layer, layer_number);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_SwitchLayer(LayerClass *layer, uint8_t layer_number)
    {
      static uint8_t buf[sizeof(SwitchLayer)];
      return new (buf) SwitchLayer(layer, layer_number);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_OneShotLayer(LayerClass *layer, uint8_t layer_number)
    {
      static uint8_t buf[sizeof(OneShotLayer)];
      return new (buf) OneShotLayer(layer, layer_number);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_Tap(Command *command, uint8_t n_times, uint16_t tap_speed_ms = HID_ENGINE_TAP_SPEED_MS)
    {
      static uint8_t buf[sizeof(Tap)];
      return new (buf) Tap(command, n_times, tap_speed_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    Command *new_TapDance(const TapDance::Pair (&arr)[N], bool confirm_command_with_mouse_move = false)
    {
      static TapDance::Pair arg[N];
      static uint8_t buf[sizeof(TapDance)];

      for (size_t i = 0; i < N; i++)
      {
        arg[i].tap_command = arr[i].tap_command;
        arg[i].hold_command = arr[i].hold_command;
      }
      return new (buf) TapDance(arg, N, confirm_command_with_mouse_move);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_TapOrHold(Command *tap_command, unsigned int ms, Command *hold_command)
    {
      static uint8_t buf[sizeof(TapOrHold)];
      return new (buf) TapOrHold(tap_command, ms, hold_command);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_ConsumerControl(ConsumerControlCode usage_code)
    {
      static uint8_t buf[sizeof(ConsumerControl)];
      return new (buf) ConsumerControl(usage_code);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_SystemControl(SystemControlCode usage_code)
    {
      static uint8_t buf[sizeof(SystemControl)];
      return new (buf) SystemControl(usage_code);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_MouseMove(int16_t x, int16_t y)
    {
      static uint8_t buf[sizeof(MouseMove)];
      return new (buf) MouseMove(x, y);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_MouseSpeed(int16_t percent)
    {
      static uint8_t buf[sizeof(MouseSpeed)];
      return new (buf) MouseSpeed(percent);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_MouseScroll(int8_t scroll, int8_t horiz)
    {
      static uint8_t buf[sizeof(MouseScroll)];
      return new (buf) MouseScroll(scroll, horiz);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_MouseClick(MouseButtons buttons)
    {
      static uint8_t buf[sizeof(MouseClick)];
      return new (buf) MouseClick(buttons);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_RadialClick()
    {
      static uint8_t buf[sizeof(RadialClick)];
      return new (buf) RadialClick();
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_RadialRotate(int16_t deci_degree)
    {
      deci_degree = constrain(deci_degree, -3600, 3600);

      static uint8_t buf[sizeof(RadialRotate)];
      return new (buf) RadialRotate(deci_degree);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_If(bool (*func)(), Command *true_command, Command *false_command)
    {
      static uint8_t buf[sizeof(If)];
      return new (buf) If(func, true_command, false_command);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    Command *new_Multi(const CommandPtr (&arr)[N])
    {
      static Command *arg[N];
      static uint8_t buf[sizeof(Multi)];

      for (size_t i = 0; i < N; i++)
      {
        arg[i] = arr[i];
      }
      return new (buf) Multi(arg, N);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_NoOperation()
    {
      static uint8_t buf[sizeof(NoOperation)];
      return new (buf) NoOperation();
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_SequenceMode()
    {
      static uint8_t buf[sizeof(HidEngineClass::SequenceMode)];
      return new (buf) HidEngineClass::SequenceMode();
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_Tracking(uint8_t track_id)
    {
      static uint8_t buf[sizeof(HidEngineClass::Tracking)];
      return new (buf) HidEngineClass::Tracking(track_id);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_TrackTap(uint8_t track_id, Command *command)
    {
      static uint8_t buf[sizeof(HidEngineClass::TrackTap)];
      return new (buf) HidEngineClass::TrackTap(track_id, command);
    }

  } // namespace Internal

// NormalKey
#define NK(key_code) (Internal::new_NormalKey<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(key_code))

// ModifierKey
#define MO(modifiers) (Internal::new_ModifierKey<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(modifiers))

// CombinationKey
#define CK(modifiers, key_code) (Internal::new_CombinationKey<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(modifiers, key_code))

// ModifierTap
#define MT(modifiers, command) (Internal::new_ModifierTap<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(modifiers, command))

// OneShotModifier
#define OSM(modifiers) (Internal::new_OneShotModifier<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(modifiers))

// Layering
#define LY(...) (Internal::new_Layering<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(&Layer, __VA_ARGS__))
#define LY1(...) (Internal::new_Layering<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(&Layer1, __VA_ARGS__))
#define LY2(...) (Internal::new_Layering<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(&Layer2, __VA_ARGS__))

// LayerTap
#define LT(layer_number, command) (Internal::new_LayerTap<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(&Layer, layer_number, command))
#define LT1(layer_number, command) (Internal::new_LayerTap<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(&Layer1, layer_number, command))
#define LT2(layer_number, command) (Internal::new_LayerTap<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(&Layer2, layer_number, command))

// ToggleLayer
#define TL(layer_number) (Internal::new_ToggleLayer<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(&Layer, layer_number))
#define TL1(layer_number) (Internal::new_ToggleLayer<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(&Layer1, layer_number))
#define TL2(layer_number) (Internal::new_ToggleLayer<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(&Layer2, layer_number))

// SwitchLayer
#define SL(layer_number) (Internal::new_SwitchLayer<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(&Layer, layer_number))
#define SL1(layer_number) (Internal::new_SwitchLayer<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(&Layer1, layer_number))
#define SL2(layer_number) (Internal::new_SwitchLayer<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(&Layer2, layer_number))

// OneShotLayer
#define OSL(layer_number) (Internal::new_OneShotLayer<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(&Layer, layer_number))
#define OSL1(layer_number) (Internal::new_OneShotLayer<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(&Layer1, layer_number))
#define OSL2(layer_number) (Internal::new_OneShotLayer<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(&Layer2, layer_number))

// Tap
#define TAP(...) (Internal::new_Tap<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(__VA_ARGS__))

// TapDance
#define TD(...) (Internal::new_TapDance<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(__VA_ARGS__))

// TapOrHold
#define ToH(tap_command, ms, hold_command) (Internal::new_TapOrHold<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(tap_command, ms, hold_command))

// ConsumerControl
#define CC(usage_code) (Internal::new_ConsumerControl<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(usage_code))

// SystemControl
#define SC(usage_code) (Internal::new_SystemControl<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(usage_code))

// MouseMove
#define MS_MOV(x, y) (Internal::new_MouseMove<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(x, y))

// MouseSpeed
#define MS_SPD(percent) (Internal::new_MouseSpeed<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(percent))

// MouseScroll
#define MS_SCR(scroll, horiz) (Internal::new_MouseScroll<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(scroll, horiz))

// MouseClick
#define MS_CLK(buttons) (Internal::new_MouseClick<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(buttons))

// RadialClick
#define RD_CLK() (Internal::new_RadialClick<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>())

// RadialRotate
#define RD_ROT(deci_degree) (Internal::new_RadialRotate<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(deci_degree))

// If
#define IF(func, true_command, false_command) (Internal::new_If<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(func, true_command, false_command))

// Multi
#define MLT(...) (Internal::new_Multi<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(__VA_ARGS__))

// NoOperation
#define NOP() (Internal::new_NoOperation<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>())

// SequenceMode
#define SEQ_MODE() (Internal::new_SequenceMode<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>())

// Track
#define TRC(track_id) (Internal::new_Tracking<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(track_id))

// TrackTap
#define TRT(track_id, command) (Internal::new_TrackTap<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(track_id, command))

// nullptr alias (_ * 7)
#define _______ (static_cast<Command *>(nullptr))

} // namespace hidpg
