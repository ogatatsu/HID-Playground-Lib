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
  //------------------------------------------------------------------+
  // KeyCode(KC)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr KC(CharacterKey character_key)
  {
    return new Internal::CharacterKeyCommand(character_key);
  }

  static inline NotNullCommandPtr KC(Modifiers modifiers)
  {
    return new Internal::ModifiersCommand(modifiers);
  }

  static inline NotNullCommandPtr KC(CombinationKeys combination_keys)
  {
    return new Internal::CombinationKeysCommand(combination_keys);
  }

  static inline NotNullCommandPtr KC(ConsumerControlCode consumer_control_code)
  {
    return new Internal::ConsumerControl(consumer_control_code);
  }

  static inline NotNullCommandPtr KC(SystemControlCode system_control_code)
  {
    return new Internal::SystemControl(system_control_code);
  }

  //------------------------------------------------------------------+
  // Layering(LY)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr LY(const CommandPtr (&commands)[HID_ENGINE_LAYER_SIZE])
  {
    auto _commands = new etl::array<CommandPtr, HID_ENGINE_LAYER_SIZE>;
    _commands->assign(std::begin(commands), std::end(commands));
    return new Internal::Layering(Layer, *_commands);
  }

  static inline NotNullCommandPtr LY1(const CommandPtr (&commands)[HID_ENGINE_LAYER_SIZE])
  {
    auto _commands = new etl::array<CommandPtr, HID_ENGINE_LAYER_SIZE>;
    _commands->assign(std::begin(commands), std::end(commands));
    return new Internal::Layering(Layer1, *_commands);
  }

  static inline NotNullCommandPtr LY2(const CommandPtr (&commands)[HID_ENGINE_LAYER_SIZE])
  {
    auto _commands = new etl::array<CommandPtr, HID_ENGINE_LAYER_SIZE>;
    _commands->assign(std::begin(commands), std::end(commands));
    return new Internal::Layering(Layer2, *_commands);
  }

  //------------------------------------------------------------------+
  // ToggleLayer(TL)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr TL(uint8_t layer_number)
  {
    return new Internal::ToggleLayer(Layer, layer_number);
  }

  static inline NotNullCommandPtr TL1(uint8_t layer_number)
  {
    return new Internal::ToggleLayer(Layer1, layer_number);
  }

  static inline NotNullCommandPtr TL2(uint8_t layer_number)
  {
    return new Internal::ToggleLayer(Layer2, layer_number);
  }

  //------------------------------------------------------------------+
  // SwitchLayer(SL)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr SL(uint8_t layer_number)
  {
    return new Internal::SwitchLayer(Layer, layer_number);
  }

  static inline NotNullCommandPtr SL1(uint8_t layer_number)
  {
    return new Internal::SwitchLayer(Layer1, layer_number);
  }

  static inline NotNullCommandPtr SL2(uint8_t layer_number)
  {
    return new Internal::SwitchLayer(Layer2, layer_number);
  }

  //------------------------------------------------------------------+
  // UpDefaultLayer(UPL)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr UPL(uint8_t i)
  {
    return new Internal::UpDefaultLayer(Layer, i);
  }

  static inline NotNullCommandPtr UPL1(uint8_t i)
  {
    return new Internal::UpDefaultLayer(Layer1, i);
  }

  static inline NotNullCommandPtr UPL2(uint8_t i)
  {
    return new Internal::UpDefaultLayer(Layer2, i);
  }

  //------------------------------------------------------------------+
  // Tap(TAP)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr TAP(NotNullCommandPtr command, uint8_t n_times = 1, uint16_t tap_speed_ms = HID_ENGINE_TAP_SPEED_MS)
  {
    return new Internal::Tap(command, n_times, tap_speed_ms);
  }

  //------------------------------------------------------------------+
  // TapWhenReleased(TAP_R)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr TAP_R(NotNullCommandPtr command, uint8_t n_times = 1, uint16_t tap_speed_ms = HID_ENGINE_TAP_SPEED_MS)
  {
    return new Internal::TapWhenReleased(command, n_times, tap_speed_ms);
  }

  //------------------------------------------------------------------+
  // TapDance(TD)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr TD(const Internal::TapDance::Pair pair0,
                                     const Internal::TapDance::Pair pair1,
                                     uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 2>{pair0, pair1};
    auto pointing_device_ids = new etl::span<PointingDeviceId>;
    return new Internal::TapDance(*pairs, *pointing_device_ids, 0, tapping_term_ms);
  }

  static inline NotNullCommandPtr TD(const Internal::TapDance::Pair pair0,
                                     const Internal::TapDance::Pair pair1,
                                     const Internal::TapDance::Pair pair2,
                                     uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 3>{pair0, pair1, pair2};
    auto pointing_device_ids = new etl::span<PointingDeviceId>;
    return new Internal::TapDance(*pairs, *pointing_device_ids, 0, tapping_term_ms);
  }

  static inline NotNullCommandPtr TD(const Internal::TapDance::Pair pair0,
                                     const Internal::TapDance::Pair pair1,
                                     const Internal::TapDance::Pair pair2,
                                     const Internal::TapDance::Pair pair3,
                                     uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 4>{pair0, pair1, pair2, pair3};
    auto pointing_device_ids = new etl::span<PointingDeviceId>;
    return new Internal::TapDance(*pairs, *pointing_device_ids, 0, tapping_term_ms);
  }

  static inline NotNullCommandPtr TD(const Internal::TapDance::Pair pair0,
                                     const Internal::TapDance::Pair pair1,
                                     const Internal::TapDance::Pair pair2,
                                     const Internal::TapDance::Pair pair3,
                                     const Internal::TapDance::Pair pair4,
                                     uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 5>{pair0, pair1, pair2, pair3, pair4};
    auto pointing_device_ids = new etl::span<PointingDeviceId>;
    return new Internal::TapDance(*pairs, *pointing_device_ids, 0, tapping_term_ms);
  }

  static inline NotNullCommandPtr TD(const Internal::TapDance::Pair pair0,
                                     const Internal::TapDance::Pair pair1,
                                     const Internal::TapDance::Pair pair2,
                                     const Internal::TapDance::Pair pair3,
                                     const Internal::TapDance::Pair pair4,
                                     const Internal::TapDance::Pair pair5,
                                     uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 6>{pair0, pair1, pair2, pair3, pair4, pair5};
    auto pointing_device_ids = new etl::span<PointingDeviceId>;
    return new Internal::TapDance(*pairs, *pointing_device_ids, 0, tapping_term_ms);
  }

  static inline NotNullCommandPtr TD(const Internal::TapDance::Pair pair0,
                                     const Internal::TapDance::Pair pair1,
                                     const Internal::TapDance::Pair pair2,
                                     const Internal::TapDance::Pair pair3,
                                     const Internal::TapDance::Pair pair4,
                                     const Internal::TapDance::Pair pair5,
                                     const Internal::TapDance::Pair pair6,
                                     uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 7>{pair0, pair1, pair2, pair3, pair4, pair5, pair6};
    auto pointing_device_ids = new etl::span<PointingDeviceId>;
    return new Internal::TapDance(*pairs, *pointing_device_ids, 0, tapping_term_ms);
  }

  static inline NotNullCommandPtr TD(const Internal::TapDance::Pair pair0,
                                     const Internal::TapDance::Pair pair1,
                                     const Internal::TapDance::Pair pair2,
                                     const Internal::TapDance::Pair pair3,
                                     const Internal::TapDance::Pair pair4,
                                     const Internal::TapDance::Pair pair5,
                                     const Internal::TapDance::Pair pair6,
                                     const Internal::TapDance::Pair pair7,
                                     uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 8>{pair0, pair1, pair2, pair3, pair4, pair5, pair6, pair7};
    auto pointing_device_ids = new etl::span<PointingDeviceId>;
    return new Internal::TapDance(*pairs, *pointing_device_ids, 0, tapping_term_ms);
  }

  //------------------------------------------------------------------+
  // TapDanceDecideWithPointerMove(TD_DPM)
  //------------------------------------------------------------------+
  template <uint8_t N>
  static inline NotNullCommandPtr TD_DPM(const Internal::TapDance::Pair pair0,
                                         const Internal::TapDance::Pair pair1,
                                         const PointingDeviceId (&pointing_device_ids)[N],
                                         uint16_t move_threshold = 0,
                                         uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 2>{pair0, pair1};
    auto _pointing_device_ids = new etl::array<PointingDeviceId, N>;
    _pointing_device_ids->assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
    return new Internal::TapDance(*pairs, *_pointing_device_ids, move_threshold, tapping_term_ms);
  }

  template <uint8_t N>
  static inline NotNullCommandPtr TD_DPM(const Internal::TapDance::Pair pair0,
                                         const Internal::TapDance::Pair pair1,
                                         const Internal::TapDance::Pair pair2,
                                         const PointingDeviceId (&pointing_device_ids)[N],
                                         uint16_t move_threshold = 0,
                                         uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 3>{pair0, pair1, pair2};
    auto _pointing_device_ids = new etl::array<PointingDeviceId, N>;
    _pointing_device_ids->assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
    return new Internal::TapDance(*pairs, *_pointing_device_ids, move_threshold, tapping_term_ms);
  }

  template <uint8_t N>
  static inline NotNullCommandPtr TD_DPM(const Internal::TapDance::Pair pair0,
                                         const Internal::TapDance::Pair pair1,
                                         const Internal::TapDance::Pair pair2,
                                         const Internal::TapDance::Pair pair3,
                                         const PointingDeviceId (&pointing_device_ids)[N],
                                         uint16_t move_threshold = 0,
                                         uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 4>{pair0, pair1, pair2, pair3};
    auto _pointing_device_ids = new etl::array<PointingDeviceId, N>;
    _pointing_device_ids->assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
    return new Internal::TapDance(*pairs, *_pointing_device_ids, move_threshold, tapping_term_ms);
  }

  template <uint8_t N>
  static inline NotNullCommandPtr TD_DPM(const Internal::TapDance::Pair pair0,
                                         const Internal::TapDance::Pair pair1,
                                         const Internal::TapDance::Pair pair2,
                                         const Internal::TapDance::Pair pair3,
                                         const Internal::TapDance::Pair pair4,
                                         const PointingDeviceId (&pointing_device_ids)[N],
                                         uint16_t move_threshold = 0,
                                         uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 5>{pair0, pair1, pair2, pair3, pair4};
    auto _pointing_device_ids = new etl::array<PointingDeviceId, N>;
    _pointing_device_ids->assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
    return new Internal::TapDance(*pairs, *_pointing_device_ids, move_threshold, tapping_term_ms);
  }

  template <uint8_t N>
  static inline NotNullCommandPtr TD_DPM(const Internal::TapDance::Pair pair0,
                                         const Internal::TapDance::Pair pair1,
                                         const Internal::TapDance::Pair pair2,
                                         const Internal::TapDance::Pair pair3,
                                         const Internal::TapDance::Pair pair4,
                                         const Internal::TapDance::Pair pair5,
                                         const PointingDeviceId (&pointing_device_ids)[N],
                                         uint16_t move_threshold = 0,
                                         uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 6>{pair0, pair1, pair2, pair3, pair4, pair5};
    auto _pointing_device_ids = new etl::array<PointingDeviceId, N>;
    _pointing_device_ids->assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
    return new Internal::TapDance(*pairs, *_pointing_device_ids, move_threshold, tapping_term_ms);
  }

  template <uint8_t N>
  static inline NotNullCommandPtr TD_DPM(const Internal::TapDance::Pair pair0,
                                         const Internal::TapDance::Pair pair1,
                                         const Internal::TapDance::Pair pair2,
                                         const Internal::TapDance::Pair pair3,
                                         const Internal::TapDance::Pair pair4,
                                         const Internal::TapDance::Pair pair5,
                                         const Internal::TapDance::Pair pair6,
                                         const PointingDeviceId (&pointing_device_ids)[N],
                                         uint16_t move_threshold = 0,
                                         uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 7>{pair0, pair1, pair2, pair3, pair4, pair5, pair6};
    auto _pointing_device_ids = new etl::array<PointingDeviceId, N>;
    _pointing_device_ids->assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
    return new Internal::TapDance(*pairs, *_pointing_device_ids, move_threshold, tapping_term_ms);
  }

  template <uint8_t N>
  static inline NotNullCommandPtr TD_DPM(const Internal::TapDance::Pair pair0,
                                         const Internal::TapDance::Pair pair1,
                                         const Internal::TapDance::Pair pair2,
                                         const Internal::TapDance::Pair pair3,
                                         const Internal::TapDance::Pair pair4,
                                         const Internal::TapDance::Pair pair5,
                                         const Internal::TapDance::Pair pair6,
                                         const Internal::TapDance::Pair pair7,
                                         const PointingDeviceId (&pointing_device_ids)[N],
                                         uint16_t move_threshold = 0,
                                         uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 8>{pair0, pair1, pair2, pair3, pair4, pair5, pair6, pair7};
    auto _pointing_device_ids = new etl::array<PointingDeviceId, N>;
    _pointing_device_ids->assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
    return new Internal::TapDance(*pairs, *_pointing_device_ids, move_threshold, tapping_term_ms);
  }

  //------------------------------------------------------------------+
  // HoldTap(HT)
  //------------------------------------------------------------------+
  static NotNullCommandPtr HT(NotNullCommandPtr hold_command,
                              NotNullCommandPtr tap_command,
                              uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 1>{
        Internal::TapDance::Pair{hold_command, tap_command},
    };
    auto pointing_device_ids = new etl::span<PointingDeviceId>;
    return new Internal::TapDance(*pairs, *pointing_device_ids, 0, tapping_term_ms);
  }

  //------------------------------------------------------------------+
  // HoldTapDecideWithPointerMove(HT_DPM)
  //------------------------------------------------------------------+
  template <uint8_t N>
  static NotNullCommandPtr HT_DPM(NotNullCommandPtr hold_command,
                                  NotNullCommandPtr tap_command,
                                  const PointingDeviceId (&pointing_device_ids)[N],
                                  uint16_t move_threshold = 0,
                                  uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
  {
    auto pairs = new etl::array<Internal::TapDance::Pair, 1>{
        Internal::TapDance::Pair{hold_command, tap_command},
    };
    auto _pointing_device_ids = new etl::array<PointingDeviceId, N>;
    _pointing_device_ids->assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
    return new Internal::TapDance(*pairs, *_pointing_device_ids, move_threshold, tapping_term_ms);
  }

  //------------------------------------------------------------------+
  // MouseMove(MS_MOV)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr MS_MOV(int16_t x, int16_t y)
  {
    return new Internal::MouseMove(x, y);
  }

  //------------------------------------------------------------------+
  // MouseScroll(MS_SCR)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr MS_SCR(int8_t scroll, int8_t horiz)
  {
    return new Internal::MouseScroll(scroll, horiz);
  }

  //------------------------------------------------------------------+
  // MouseClick(MS_CLK)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr MS_CLK(MouseButtons buttons)
  {
    return new Internal::MouseClick(buttons);
  }

  //------------------------------------------------------------------+
  // RadialClick(RD_CLK)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr RD_CLK()
  {
    return new Internal::RadialClick();
  }

  //------------------------------------------------------------------+
  // RadialRotate(RD_ROT)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr RD_ROT(int16_t deci_degree)
  {
    deci_degree = etl::clamp<int16_t>(deci_degree, -3600, 3600);
    return new Internal::RadialRotate(deci_degree);
  }

  //------------------------------------------------------------------+
  // ConstantSpeed(CONST_SPD)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr CONST_SPD(NotNullCommandPtr command, uint32_t ms)
  {
    return new Internal::ConstantSpeed(command, ms);
  }

  //------------------------------------------------------------------+
  // StepSpeed(STEP_SPD)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr STEP_SPD(NotNullCommandPtr command, uint32_t ms)
  {
    return new Internal::StepSpeed(command, ms);
  }

  //------------------------------------------------------------------+
  // If(IF)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr IF(bool (*func)(), NotNullCommandPtr true_command, NotNullCommandPtr false_command)
  {
    return new Internal::If(func, true_command, false_command);
  }

  //------------------------------------------------------------------+
  // Multi(MLT)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr MLT(const NotNullCommandPtr command0,
                                      const NotNullCommandPtr command1)
  {
    auto commands = new etl::array<NotNullCommandPtr, 2>{command0, command1};
    return new Internal::Multi(*commands);
  }

  static inline NotNullCommandPtr MLT(const NotNullCommandPtr command0,
                                      const NotNullCommandPtr command1,
                                      const NotNullCommandPtr command2)
  {
    auto commands = new etl::array<NotNullCommandPtr, 3>{command0, command1, command2};
    return new Internal::Multi(*commands);
  }

  static inline NotNullCommandPtr MLT(const NotNullCommandPtr command0,
                                      const NotNullCommandPtr command1,
                                      const NotNullCommandPtr command2,
                                      const NotNullCommandPtr command3)
  {
    auto commands = new etl::array<NotNullCommandPtr, 4>{command0, command1, command2, command3};
    return new Internal::Multi(*commands);
  }

  static inline NotNullCommandPtr MLT(const NotNullCommandPtr command0,
                                      const NotNullCommandPtr command1,
                                      const NotNullCommandPtr command2,
                                      const NotNullCommandPtr command3,
                                      const NotNullCommandPtr command4)
  {
    auto commands = new etl::array<NotNullCommandPtr, 5>{command0, command1, command2, command3, command4};
    return new Internal::Multi(*commands);
  }

  static inline NotNullCommandPtr MLT(const NotNullCommandPtr command0,
                                      const NotNullCommandPtr command1,
                                      const NotNullCommandPtr command2,
                                      const NotNullCommandPtr command3,
                                      const NotNullCommandPtr command4,
                                      const NotNullCommandPtr command5)
  {
    auto commands = new etl::array<NotNullCommandPtr, 6>{command0, command1, command2, command3, command4, command5};
    return new Internal::Multi(*commands);
  }

  static inline NotNullCommandPtr MLT(const NotNullCommandPtr command0,
                                      const NotNullCommandPtr command1,
                                      const NotNullCommandPtr command2,
                                      const NotNullCommandPtr command3,
                                      const NotNullCommandPtr command4,
                                      const NotNullCommandPtr command5,
                                      const NotNullCommandPtr command6)
  {
    auto commands = new etl::array<NotNullCommandPtr, 7>{command0, command1, command2, command3, command4, command5, command6};
    return new Internal::Multi(*commands);
  }

  static inline NotNullCommandPtr MLT(const NotNullCommandPtr command0,
                                      const NotNullCommandPtr command1,
                                      const NotNullCommandPtr command2,
                                      const NotNullCommandPtr command3,
                                      const NotNullCommandPtr command4,
                                      const NotNullCommandPtr command5,
                                      const NotNullCommandPtr command6,
                                      const NotNullCommandPtr command7)
  {
    auto commands = new etl::array<NotNullCommandPtr, 8>{command0, command1, command2, command3, command4, command5, command6, command7};
    return new Internal::Multi(*commands);
  }

  //------------------------------------------------------------------+
  // Toggle(TGL)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr TGL(NotNullCommandPtr command)
  {
    return new Internal::Toggle(command);
  }

  //------------------------------------------------------------------+
  // Repeat(RPT)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr RPT(NotNullCommandPtr command, uint32_t delay_ms, uint32_t interval_ms)
  {
    return new Internal::Repeat(command, delay_ms, interval_ms);
  }

  //------------------------------------------------------------------+
  // Cycle(CYC)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr CYC(const NotNullCommandPtr command0,
                                      const NotNullCommandPtr command1)
  {
    auto commands = new etl::array<NotNullCommandPtr, 2>{command0, command1};
    return new Internal::Cycle(*commands);
  }

  static inline NotNullCommandPtr CYC(const NotNullCommandPtr command0,
                                      const NotNullCommandPtr command1,
                                      const NotNullCommandPtr command2)
  {
    auto commands = new etl::array<NotNullCommandPtr, 3>{command0, command1, command2};
    return new Internal::Cycle(*commands);
  }

  static inline NotNullCommandPtr CYC(const NotNullCommandPtr command0,
                                      const NotNullCommandPtr command1,
                                      const NotNullCommandPtr command2,
                                      const NotNullCommandPtr command3)
  {
    auto commands = new etl::array<NotNullCommandPtr, 4>{command0, command1, command2, command3};
    return new Internal::Cycle(*commands);
  }

  static inline NotNullCommandPtr CYC(const NotNullCommandPtr command0,
                                      const NotNullCommandPtr command1,
                                      const NotNullCommandPtr command2,
                                      const NotNullCommandPtr command3,
                                      const NotNullCommandPtr command4)
  {
    auto commands = new etl::array<NotNullCommandPtr, 5>{command0, command1, command2, command3, command4};
    return new Internal::Cycle(*commands);
  }

  static inline NotNullCommandPtr CYC(const NotNullCommandPtr command0,
                                      const NotNullCommandPtr command1,
                                      const NotNullCommandPtr command2,
                                      const NotNullCommandPtr command3,
                                      const NotNullCommandPtr command4,
                                      const NotNullCommandPtr command5)
  {
    auto commands = new etl::array<NotNullCommandPtr, 6>{command0, command1, command2, command3, command4, command5};
    return new Internal::Cycle(*commands);
  }

  static inline NotNullCommandPtr CYC(const NotNullCommandPtr command0,
                                      const NotNullCommandPtr command1,
                                      const NotNullCommandPtr command2,
                                      const NotNullCommandPtr command3,
                                      const NotNullCommandPtr command4,
                                      const NotNullCommandPtr command5,
                                      const NotNullCommandPtr command6)
  {
    auto commands = new etl::array<NotNullCommandPtr, 7>{command0, command1, command2, command3, command4, command5, command6};
    return new Internal::Cycle(*commands);
  }

  static inline NotNullCommandPtr CYC(const NotNullCommandPtr command0,
                                      const NotNullCommandPtr command1,
                                      const NotNullCommandPtr command2,
                                      const NotNullCommandPtr command3,
                                      const NotNullCommandPtr command4,
                                      const NotNullCommandPtr command5,
                                      const NotNullCommandPtr command6,
                                      const NotNullCommandPtr command7)
  {
    auto commands = new etl::array<NotNullCommandPtr, 8>{command0, command1, command2, command3, command4, command5, command6, command7};
    return new Internal::Cycle(*commands);
  }

  //------------------------------------------------------------------+
  // CyclePhaseShift(CYC_PS)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr CYC_PS(const NotNullCommandPtr command0,
                                         const NotNullCommandPtr command1)
  {
    auto commands = new etl::array<NotNullCommandPtr, 2>{command0, command1};
    return (new Internal::CyclePhaseShift(*commands));
  }

  static inline NotNullCommandPtr CYC_PS(const NotNullCommandPtr command0,
                                         const NotNullCommandPtr command1,
                                         const NotNullCommandPtr command2)
  {
    auto commands = new etl::array<NotNullCommandPtr, 3>{command0, command1, command2};
    return (new Internal::CyclePhaseShift(*commands));
  }

  static inline NotNullCommandPtr CYC_PS(const NotNullCommandPtr command0,
                                         const NotNullCommandPtr command1,
                                         const NotNullCommandPtr command2,
                                         const NotNullCommandPtr command3)
  {
    auto commands = new etl::array<NotNullCommandPtr, 4>{command0, command1, command2, command3};
    return (new Internal::CyclePhaseShift(*commands));
  }

  static inline NotNullCommandPtr CYC_PS(const NotNullCommandPtr command0,
                                         const NotNullCommandPtr command1,
                                         const NotNullCommandPtr command2,
                                         const NotNullCommandPtr command3,
                                         const NotNullCommandPtr command4)
  {
    auto commands = new etl::array<NotNullCommandPtr, 5>{command0, command1, command2, command3, command4};
    return (new Internal::CyclePhaseShift(*commands));
  }

  static inline NotNullCommandPtr CYC_PS(const NotNullCommandPtr command0,
                                         const NotNullCommandPtr command1,
                                         const NotNullCommandPtr command2,
                                         const NotNullCommandPtr command3,
                                         const NotNullCommandPtr command4,
                                         const NotNullCommandPtr command5)
  {
    auto commands = new etl::array<NotNullCommandPtr, 6>{command0, command1, command2, command3, command4, command5};
    return (new Internal::CyclePhaseShift(*commands));
  }

  static inline NotNullCommandPtr CYC_PS(const NotNullCommandPtr command0,
                                         const NotNullCommandPtr command1,
                                         const NotNullCommandPtr command2,
                                         const NotNullCommandPtr command3,
                                         const NotNullCommandPtr command4,
                                         const NotNullCommandPtr command5,
                                         const NotNullCommandPtr command6)
  {
    auto commands = new etl::array<NotNullCommandPtr, 7>{command0, command1, command2, command3, command4, command5, command6};
    return (new Internal::CyclePhaseShift(*commands));
  }

  static inline NotNullCommandPtr CYC_PS(const NotNullCommandPtr command0,
                                         const NotNullCommandPtr command1,
                                         const NotNullCommandPtr command2,
                                         const NotNullCommandPtr command3,
                                         const NotNullCommandPtr command4,
                                         const NotNullCommandPtr command5,
                                         const NotNullCommandPtr command6,
                                         const NotNullCommandPtr command7)
  {
    auto commands = new etl::array<NotNullCommandPtr, 8>{command0, command1, command2, command3, command4, command5, command6, command7};
    return (new Internal::CyclePhaseShift(*commands));
  }

  //------------------------------------------------------------------+
  // NoOperation(NOP)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr NOP() { return (new Internal::NoOperation()); }

  //------------------------------------------------------------------+
  // Shift(SFT)
  //------------------------------------------------------------------+
  static inline NotNullCommandPtr SFT(Internal::ShiftId shift_id)
  {
    auto shift_id_links = new etl::array<Internal::ShiftIdLink, 1>{
        Internal::Shift::IdToLink(shift_id),
    };
    return (new Internal::Shift(*shift_id_links));
  }

  static inline NotNullCommandPtr SFT(Internal::ShiftId shift_id0,
                                      Internal::ShiftId shift_id1)
  {
    auto shift_id_links = new etl::array<Internal::ShiftIdLink, 2>{
        Internal::Shift::IdToLink(shift_id0),
        Internal::Shift::IdToLink(shift_id1),
    };
    return (new Internal::Shift(*shift_id_links));
  }

  static inline NotNullCommandPtr SFT(Internal::ShiftId shift_id0,
                                      Internal::ShiftId shift_id1,
                                      Internal::ShiftId shift_id2)
  {
    auto shift_id_links = new etl::array<Internal::ShiftIdLink, 3>{
        Internal::Shift::IdToLink(shift_id0),
        Internal::Shift::IdToLink(shift_id1),
        Internal::Shift::IdToLink(shift_id2),
    };
    return (new Internal::Shift(*shift_id_links));
  }

  static inline NotNullCommandPtr SFT(Internal::ShiftId shift_id0,
                                      Internal::ShiftId shift_id1,
                                      Internal::ShiftId shift_id2,
                                      Internal::ShiftId shift_id3)
  {
    auto shift_id_links = new etl::array<Internal::ShiftIdLink, 4>{
        Internal::Shift::IdToLink(shift_id0),
        Internal::Shift::IdToLink(shift_id1),
        Internal::Shift::IdToLink(shift_id2),
        Internal::Shift::IdToLink(shift_id3),
    };
    return (new Internal::Shift(*shift_id_links));
  }

  static inline NotNullCommandPtr SFT(Internal::ShiftId shift_id0,
                                      Internal::ShiftId shift_id1,
                                      Internal::ShiftId shift_id2,
                                      Internal::ShiftId shift_id3,
                                      Internal::ShiftId shift_id4)
  {
    auto shift_id_links = new etl::array<Internal::ShiftIdLink, 5>{
        Internal::Shift::IdToLink(shift_id0),
        Internal::Shift::IdToLink(shift_id1),
        Internal::Shift::IdToLink(shift_id2),
        Internal::Shift::IdToLink(shift_id3),
        Internal::Shift::IdToLink(shift_id4),
    };
    return (new Internal::Shift(*shift_id_links));
  }

  static inline NotNullCommandPtr SFT(Internal::ShiftId shift_id0,
                                      Internal::ShiftId shift_id1,
                                      Internal::ShiftId shift_id2,
                                      Internal::ShiftId shift_id3,
                                      Internal::ShiftId shift_id4,
                                      Internal::ShiftId shift_id5)
  {
    auto shift_id_links = new etl::array<Internal::ShiftIdLink, 6>{
        Internal::Shift::IdToLink(shift_id0),
        Internal::Shift::IdToLink(shift_id1),
        Internal::Shift::IdToLink(shift_id2),
        Internal::Shift::IdToLink(shift_id3),
        Internal::Shift::IdToLink(shift_id4),
        Internal::Shift::IdToLink(shift_id5),
    };
    return (new Internal::Shift(*shift_id_links));
  }

  static inline NotNullCommandPtr SFT(Internal::ShiftId shift_id0,
                                      Internal::ShiftId shift_id1,
                                      Internal::ShiftId shift_id2,
                                      Internal::ShiftId shift_id3,
                                      Internal::ShiftId shift_id4,
                                      Internal::ShiftId shift_id5,
                                      Internal::ShiftId shift_id6)
  {
    auto shift_id_links = new etl::array<Internal::ShiftIdLink, 7>{
        Internal::Shift::IdToLink(shift_id0),
        Internal::Shift::IdToLink(shift_id1),
        Internal::Shift::IdToLink(shift_id2),
        Internal::Shift::IdToLink(shift_id3),
        Internal::Shift::IdToLink(shift_id4),
        Internal::Shift::IdToLink(shift_id5),
        Internal::Shift::IdToLink(shift_id6),
    };
    return (new Internal::Shift(*shift_id_links));
  }

  static inline NotNullCommandPtr SFT(Internal::ShiftId shift_id0,
                                      Internal::ShiftId shift_id1,
                                      Internal::ShiftId shift_id2,
                                      Internal::ShiftId shift_id3,
                                      Internal::ShiftId shift_id4,
                                      Internal::ShiftId shift_id5,
                                      Internal::ShiftId shift_id6,
                                      Internal::ShiftId shift_id7)
  {
    auto shift_id_links = new etl::array<Internal::ShiftIdLink, 8>{
        Internal::Shift::IdToLink(shift_id0),
        Internal::Shift::IdToLink(shift_id1),
        Internal::Shift::IdToLink(shift_id2),
        Internal::Shift::IdToLink(shift_id3),
        Internal::Shift::IdToLink(shift_id4),
        Internal::Shift::IdToLink(shift_id5),
        Internal::Shift::IdToLink(shift_id6),
        Internal::Shift::IdToLink(shift_id7),
    };
    return (new Internal::Shift(*shift_id_links));
  }

  //------------------------------------------------------------------+
  // KEYMAP
  //------------------------------------------------------------------+
  template <size_t N>
  static inline etl::span<Key> KEYMAP(const Key (&keymap)[N])
  {
    auto result = new etl::vector<Key, N>;
    result->assign(std::begin(keymap), std::end(keymap));
    return *result;
  }

  //------------------------------------------------------------------+
  // nullptr alias (_ * 7)
  //------------------------------------------------------------------+

#define _______ (nullptr)

} // namespace hidpg
