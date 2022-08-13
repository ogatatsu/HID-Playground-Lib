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
#include "consthash/cityhash64.hxx"
#include "consthash/crc64.hxx"
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
    Command *new_ModifierTap(Modifiers modifiers, Command *command, TapHoldBehavior behavior = TapHoldBehavior::HoldPreferred)
    {
      static TapDance::Pair arg[1];
      static uint8_t buf[sizeof(TapDance)];
      static uint8_t mod_cmd_buf[sizeof(ModifierKey)];

      arg[0].tap_command = command;
      arg[0].hold_command = new (mod_cmd_buf) ModifierKey(modifiers);

      return new (buf) TapDance(arg, 1, nullptr, 0, 0, behavior);
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
    Command *new_LayerTap(LayerClass *layer, uint8_t layer_number, Command *command, TapHoldBehavior behavior = TapHoldBehavior::HoldPreferred)
    {
      static TapDance::Pair arg[1];
      static uint8_t buf[sizeof(TapDance)];
      static uint8_t ly_cmd_buf[sizeof(SwitchLayer)];

      arg[0].tap_command = command;
      arg[0].hold_command = new (ly_cmd_buf) SwitchLayer(layer, layer_number);

      return new (buf) TapDance(arg, 1, nullptr, 0, 0, behavior);
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
    Command *new_UpDefaultLayer(LayerClass *layer, uint8_t i)
    {
      static uint8_t buf[sizeof(UpDefaultLayer)];
      return new (buf) UpDefaultLayer(layer, i);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_Tap(Command *command, uint8_t n_times = 1, uint16_t tap_speed_ms = HID_ENGINE_TAP_SPEED_MS)
    {
      static uint8_t buf[sizeof(Tap)];
      return new (buf) Tap(command, n_times, tap_speed_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_TapWhenReleased(Command *command, uint8_t n_times = 1, uint16_t tap_speed_ms = HID_ENGINE_TAP_SPEED_MS)
    {
      static uint8_t buf[sizeof(TapWhenReleased)];
      return new (buf) TapWhenReleased(command, n_times, tap_speed_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, uint8_t N>
    Command *new_TapDance(const TapDance::Pair (&arr)[N], TapHoldBehavior behavior = TapHoldBehavior::HoldPreferred)
    {
      static TapDance::Pair arg[N];
      static uint8_t buf[sizeof(TapDance)];

      for (size_t i = 0; i < N; i++)
      {
        arg[i].tap_command = arr[i].tap_command;
        arg[i].hold_command = arr[i].hold_command;
      }
      return new (buf) TapDance(arg, N, nullptr, 0, 0, behavior);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, uint8_t N1, uint8_t N2>
    Command *new_TapDanceDecideWithMouseMove(const TapDance::Pair (&arr)[N1],
                                             const uint8_t (&mouse_ids)[N2],
                                             uint16_t move_threshold = 4,
                                             TapHoldBehavior behavior = TapHoldBehavior::HoldPreferred)
    {
      static TapDance::Pair arg1[N1];
      static uint8_t arg2[N2];
      static uint8_t buf[sizeof(TapDance)];

      for (size_t i = 0; i < N1; i++)
      {
        arg1[i].tap_command = arr[i].tap_command;
        arg1[i].hold_command = arr[i].hold_command;
      }

      for (size_t i = 0; i < N2; i++)
      {
        arg2[i] = mouse_ids[i];
      }

      return new (buf) TapDance(arg1, N1, arg2, N2, move_threshold, behavior);
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
    Command *new_OnceEvery(uint32_t ms, Command *command)
    {
      static uint8_t buf[sizeof(OnceEvery)];
      return new (buf) OnceEvery(ms, command);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_NTimesEvery(uint32_t ms, Command *command)
    {
      static uint8_t buf[sizeof(NTimesEvery)];
      return new (buf) NTimesEvery(ms, command);
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
    Command *new_Toggle(Command *command)
    {
      static uint8_t buf[sizeof(Toggle)];
      return new (buf) Toggle(command);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    Command *new_Cycle(const CommandPtr (&arr)[N])
    {
      static Command *arg[N];
      static uint8_t buf[sizeof(Cycle)];

      for (size_t i = 0; i < N; i++)
      {
        arg[i] = arr[i];
      }
      return new (buf) Cycle(arg, N);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    Command *new_CyclePhaseShift(const CommandPtr (&arr)[N])
    {
      static Command *arg[N];
      static uint8_t buf[sizeof(CyclePhaseShift)];

      for (size_t i = 0; i < N; i++)
      {
        arg[i] = arr[i];
      }
      return new (buf) CyclePhaseShift(arg, N);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_NoOperation()
    {
      static uint8_t buf[sizeof(NoOperation)];
      return new (buf) NoOperation();
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_GestureCommand(uint8_t gesture_id)
    {
      static uint8_t buf[sizeof(GestureCommand)];
      return new (buf) GestureCommand(gesture_id);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_GestureOr(uint8_t gesture_id, Command *command)
    {
      static uint8_t buf[sizeof(GestureOr)];
      return new (buf) GestureOr(gesture_id, command);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_GestureOrNK(uint8_t gesture_id, KeyCode key_code)
    {
      static uint8_t buf[sizeof(GestureOrNK)];
      return new (buf) GestureOrNK(gesture_id, key_code);
    }

  } // namespace Internal

// NormalKey
#define NK(key_code) (Internal::new_NormalKey<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(key_code))

// ModifierKey
#define MO(modifiers) (Internal::new_ModifierKey<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(modifiers))

// CombinationKey
#define CK(modifiers, key_code) (Internal::new_CombinationKey<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(modifiers, key_code))

// ModifierTap
#define MT(...) (Internal::new_ModifierTap<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

// Layering
#define LY(...) (Internal::new_Layering<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(&Layer, __VA_ARGS__))
#define LY1(...) (Internal::new_Layering<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(&Layer1, __VA_ARGS__))
#define LY2(...) (Internal::new_Layering<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(&Layer2, __VA_ARGS__))

// LayerTap
#define LT(...) (Internal::new_LayerTap<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(&Layer, __VA_ARGS__))
#define LT1(...) (Internal::new_LayerTap<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(&Layer1, __VA_ARGS__))
#define LT2(...) (Internal::new_LayerTap<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(&Layer2, __VA_ARGS__))

// ToggleLayer
#define TL(layer_number) (Internal::new_ToggleLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(&Layer, layer_number))
#define TL1(layer_number) (Internal::new_ToggleLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(&Layer1, layer_number))
#define TL2(layer_number) (Internal::new_ToggleLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(&Layer2, layer_number))

// SwitchLayer
#define SL(layer_number) (Internal::new_SwitchLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(&Layer, layer_number))
#define SL1(layer_number) (Internal::new_SwitchLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(&Layer1, layer_number))
#define SL2(layer_number) (Internal::new_SwitchLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(&Layer2, layer_number))

// UpDefaultLayer
#define UPL(i) (Internal::new_UpDefaultLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(&Layer, i))
#define UPL1(i) (Internal::new_UpDefaultLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(&Layer1, i))
#define UPL2(i) (Internal::new_UpDefaultLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(&Layer2, i))

// Tap
#define TAP(...) (Internal::new_Tap<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

// TapWhenReleased
#define TAP_R(...) (Internal::new_TapWhenReleased<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

// TapDance
#define TD(...) (Internal::new_TapDance<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

// TapDanceDecideWithMouseMove
#define TDDM(...) (Internal::new_TapDanceDecideWithMouseMove<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

// TapOrHold
#define ToH(tap_command, ms, hold_command) (Internal::new_TapOrHold<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(tap_command, ms, hold_command))

// ConsumerControl
#define CC(usage_code) (Internal::new_ConsumerControl<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(usage_code))

// SystemControl
#define SC(usage_code) (Internal::new_SystemControl<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(usage_code))

// MouseMove
#define MS_MOV(x, y) (Internal::new_MouseMove<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(x, y))

// MouseSpeed
#define MS_SPD(percent) (Internal::new_MouseSpeed<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(percent))

// MouseScroll
#define MS_SCR(scroll, horiz) (Internal::new_MouseScroll<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(scroll, horiz))

// MouseClick
#define MS_CLK(buttons) (Internal::new_MouseClick<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(buttons))

// RadialClick
#define RD_CLK() (Internal::new_RadialClick<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>())

// RadialRotate
#define RD_ROT(deci_degree) (Internal::new_RadialRotate<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(deci_degree))

// OnceEvery
#define OE(ms, command) (Internal::new_OnceEvery<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(ms, command))

// NTimesEvery
#define NTE(ms, command) (Internal::new_NTimesEvery<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(ms, command))

// If
#define IF(func, true_command, false_command) (Internal::new_If<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(func, true_command, false_command))

// Multi
#define MLT(...) (Internal::new_Multi<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

// Toggle
#define TGL(command) (Internal::new_Toggle<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(command))

// Cycle
#define CYC(...) (Internal::new_Cycle<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

// CyclePhaseShift
#define CYC_PS(...) (Internal::new_CyclePhaseShift<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

// NoOperation
#define NOP() (Internal::new_NoOperation<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>())

// Gesture
#define GST(gesture_id) (Internal::new_GestureCommand<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(gesture_id))

// GestureOr
#define GST_OR(gesture_id, command) (Internal::new_GestureOr<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(gesture_id, command))

// GestureOrNK
#define GST_OR_NK(gesture_id, key_code) (Internal::new_GestureOrNK<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(gesture_id, key_code))

// nullptr alias (_ * 7)
#define _______ (static_cast<Command *>(nullptr))

} // namespace hidpg
