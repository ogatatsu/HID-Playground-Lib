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

#include "CommandImpl.h"
#include "etl/array.h"
#include "etl/vector.h"

namespace hidpg
{
  // CommandDsl Dynamic Memory Allocation virsion.

  static inline NotNullCommandPtr NK(KeyCode key_code) { return (new Internal::NormalKey(key_code)); }

  static inline NotNullCommandPtr MO(Modifiers modifiers) { return (new Internal::ModifierKey(modifiers)); }

  static inline NotNullCommandPtr CK(Modifiers modifiers, KeyCode key_code) { return (new Internal::CombinationKey(modifiers, key_code)); }

  static inline NotNullCommandPtr LY(const CommandPtr (&commands)[HID_ENGINE_LAYER_SIZE])
  {
    auto _commands = new etl::array<CommandPtr, HID_ENGINE_LAYER_SIZE>;
    _commands->assign(std::begin(commands), std::end(commands));

    return (new Internal::Layering(Layer, *_commands));
  }

  static inline NotNullCommandPtr LY1(const CommandPtr (&commands)[HID_ENGINE_LAYER_SIZE])
  {
    auto _commands = new etl::array<CommandPtr, HID_ENGINE_LAYER_SIZE>;
    _commands->assign(std::begin(commands), std::end(commands));

    return (new Internal::Layering(Layer1, *_commands));
  }

  static inline NotNullCommandPtr LY2(const CommandPtr (&commands)[HID_ENGINE_LAYER_SIZE])
  {
    auto _commands = new etl::array<CommandPtr, HID_ENGINE_LAYER_SIZE>;
    _commands->assign(std::begin(commands), std::end(commands));

    return (new Internal::Layering(Layer2, *_commands));
  }

  static inline NotNullCommandPtr TL(uint8_t layer_number) { return (new Internal::ToggleLayer(Layer, layer_number)); }
  static inline NotNullCommandPtr TL1(uint8_t layer_number) { return (new Internal::ToggleLayer(Layer1, layer_number)); }
  static inline NotNullCommandPtr TL2(uint8_t layer_number) { return (new Internal::ToggleLayer(Layer2, layer_number)); }

  static inline NotNullCommandPtr SL(uint8_t layer_number) { return (new Internal::SwitchLayer(Layer, layer_number)); }
  static inline NotNullCommandPtr SL1(uint8_t layer_number) { return (new Internal::SwitchLayer(Layer1, layer_number)); }
  static inline NotNullCommandPtr SL2(uint8_t layer_number) { return (new Internal::SwitchLayer(Layer2, layer_number)); }

  static inline NotNullCommandPtr UPL(uint8_t i) { return (new Internal::UpDefaultLayer(Layer, i)); }
  static inline NotNullCommandPtr UPL1(uint8_t i) { return (new Internal::UpDefaultLayer(Layer1, i)); }
  static inline NotNullCommandPtr UPL2(uint8_t i) { return (new Internal::UpDefaultLayer(Layer2, i)); }

  static inline NotNullCommandPtr TAP(NotNullCommandPtr command, uint8_t n_times = 1, uint16_t tap_speed_ms = HID_ENGINE_TAP_SPEED_MS) { return (new Internal::Tap(command, n_times, tap_speed_ms)); }

  static inline NotNullCommandPtr TAP_R(NotNullCommandPtr command, uint8_t n_times = 1, uint16_t tap_speed_ms = HID_ENGINE_TAP_SPEED_MS) { return (new Internal::TapWhenReleased(command, n_times, tap_speed_ms)); }

  template <uint8_t N>
  static NotNullCommandPtr TD(const Internal::TapDance::Pair (&pairs)[N], const uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto _pairs = new etl::vector<Internal::TapDance::Pair, N>{std::begin(pairs), std::end(pairs)};
    auto pointing_device_ids = new etl::span<PointingDeviceId>;

    return (new Internal::TapDance(*_pairs, *pointing_device_ids, 0, tapping_term_ms));
  }

  template <uint8_t N1, uint8_t N2>
  static NotNullCommandPtr TD_DPM(const Internal::TapDance::Pair (&pairs)[N1],
                                  const PointingDeviceId (&pointing_device_ids)[N2],
                                  const uint16_t move_threshold = 0,
                                  const uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto _pairs = new etl::vector<Internal::TapDance::Pair, N1>{std::begin(pairs), std::end(pairs)};
    auto _pointing_device_ids = new etl::array<PointingDeviceId, N2>;
    _pointing_device_ids->assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));

    return (new Internal::TapDance(*_pairs, *_pointing_device_ids, move_threshold, tapping_term_ms));
  }

  static NotNullCommandPtr HT(NotNullCommandPtr hold_command,
                              NotNullCommandPtr tap_command,
                              const uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::vector<Internal::TapDance::Pair, 1>{
        {hold_command, tap_command},
    };
    auto pointing_device_ids = new etl::span<PointingDeviceId>;

    return (new Internal::TapDance(*pairs, *pointing_device_ids, 0, tapping_term_ms));
  }

  template <uint8_t N>
  static NotNullCommandPtr HT_DPM(NotNullCommandPtr hold_command,
                                  NotNullCommandPtr tap_command,
                                  const PointingDeviceId (&pointing_device_ids)[N],
                                  const uint16_t move_threshold = 0,
                                  const uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::vector<Internal::TapDance::Pair, 1>{
        {hold_command, tap_command},
    };
    auto _pointing_device_ids = new etl::array<PointingDeviceId, N>;
    _pointing_device_ids->assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));

    return (new Internal::TapDance(*pairs, *_pointing_device_ids, move_threshold, tapping_term_ms));
  }

  static inline NotNullCommandPtr CC(ConsumerControlCode usage_code) { return (new Internal::ConsumerControl(usage_code)); }

  static inline NotNullCommandPtr SC(SystemControlCode usage_code) { return (new Internal::SystemControl(usage_code)); }

  static inline NotNullCommandPtr MS_MOV(int16_t x, int16_t y) { return (new Internal::MouseMove(x, y)); }

  static inline NotNullCommandPtr MS_SCR(int8_t scroll, int8_t horiz) { return (new Internal::MouseScroll(scroll, horiz)); }

  static inline NotNullCommandPtr MS_CLK(MouseButtons buttons) { return (new Internal::MouseClick(buttons)); }

  static inline NotNullCommandPtr RD_CLK() { return (new Internal::RadialClick()); }

  static inline NotNullCommandPtr RD_ROT(int16_t deci_degree)
  {
    deci_degree = etl::clamp<int16_t>(deci_degree, -3600, 3600);
    return (new Internal::RadialRotate(deci_degree));
  }

  static inline NotNullCommandPtr OE(NotNullCommandPtr command, uint32_t ms) { return (new Internal::OnceEvery(command, ms)); }

  static inline NotNullCommandPtr NTE(NotNullCommandPtr command, uint32_t ms) { return (new Internal::NTimesEvery(command, ms)); }

  static inline NotNullCommandPtr IF(bool (*func)(), NotNullCommandPtr true_command, NotNullCommandPtr false_command) { return (new Internal::If(func, true_command, false_command)); }

  template <uint8_t N>
  static NotNullCommandPtr MLT(const NotNullCommandPtr (&commands)[N])
  {
    auto _commands = new etl::vector<NotNullCommandPtr, N>{std::begin(commands), std::end(commands)};
    return (new Internal::Multi(*_commands));
  }

  static inline NotNullCommandPtr TGL(NotNullCommandPtr command) { return (new Internal::Toggle(command)); }

  static inline NotNullCommandPtr RPT(NotNullCommandPtr command, uint32_t delay_ms, uint32_t interval_ms) { return (new Internal::Repeat(command, delay_ms, interval_ms)); }

  template <uint8_t N>
  static inline NotNullCommandPtr CYC(const NotNullCommandPtr (&commands)[N])
  {
    auto _commands = new etl::vector<NotNullCommandPtr, N>{std::begin(commands), std::end(commands)};
    return new Internal::Cycle(*_commands);
  }

  template <uint8_t N>
  static inline NotNullCommandPtr CYC_PS(const NotNullCommandPtr (&commands)[N])
  {
    auto _commands = new etl::vector<NotNullCommandPtr, N>{std::begin(commands), std::end(commands)};
    return (new Internal::CyclePhaseShift(*_commands));
  }

  static inline NotNullCommandPtr NOP() { return (new Internal::NoOperation()); }

  static inline NotNullCommandPtr SFT(Internal::ShiftIds1 shift_ids)
  {
    auto shift_id_links = new etl::array<Internal::ShiftIdLink, 1>{
        Internal::Shift::IdToLink(shift_ids.id0),
    };
    return (new Internal::Shift(*shift_id_links));
  }

  static inline NotNullCommandPtr SFT(Internal::ShiftIds2 shift_ids)
  {
    auto shift_id_links = new etl::array<Internal::ShiftIdLink, 2>{
        Internal::Shift::IdToLink(shift_ids.id0),
        Internal::Shift::IdToLink(shift_ids.id1),
    };
    return (new Internal::Shift(*shift_id_links));
  }

  static inline NotNullCommandPtr SFT(Internal::ShiftIds3 shift_ids)
  {
    auto shift_id_links = new etl::array<Internal::ShiftIdLink, 3>{
        Internal::Shift::IdToLink(shift_ids.id0),
        Internal::Shift::IdToLink(shift_ids.id1),
        Internal::Shift::IdToLink(shift_ids.id2),
    };
    return (new Internal::Shift(*shift_id_links));
  }

  static inline NotNullCommandPtr SFT(Internal::ShiftIds4 shift_ids)
  {
    auto shift_id_links = new etl::array<Internal::ShiftIdLink, 4>{
        Internal::Shift::IdToLink(shift_ids.id0),
        Internal::Shift::IdToLink(shift_ids.id1),
        Internal::Shift::IdToLink(shift_ids.id2),
        Internal::Shift::IdToLink(shift_ids.id3),
    };
    return (new Internal::Shift(*shift_id_links));
  }

  static inline NotNullCommandPtr SFT(Internal::ShiftIds5 shift_ids)
  {
    auto shift_id_links = new etl::array<Internal::ShiftIdLink, 5>{
        Internal::Shift::IdToLink(shift_ids.id0),
        Internal::Shift::IdToLink(shift_ids.id1),
        Internal::Shift::IdToLink(shift_ids.id2),
        Internal::Shift::IdToLink(shift_ids.id3),
        Internal::Shift::IdToLink(shift_ids.id4),
    };
    return (new Internal::Shift(*shift_id_links));
  }

  template <size_t N>
  static inline etl::span<Key> KEYMAP(const Key (&keymap)[N])
  {
    auto result = new etl::vector<Key, N>;
    result->assign(std::begin(keymap), std::end(keymap));
    return *result;
  }

// nullptr alias (_ * 7)
#define _______ (nullptr)

} // namespace hidpg
