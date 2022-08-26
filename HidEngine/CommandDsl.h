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
#include "etl/array.h"
#include "etl/optional.h"
#include "etl/vector.h"
#include <new>

namespace hidpg
{

  namespace Internal
  {

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_NormalKey(KeyCode key_code)
    {
      static uint8_t buf[sizeof(NormalKey)];
      return new (buf) NormalKey(key_code);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_ModifierKey(Modifiers modifiers)
    {
      static uint8_t buf[sizeof(ModifierKey)];
      return new (buf) ModifierKey(modifiers);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_CombinationKey(Modifiers modifiers, KeyCode key_code)
    {
      static uint8_t buf[sizeof(CombinationKey)];
      return new (buf) CombinationKey(modifiers, key_code);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_ModifierTap(Modifiers modifiers, NotNullCommandPtr command, TapHoldBehavior behavior = TapHoldBehavior::HoldPreferred)
    {
      static uint8_t cmd_buf[sizeof(ModifierKey)];
      static etl::vector<Internal::TapDance::Pair, 1> pairs{
          {command, new (cmd_buf) Internal::ModifierKey(modifiers)},
      };
      static etl::span<uint8_t> mouse_ids;
      static uint8_t buf[sizeof(TapDance)];

      return new (buf) TapDance(pairs, mouse_ids, 0, behavior);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Layering(LayerClass &layer, const CommandPtr (&commands)[HID_ENGINE_LAYER_SIZE])
    {
      static etl::array<CommandPtr, HID_ENGINE_LAYER_SIZE> _commands;
      _commands.assign(std::begin(commands), std::end(commands));
      static uint8_t buf[sizeof(Layering)];

      return new (buf) Layering(layer, _commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_LayerTap(LayerClass &layer, uint8_t layer_number, NotNullCommandPtr command, TapHoldBehavior behavior = TapHoldBehavior::HoldPreferred)
    {
      static uint8_t cmd_buf[sizeof(SwitchLayer)];
      static etl::vector<Internal::TapDance::Pair, 1> pairs{
          {command, new (cmd_buf) SwitchLayer(layer, layer_number)},
      };
      static etl::span<uint8_t> mouse_ids;
      static uint8_t buf[sizeof(TapDance)];

      return new (buf) TapDance(pairs, mouse_ids, 0, behavior);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_ToggleLayer(LayerClass &layer, uint8_t layer_number)
    {
      static uint8_t buf[sizeof(ToggleLayer)];
      return new (buf) ToggleLayer(layer, layer_number);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_SwitchLayer(LayerClass &layer, uint8_t layer_number)
    {
      static uint8_t buf[sizeof(SwitchLayer)];
      return new (buf) SwitchLayer(layer, layer_number);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_UpDefaultLayer(LayerClass &layer, uint8_t i)
    {
      static uint8_t buf[sizeof(UpDefaultLayer)];
      return new (buf) UpDefaultLayer(layer, i);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Tap(NotNullCommandPtr command, uint8_t n_times = 1, uint16_t tap_speed_ms = HID_ENGINE_TAP_SPEED_MS)
    {
      static uint8_t buf[sizeof(Tap)];
      return new (buf) Tap(command, n_times, tap_speed_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_TapWhenReleased(NotNullCommandPtr command, uint8_t n_times = 1, uint16_t tap_speed_ms = HID_ENGINE_TAP_SPEED_MS)
    {
      static uint8_t buf[sizeof(TapWhenReleased)];
      return new (buf) TapWhenReleased(command, n_times, tap_speed_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    NotNullCommandPtr new_TapDance(const TapDance::Pair (&pairs)[N], TapHoldBehavior behavior = TapHoldBehavior::HoldPreferred)
    {
      static etl::vector<Internal::TapDance::Pair, N> _pairs{std::begin(pairs), std::end(pairs)};
      static etl::span<uint8_t> mouse_ids;
      static uint8_t buf[sizeof(TapDance)];

      return new (buf) TapDance(_pairs, mouse_ids, 0, behavior);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N1, size_t N2>
    NotNullCommandPtr new_TapDanceDecideWithMouseMove(const TapDance::Pair (&pairs)[N1],
                                                      const uint8_t (&mouse_ids)[N2],
                                                      uint16_t move_threshold = 4,
                                                      TapHoldBehavior behavior = TapHoldBehavior::HoldPreferred)
    {
      static etl::vector<Internal::TapDance::Pair, N1> _pairs{std::begin(pairs), std::end(pairs)};
      static etl::array<uint8_t, N2> _mouse_ids;
      _mouse_ids.assign(std::begin(mouse_ids), std::end(mouse_ids));
      static uint8_t buf[sizeof(TapDance)];

      return new (buf) TapDance(_pairs, _mouse_ids, move_threshold, behavior);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_TapOrHold(NotNullCommandPtr tap_command, unsigned int ms, NotNullCommandPtr hold_command)
    {
      static uint8_t buf[sizeof(TapOrHold)];
      return new (buf) TapOrHold(tap_command, ms, hold_command);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_ConsumerControl(ConsumerControlCode usage_code)
    {
      static uint8_t buf[sizeof(ConsumerControl)];
      return new (buf) ConsumerControl(usage_code);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_SystemControl(SystemControlCode usage_code)
    {
      static uint8_t buf[sizeof(SystemControl)];
      return new (buf) SystemControl(usage_code);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_MouseMove(int16_t x, int16_t y)
    {
      static uint8_t buf[sizeof(MouseMove)];
      return new (buf) MouseMove(x, y);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_MouseScroll(int8_t scroll, int8_t horiz)
    {
      static uint8_t buf[sizeof(MouseScroll)];
      return new (buf) MouseScroll(scroll, horiz);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_MouseClick(MouseButtons buttons)
    {
      static uint8_t buf[sizeof(MouseClick)];
      return new (buf) MouseClick(buttons);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_RadialClick()
    {
      static uint8_t buf[sizeof(RadialClick)];
      return new (buf) RadialClick();
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_RadialRotate(int16_t deci_degree)
    {
      deci_degree = etl::clamp<int16_t>(deci_degree, -3600, 3600);

      static uint8_t buf[sizeof(RadialRotate)];
      return new (buf) RadialRotate(deci_degree);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_OnceEvery(NotNullCommandPtr command, uint32_t ms)
    {
      static uint8_t buf[sizeof(OnceEvery)];
      return new (buf) OnceEvery(command, ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_NTimesEvery(NotNullCommandPtr command, uint32_t ms)
    {
      static uint8_t buf[sizeof(NTimesEvery)];
      return new (buf) NTimesEvery(command, ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_If(bool (*func)(), NotNullCommandPtr true_command, NotNullCommandPtr false_command)
    {
      static uint8_t buf[sizeof(If)];
      return new (buf) If(func, true_command, false_command);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    NotNullCommandPtr new_Multi(const NotNullCommandPtr (&commands)[N])
    {
      static etl::vector<NotNullCommandPtr, N> _commands{std::begin(commands), std::end(commands)};
      static uint8_t buf[sizeof(Multi)];

      return new (buf) Multi(_commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Toggle(NotNullCommandPtr command)
    {
      static uint8_t buf[sizeof(Toggle)];
      return new (buf) Toggle(command);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Repeat(NotNullCommandPtr command, uint32_t delay_ms, uint32_t interval_ms)
    {
      static uint8_t buf[sizeof(Repeat)];
      return new (buf) Repeat(command, delay_ms, interval_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    NotNullCommandPtr new_Cycle(const NotNullCommandPtr (&commands)[N])
    {
      static etl::vector<NotNullCommandPtr, N> _commands{std::begin(commands), std::end(commands)};
      static uint8_t buf[sizeof(Cycle)];

      return new (buf) Cycle(_commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    NotNullCommandPtr new_CyclePhaseShift(const NotNullCommandPtr (&commands)[N])
    {
      static etl::vector<NotNullCommandPtr, N> _commands{std::begin(commands), std::end(commands)};
      static uint8_t buf[sizeof(CyclePhaseShift)];

      return new (buf) CyclePhaseShift(_commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_NoOperation()
    {
      static uint8_t buf[sizeof(NoOperation)];
      return new (buf) NoOperation();
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_GestureCommand(uint8_t gesture_id)
    {
      static uint8_t buf[sizeof(GestureCommand)];
      return new (buf) GestureCommand(gesture_id);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_GestureOr(uint8_t gesture_id, NotNullCommandPtr command)
    {
      static uint8_t buf[sizeof(GestureOr)];
      return new (buf) GestureOr(gesture_id, command);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_GestureOrNK(uint8_t gesture_id, KeyCode key_code)
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
#define LY(...) (Internal::new_Layering<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer, __VA_ARGS__))
#define LY1(...) (Internal::new_Layering<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer1, __VA_ARGS__))
#define LY2(...) (Internal::new_Layering<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer2, __VA_ARGS__))

// LayerTap
#define LT(...) (Internal::new_LayerTap<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer, __VA_ARGS__))
#define LT1(...) (Internal::new_LayerTap<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer1, __VA_ARGS__))
#define LT2(...) (Internal::new_LayerTap<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer2, __VA_ARGS__))

// ToggleLayer
#define TL(layer_number) (Internal::new_ToggleLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer, layer_number))
#define TL1(layer_number) (Internal::new_ToggleLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer1, layer_number))
#define TL2(layer_number) (Internal::new_ToggleLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer2, layer_number))

// SwitchLayer
#define SL(layer_number) (Internal::new_SwitchLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer, layer_number))
#define SL1(layer_number) (Internal::new_SwitchLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer1, layer_number))
#define SL2(layer_number) (Internal::new_SwitchLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer2, layer_number))

// UpDefaultLayer
#define UPL(i) (Internal::new_UpDefaultLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer, i))
#define UPL1(i) (Internal::new_UpDefaultLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer1, i))
#define UPL2(i) (Internal::new_UpDefaultLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer2, i))

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

// MouseScroll
#define MS_SCR(scroll, horiz) (Internal::new_MouseScroll<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(scroll, horiz))

// MouseClick
#define MS_CLK(buttons) (Internal::new_MouseClick<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(buttons))

// RadialClick
#define RD_CLK() (Internal::new_RadialClick<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>())

// RadialRotate
#define RD_ROT(deci_degree) (Internal::new_RadialRotate<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(deci_degree))

// OnceEvery
#define OE(command, ms) (Internal::new_OnceEvery<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(command, ms))

// NTimesEvery
#define NTE(command, ms) (Internal::new_NTimesEvery<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(command, ms))

// If
#define IF(func, true_command, false_command) (Internal::new_If<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(func, true_command, false_command))

// Multi
#define MLT(...) (Internal::new_Multi<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

// Toggle
#define TGL(command) (Internal::new_Toggle<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(command))

// Repeat
#define RPT(command, delay_ms, interval_ms) (Internal::new_Repeat<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(command, delay_ms, interval_ms))

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
#define _______ (nullptr)

} // namespace hidpg
