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
#include "consthash/cityhash64.hxx"
#include "consthash/crc64.hxx"
#include "etl/array.h"
#include "etl/vector.h"
#include <new>

namespace hidpg
{

  namespace Internal
  {
    //------------------------------------------------------------------+
    // KeyCode(KC)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_KeyCode(CharacterKey character_key)
    {
      static uint8_t buf[sizeof(CharacterKeyCommand)];
      return new (buf) CharacterKeyCommand(character_key);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_KeyCode(Modifiers modifiers)
    {
      static uint8_t buf[sizeof(ModifiersCommand)];
      return new (buf) ModifiersCommand(modifiers);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_KeyCode(CombinationKeys combination_keys)
    {
      static uint8_t buf[sizeof(CombinationKeysCommand)];
      return new (buf) CombinationKeysCommand(combination_keys);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_KeyCode(ConsumerControlCode consumer_control_code)
    {
      static uint8_t buf[sizeof(ConsumerControl)];
      return new (buf) ConsumerControl(consumer_control_code);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_KeyCode(SystemControlCode system_control_code)
    {
      static uint8_t buf[sizeof(SystemControl)];
      return new (buf) SystemControl(system_control_code);
    }

#define KC(key_code) (Internal::new_KeyCode<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(key_code))

    //------------------------------------------------------------------+
    // Layering(LY)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Layering(LayerClass &layer, const CommandPtr (&commands)[HID_ENGINE_LAYER_SIZE])
    {
      static etl::array<CommandPtr, HID_ENGINE_LAYER_SIZE> _commands;
      _commands.assign(std::begin(commands), std::end(commands));
      static uint8_t buf[sizeof(Layering)];

      return new (buf) Layering(layer, _commands);
    }

#define LY(...) (Internal::new_Layering<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer, __VA_ARGS__))
#define LY1(...) (Internal::new_Layering<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer1, __VA_ARGS__))
#define LY2(...) (Internal::new_Layering<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer2, __VA_ARGS__))

    //------------------------------------------------------------------+
    // ToggleLayer(TL)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_ToggleLayer(LayerClass &layer, uint8_t layer_number)
    {
      static uint8_t buf[sizeof(ToggleLayer)];
      return new (buf) ToggleLayer(layer, layer_number);
    }

#define TL(layer_number) (Internal::new_ToggleLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer, layer_number))
#define TL1(layer_number) (Internal::new_ToggleLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer1, layer_number))
#define TL2(layer_number) (Internal::new_ToggleLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer2, layer_number))

    //------------------------------------------------------------------+
    // SwitchLayer(SL)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_SwitchLayer(LayerClass &layer, uint8_t layer_number)
    {
      static uint8_t buf[sizeof(SwitchLayer)];
      return new (buf) SwitchLayer(layer, layer_number);
    }

#define SL(layer_number) (Internal::new_SwitchLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer, layer_number))
#define SL1(layer_number) (Internal::new_SwitchLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer1, layer_number))
#define SL2(layer_number) (Internal::new_SwitchLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer2, layer_number))

    //------------------------------------------------------------------+
    // UpDefaultLayer(UPL)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_UpDefaultLayer(LayerClass &layer, uint8_t i)
    {
      static uint8_t buf[sizeof(UpDefaultLayer)];
      return new (buf) UpDefaultLayer(layer, i);
    }

#define UPL(i) (Internal::new_UpDefaultLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer, i))
#define UPL1(i) (Internal::new_UpDefaultLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer1, i))
#define UPL2(i) (Internal::new_UpDefaultLayer<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(Layer2, i))

    //------------------------------------------------------------------+
    // Tap(TAP)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Tap(NotNullCommandPtr command, uint8_t n_times = 1, uint16_t tap_speed_ms = HID_ENGINE_TAP_SPEED_MS)
    {
      static uint8_t buf[sizeof(Tap)];
      return new (buf) Tap(command, n_times, tap_speed_ms);
    }

#define TAP(...) (Internal::new_Tap<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

    //------------------------------------------------------------------+
    // TapWhenReleased(TAP_R)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_TapWhenReleased(NotNullCommandPtr command, uint8_t n_times = 1, uint16_t tap_speed_ms = HID_ENGINE_TAP_SPEED_MS)
    {
      static uint8_t buf[sizeof(TapWhenReleased)];
      return new (buf) TapWhenReleased(command, n_times, tap_speed_ms);
    }

#define TAP_R(...) (Internal::new_TapWhenReleased<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

    //------------------------------------------------------------------+
    // TapDance(TD)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_TapDance(const TapDance::Pair pair0,
                                   const TapDance::Pair pair1,
                                   uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 2> pairs{pair0, pair1};
      static etl::span<PointingDeviceId> pointing_device_ids;
      static uint8_t buf[sizeof(TapDance)];
      return new (buf) TapDance(pairs, pointing_device_ids, 0, tapping_term_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_TapDance(const TapDance::Pair pair0,
                                   const TapDance::Pair pair1,
                                   const TapDance::Pair pair2,
                                   uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 3> pairs{pair0, pair1, pair2};
      static etl::span<PointingDeviceId> pointing_device_ids;
      static uint8_t buf[sizeof(TapDance)];
      return new (buf) TapDance(pairs, pointing_device_ids, 0, tapping_term_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_TapDance(const TapDance::Pair pair0,
                                   const TapDance::Pair pair1,
                                   const TapDance::Pair pair2,
                                   const TapDance::Pair pair3,
                                   uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 4> pairs{pair0, pair1, pair2, pair3};
      static etl::span<PointingDeviceId> pointing_device_ids;
      static uint8_t buf[sizeof(TapDance)];
      return new (buf) TapDance(pairs, pointing_device_ids, 0, tapping_term_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_TapDance(const TapDance::Pair pair0,
                                   const TapDance::Pair pair1,
                                   const TapDance::Pair pair2,
                                   const TapDance::Pair pair3,
                                   const TapDance::Pair pair4,
                                   uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 5> pairs{pair0, pair1, pair2, pair3, pair4};
      static etl::span<PointingDeviceId> pointing_device_ids;
      static uint8_t buf[sizeof(TapDance)];
      return new (buf) TapDance(pairs, pointing_device_ids, 0, tapping_term_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_TapDance(const TapDance::Pair pair0,
                                   const TapDance::Pair pair1,
                                   const TapDance::Pair pair2,
                                   const TapDance::Pair pair3,
                                   const TapDance::Pair pair4,
                                   const TapDance::Pair pair5,
                                   uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 6> pairs{pair0, pair1, pair2, pair3, pair4, pair5};
      static etl::span<PointingDeviceId> pointing_device_ids;
      static uint8_t buf[sizeof(TapDance)];
      return new (buf) TapDance(pairs, pointing_device_ids, 0, tapping_term_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_TapDance(const TapDance::Pair pair0,
                                   const TapDance::Pair pair1,
                                   const TapDance::Pair pair2,
                                   const TapDance::Pair pair3,
                                   const TapDance::Pair pair4,
                                   const TapDance::Pair pair5,
                                   const TapDance::Pair pair6,
                                   uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 7> pairs{pair0, pair1, pair2, pair3, pair4, pair5, pair6};
      static etl::span<PointingDeviceId> pointing_device_ids;
      static uint8_t buf[sizeof(TapDance)];
      return new (buf) TapDance(pairs, pointing_device_ids, 0, tapping_term_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_TapDance(const TapDance::Pair pair0,
                                   const TapDance::Pair pair1,
                                   const TapDance::Pair pair2,
                                   const TapDance::Pair pair3,
                                   const TapDance::Pair pair4,
                                   const TapDance::Pair pair5,
                                   const TapDance::Pair pair6,
                                   const TapDance::Pair pair7,
                                   uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 8> pairs{pair0, pair1, pair2, pair3, pair4, pair5, pair6, pair7};
      static etl::span<PointingDeviceId> pointing_device_ids;
      static uint8_t buf[sizeof(TapDance)];
      return new (buf) TapDance(pairs, pointing_device_ids, 0, tapping_term_ms);
    }

#define TD(...) (Internal::new_TapDance<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

    //------------------------------------------------------------------+
    // TapDanceDecideWithPointerMove(TD_DPM)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    NotNullCommandPtr new_TapDanceDecideWithPointerMove(const TapDance::Pair pair0,
                                                        const TapDance::Pair pair1,
                                                        const PointingDeviceId (&pointing_device_ids)[N],
                                                        uint16_t move_threshold = 0,
                                                        uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 2> pairs{pair0, pair1};
      static etl::array<PointingDeviceId, N> _pointing_device_ids;
      _pointing_device_ids.assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
      static uint8_t buf[sizeof(TapDance)];
      return new (buf) TapDance(pairs, _pointing_device_ids, move_threshold, tapping_term_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    NotNullCommandPtr new_TapDanceDecideWithPointerMove(const TapDance::Pair pair0,
                                                        const TapDance::Pair pair1,
                                                        const TapDance::Pair pair2,
                                                        const PointingDeviceId (&pointing_device_ids)[N],
                                                        uint16_t move_threshold = 0,
                                                        uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 3> pairs{pair0, pair1, pair2};
      static etl::array<PointingDeviceId, N> _pointing_device_ids;
      _pointing_device_ids.assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
      static uint8_t buf[sizeof(TapDance)];
      return new (buf) TapDance(pairs, _pointing_device_ids, move_threshold, tapping_term_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    NotNullCommandPtr new_TapDanceDecideWithPointerMove(const TapDance::Pair pair0,
                                                        const TapDance::Pair pair1,
                                                        const TapDance::Pair pair2,
                                                        const TapDance::Pair pair3,
                                                        const PointingDeviceId (&pointing_device_ids)[N],
                                                        uint16_t move_threshold = 0,
                                                        uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 4> pairs{pair0, pair1, pair2, pair3};
      static etl::array<PointingDeviceId, N> _pointing_device_ids;
      _pointing_device_ids.assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
      static uint8_t buf[sizeof(TapDance)];
      return new (buf) TapDance(pairs, _pointing_device_ids, move_threshold, tapping_term_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    NotNullCommandPtr new_TapDanceDecideWithPointerMove(const TapDance::Pair pair0,
                                                        const TapDance::Pair pair1,
                                                        const TapDance::Pair pair2,
                                                        const TapDance::Pair pair3,
                                                        const TapDance::Pair pair4,
                                                        const PointingDeviceId (&pointing_device_ids)[N],
                                                        uint16_t move_threshold = 0,
                                                        uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 5> pairs{pair0, pair1, pair2, pair3, pair4};
      static etl::array<PointingDeviceId, N> _pointing_device_ids;
      _pointing_device_ids.assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
      static uint8_t buf[sizeof(TapDance)];
      return new (buf) TapDance(pairs, _pointing_device_ids, move_threshold, tapping_term_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    NotNullCommandPtr new_TapDanceDecideWithPointerMove(const TapDance::Pair pair0,
                                                        const TapDance::Pair pair1,
                                                        const TapDance::Pair pair2,
                                                        const TapDance::Pair pair3,
                                                        const TapDance::Pair pair4,
                                                        const TapDance::Pair pair5,
                                                        const PointingDeviceId (&pointing_device_ids)[N],
                                                        uint16_t move_threshold = 0,
                                                        uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 6> pairs{pair0, pair1, pair2, pair3, pair4, pair5};
      static etl::array<PointingDeviceId, N> _pointing_device_ids;
      _pointing_device_ids.assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
      static uint8_t buf[sizeof(TapDance)];
      return new (buf) TapDance(pairs, _pointing_device_ids, move_threshold, tapping_term_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    NotNullCommandPtr new_TapDanceDecideWithPointerMove(const TapDance::Pair pair0,
                                                        const TapDance::Pair pair1,
                                                        const TapDance::Pair pair2,
                                                        const TapDance::Pair pair3,
                                                        const TapDance::Pair pair4,
                                                        const TapDance::Pair pair5,
                                                        const TapDance::Pair pair6,
                                                        const PointingDeviceId (&pointing_device_ids)[N],
                                                        uint16_t move_threshold = 0,
                                                        uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 7> pairs{pair0, pair1, pair2, pair3, pair4, pair5, pair6};
      static etl::array<PointingDeviceId, N> _pointing_device_ids;
      _pointing_device_ids.assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
      static uint8_t buf[sizeof(TapDance)];
      return new (buf) TapDance(pairs, _pointing_device_ids, move_threshold, tapping_term_ms);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    NotNullCommandPtr new_TapDanceDecideWithPointerMove(const TapDance::Pair pair0,
                                                        const TapDance::Pair pair1,
                                                        const TapDance::Pair pair2,
                                                        const TapDance::Pair pair3,
                                                        const TapDance::Pair pair4,
                                                        const TapDance::Pair pair5,
                                                        const TapDance::Pair pair6,
                                                        const TapDance::Pair pair7,
                                                        const PointingDeviceId (&pointing_device_ids)[N],
                                                        uint16_t move_threshold = 0,
                                                        uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 8> pairs{pair0, pair1, pair2, pair3, pair4, pair5, pair6, pair7};
      static etl::array<PointingDeviceId, N> _pointing_device_ids;
      _pointing_device_ids.assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
      static uint8_t buf[sizeof(TapDance)];
      return new (buf) TapDance(pairs, _pointing_device_ids, move_threshold, tapping_term_ms);
    }

#define TD_DPM(...) (Internal::new_TapDanceDecideWithPointerMove<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

    //------------------------------------------------------------------+
    // HoldTap(HT)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_HoldTap(NotNullCommandPtr hold_command,
                                  NotNullCommandPtr tap_command,
                                  uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 1> pairs{
          TapDance::Pair{hold_command, tap_command},
      };
      static etl::span<PointingDeviceId> pointing_device_ids;
      static uint8_t buf[sizeof(TapDance)];
      return new (buf) TapDance(pairs, pointing_device_ids, 0, tapping_term_ms);
    }

#define HT(...) (Internal::new_HoldTap<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

    //------------------------------------------------------------------+
    // HoldTapDecideWithPointerMove(HT_DPM)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    NotNullCommandPtr new_HoldTapDecideWithPointerMove(NotNullCommandPtr hold_command,
                                                       NotNullCommandPtr tap_command,
                                                       const PointingDeviceId (&pointing_device_ids)[N],
                                                       uint16_t move_threshold = 0,
                                                       uint32_t tapping_term_ms = HID_ENGINE_TAPPING_TERM_MS)
    {
      static etl::array<Internal::TapDance::Pair, 1> pairs{
          TapDance::Pair{hold_command, tap_command},
      };
      static etl::array<PointingDeviceId, N> _pointing_device_ids;
      _pointing_device_ids.assign(std::begin(pointing_device_ids), std::end(pointing_device_ids));
      static uint8_t buf[sizeof(TapDance)];

      return new (buf) TapDance(pairs, _pointing_device_ids, move_threshold, tapping_term_ms);
    }

#define HT_DPM(...) (Internal::new_HoldTapDecideWithPointerMove<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

    //------------------------------------------------------------------+
    // MouseMove(MS_MOV)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_MouseMove(int16_t x, int16_t y)
    {
      static uint8_t buf[sizeof(MouseMove)];
      return new (buf) MouseMove(x, y);
    }

#define MS_MOV(x, y) (Internal::new_MouseMove<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(x, y))

    //------------------------------------------------------------------+
    // MouseScroll(MS_SCR)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_MouseScroll(int8_t scroll, int8_t horiz)
    {
      static uint8_t buf[sizeof(MouseScroll)];
      return new (buf) MouseScroll(scroll, horiz);
    }

#define MS_SCR(scroll, horiz) (Internal::new_MouseScroll<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(scroll, horiz))

    //------------------------------------------------------------------+
    // MouseClick(MS_CLK)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_MouseClick(MouseButtons buttons)
    {
      static uint8_t buf[sizeof(MouseClick)];
      return new (buf) MouseClick(buttons);
    }

#define MS_CLK(buttons) (Internal::new_MouseClick<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(buttons))

    //------------------------------------------------------------------+
    // RadialClick(RD_CLK)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_RadialClick()
    {
      static uint8_t buf[sizeof(RadialClick)];
      return new (buf) RadialClick();
    }

#define RD_CLK() (Internal::new_RadialClick<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>())

    //------------------------------------------------------------------+
    // RadialRotate(RD_ROT)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_RadialRotate(int16_t deci_degree)
    {
      deci_degree = etl::clamp<int16_t>(deci_degree, -3600, 3600);

      static uint8_t buf[sizeof(RadialRotate)];
      return new (buf) RadialRotate(deci_degree);
    }

#define RD_ROT(deci_degree) (Internal::new_RadialRotate<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(deci_degree))

    //------------------------------------------------------------------+
    // ConstantSpeed(CONST_SPD)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_ConstantSpeed(NotNullCommandPtr command, uint32_t ms)
    {
      static uint8_t buf[sizeof(ConstantSpeed)];
      return new (buf) ConstantSpeed(command, ms);
    }

#define CONST_SPD(command, ms) (Internal::new_ConstantSpeed<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(command, ms))

    //------------------------------------------------------------------+
    // StepSpeed(STEP_SPD)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_StepSpeed(NotNullCommandPtr command, uint32_t ms)
    {
      static uint8_t buf[sizeof(StepSpeed)];
      return new (buf) StepSpeed(command, ms);
    }

#define STEP_SPD(command, ms) (Internal::new_StepSpeed<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(command, ms))

    //------------------------------------------------------------------+
    // If(IF)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_If(bool (*func)(), NotNullCommandPtr true_command, NotNullCommandPtr false_command)
    {
      static uint8_t buf[sizeof(If)];
      return new (buf) If(func, true_command, false_command);
    }

#define IF(func, true_command, false_command) (Internal::new_If<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(func, true_command, false_command))

    //------------------------------------------------------------------+
    // Multi(MLT)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Multi(const NotNullCommandPtr command0,
                                const NotNullCommandPtr command1)
    {
      static etl::array<NotNullCommandPtr, 2> commands{command0, command1};
      static uint8_t buf[sizeof(Multi)];
      return new (buf) Multi(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Multi(const NotNullCommandPtr command0,
                                const NotNullCommandPtr command1,
                                const NotNullCommandPtr command2)
    {
      static etl::array<NotNullCommandPtr, 3> commands{command0, command1, command2};
      static uint8_t buf[sizeof(Multi)];
      return new (buf) Multi(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Multi(const NotNullCommandPtr command0,
                                const NotNullCommandPtr command1,
                                const NotNullCommandPtr command2,
                                const NotNullCommandPtr command3)
    {
      static etl::array<NotNullCommandPtr, 4> commands{command0, command1, command2, command3};
      static uint8_t buf[sizeof(Multi)];
      return new (buf) Multi(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Multi(const NotNullCommandPtr command0,
                                const NotNullCommandPtr command1,
                                const NotNullCommandPtr command2,
                                const NotNullCommandPtr command3,
                                const NotNullCommandPtr command4)
    {
      static etl::array<NotNullCommandPtr, 5> commands{command0, command1, command2, command3, command4};
      static uint8_t buf[sizeof(Multi)];
      return new (buf) Multi(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Multi(const NotNullCommandPtr command0,
                                const NotNullCommandPtr command1,
                                const NotNullCommandPtr command2,
                                const NotNullCommandPtr command3,
                                const NotNullCommandPtr command4,
                                const NotNullCommandPtr command5)
    {
      static etl::array<NotNullCommandPtr, 6> commands{command0, command1, command2, command3, command4, command5};
      static uint8_t buf[sizeof(Multi)];
      return new (buf) Multi(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Multi(const NotNullCommandPtr command0,
                                const NotNullCommandPtr command1,
                                const NotNullCommandPtr command2,
                                const NotNullCommandPtr command3,
                                const NotNullCommandPtr command4,
                                const NotNullCommandPtr command5,
                                const NotNullCommandPtr command6)
    {
      static etl::array<NotNullCommandPtr, 7> commands{command0, command1, command2, command3, command4, command5, command6};
      static uint8_t buf[sizeof(Multi)];
      return new (buf) Multi(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Multi(const NotNullCommandPtr command0,
                                const NotNullCommandPtr command1,
                                const NotNullCommandPtr command2,
                                const NotNullCommandPtr command3,
                                const NotNullCommandPtr command4,
                                const NotNullCommandPtr command5,
                                const NotNullCommandPtr command6,
                                const NotNullCommandPtr command7)
    {
      static etl::array<NotNullCommandPtr, 8> commands{command0, command1, command2, command3, command4, command5, command6, command7};
      static uint8_t buf[sizeof(Multi)];
      return new (buf) Multi(commands);
    }

#define MLT(...) (Internal::new_Multi<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

    //------------------------------------------------------------------+
    // Toggle(TGL)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Toggle(NotNullCommandPtr command)
    {
      static uint8_t buf[sizeof(Toggle)];
      return new (buf) Toggle(command);
    }

#define TGL(command) (Internal::new_Toggle<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(command))

    //------------------------------------------------------------------+
    // Repeat(RPT)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Repeat(NotNullCommandPtr command, uint32_t delay_ms, uint32_t interval_ms)
    {
      static uint8_t buf[sizeof(Repeat)];
      return new (buf) Repeat(command, delay_ms, interval_ms);
    }

#define RPT(command, delay_ms, interval_ms) (Internal::new_Repeat<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(command, delay_ms, interval_ms))

    //------------------------------------------------------------------+
    // Cycle(CYC)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Cycle(const NotNullCommandPtr command0,
                                const NotNullCommandPtr command1)
    {
      static etl::array<NotNullCommandPtr, 2> commands{command0, command1};
      static uint8_t buf[sizeof(Cycle)];

      return new (buf) Cycle(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Cycle(const NotNullCommandPtr command0,
                                const NotNullCommandPtr command1,
                                const NotNullCommandPtr command2)
    {
      static etl::array<NotNullCommandPtr, 3> commands{command0, command1, command2};
      static uint8_t buf[sizeof(Cycle)];

      return new (buf) Cycle(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Cycle(const NotNullCommandPtr command0,
                                const NotNullCommandPtr command1,
                                const NotNullCommandPtr command2,
                                const NotNullCommandPtr command3)
    {
      static etl::array<NotNullCommandPtr, 4> commands{command0, command1, command2, command3};
      static uint8_t buf[sizeof(Cycle)];

      return new (buf) Cycle(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Cycle(const NotNullCommandPtr command0,
                                const NotNullCommandPtr command1,
                                const NotNullCommandPtr command2,
                                const NotNullCommandPtr command3,
                                const NotNullCommandPtr command4)
    {
      static etl::array<NotNullCommandPtr, 5> commands{command0, command1, command2, command3, command4};
      static uint8_t buf[sizeof(Cycle)];

      return new (buf) Cycle(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Cycle(const NotNullCommandPtr command0,
                                const NotNullCommandPtr command1,
                                const NotNullCommandPtr command2,
                                const NotNullCommandPtr command3,
                                const NotNullCommandPtr command4,
                                const NotNullCommandPtr command5)
    {
      static etl::array<NotNullCommandPtr, 6> commands{command0, command1, command2, command3, command4, command5};
      static uint8_t buf[sizeof(Cycle)];

      return new (buf) Cycle(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Cycle(const NotNullCommandPtr command0,
                                const NotNullCommandPtr command1,
                                const NotNullCommandPtr command2,
                                const NotNullCommandPtr command3,
                                const NotNullCommandPtr command4,
                                const NotNullCommandPtr command5,
                                const NotNullCommandPtr command6)
    {
      static etl::array<NotNullCommandPtr, 7> commands{command0, command1, command2, command3, command4, command5, command6};
      static uint8_t buf[sizeof(Cycle)];

      return new (buf) Cycle(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Cycle(const NotNullCommandPtr command0,
                                const NotNullCommandPtr command1,
                                const NotNullCommandPtr command2,
                                const NotNullCommandPtr command3,
                                const NotNullCommandPtr command4,
                                const NotNullCommandPtr command5,
                                const NotNullCommandPtr command6,
                                const NotNullCommandPtr command7)
    {
      static etl::array<NotNullCommandPtr, 8> commands{command0, command1, command2, command3, command4, command5, command6, command7};
      static uint8_t buf[sizeof(Cycle)];

      return new (buf) Cycle(commands);
    }

#define CYC(...) (Internal::new_Cycle<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

    //------------------------------------------------------------------+
    // CyclePhaseShift(CYC_PS)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_CyclePhaseShift(const NotNullCommandPtr command0,
                                          const NotNullCommandPtr command1)
    {
      static etl::array<NotNullCommandPtr, 2> commands{command0, command1};
      static uint8_t buf[sizeof(CyclePhaseShift)];

      return new (buf) CyclePhaseShift(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_CyclePhaseShift(const NotNullCommandPtr command0,
                                          const NotNullCommandPtr command1,
                                          const NotNullCommandPtr command2)
    {
      static etl::array<NotNullCommandPtr, 3> commands{command0, command1, command2};
      static uint8_t buf[sizeof(CyclePhaseShift)];

      return new (buf) CyclePhaseShift(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_CyclePhaseShift(const NotNullCommandPtr command0,
                                          const NotNullCommandPtr command1,
                                          const NotNullCommandPtr command2,
                                          const NotNullCommandPtr command3)
    {
      static etl::array<NotNullCommandPtr, 4> commands{command0, command1, command2, command3};
      static uint8_t buf[sizeof(CyclePhaseShift)];

      return new (buf) CyclePhaseShift(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_CyclePhaseShift(const NotNullCommandPtr command0,
                                          const NotNullCommandPtr command1,
                                          const NotNullCommandPtr command2,
                                          const NotNullCommandPtr command3,
                                          const NotNullCommandPtr command4)
    {
      static etl::array<NotNullCommandPtr, 5> commands{command0, command1, command2, command3, command4};
      static uint8_t buf[sizeof(CyclePhaseShift)];

      return new (buf) CyclePhaseShift(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_CyclePhaseShift(const NotNullCommandPtr command0,
                                          const NotNullCommandPtr command1,
                                          const NotNullCommandPtr command2,
                                          const NotNullCommandPtr command3,
                                          const NotNullCommandPtr command4,
                                          const NotNullCommandPtr command5)
    {
      static etl::array<NotNullCommandPtr, 6> commands{command0, command1, command2, command3, command4, command5};
      static uint8_t buf[sizeof(CyclePhaseShift)];

      return new (buf) CyclePhaseShift(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_CyclePhaseShift(const NotNullCommandPtr command0,
                                          const NotNullCommandPtr command1,
                                          const NotNullCommandPtr command2,
                                          const NotNullCommandPtr command3,
                                          const NotNullCommandPtr command4,
                                          const NotNullCommandPtr command5,
                                          const NotNullCommandPtr command6)
    {
      static etl::array<NotNullCommandPtr, 7> commands{command0, command1, command2, command3, command4, command5, command6};
      static uint8_t buf[sizeof(CyclePhaseShift)];

      return new (buf) CyclePhaseShift(commands);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_CyclePhaseShift(const NotNullCommandPtr command0,
                                          const NotNullCommandPtr command1,
                                          const NotNullCommandPtr command2,
                                          const NotNullCommandPtr command3,
                                          const NotNullCommandPtr command4,
                                          const NotNullCommandPtr command5,
                                          const NotNullCommandPtr command6,
                                          const NotNullCommandPtr command7)
    {
      static etl::array<NotNullCommandPtr, 8> commands{command0, command1, command2, command3, command4, command5, command6, command7};
      static uint8_t buf[sizeof(CyclePhaseShift)];

      return new (buf) CyclePhaseShift(commands);
    }

#define CYC_PS(...) (Internal::new_CyclePhaseShift<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

    //------------------------------------------------------------------+
    // NoOperation(NOP)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_NoOperation()
    {
      static uint8_t buf[sizeof(NoOperation)];
      return new (buf) NoOperation();
    }

#define NOP() (Internal::new_NoOperation<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>())

    //------------------------------------------------------------------+
    // Shift(SFT)
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Shift(ShiftId shift_id)
    {
      static etl::array<ShiftIdLink, 1> shift_ids{
          Shift::IdToLink(shift_id),
      };
      static uint8_t buf[sizeof(Shift)];
      return new (buf) Shift(shift_ids);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Shift(ShiftId shift_id0,
                                ShiftId shift_id1)
    {
      static etl::array<ShiftIdLink, 2> shift_ids{
          Shift::IdToLink(shift_id0),
          Shift::IdToLink(shift_id1),
      };
      static uint8_t buf[sizeof(Shift)];
      return new (buf) Shift(shift_ids);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Shift(ShiftId shift_id0,
                                ShiftId shift_id1,
                                ShiftId shift_id2)
    {
      static etl::array<ShiftIdLink, 3> shift_ids{
          Shift::IdToLink(shift_id0),
          Shift::IdToLink(shift_id1),
          Shift::IdToLink(shift_id2),
      };
      static uint8_t buf[sizeof(Shift)];
      return new (buf) Shift(shift_ids);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Shift(ShiftId shift_id0,
                                ShiftId shift_id1,
                                ShiftId shift_id2,
                                ShiftId shift_id3)
    {
      static etl::array<ShiftIdLink, 4> shift_ids{
          Shift::IdToLink(shift_id0),
          Shift::IdToLink(shift_id1),
          Shift::IdToLink(shift_id2),
          Shift::IdToLink(shift_id3),
      };
      static uint8_t buf[sizeof(Shift)];
      return new (buf) Shift(shift_ids);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Shift(ShiftId shift_id0,
                                ShiftId shift_id1,
                                ShiftId shift_id2,
                                ShiftId shift_id3,
                                ShiftId shift_id4)
    {
      static etl::array<ShiftIdLink, 5> shift_ids{
          Shift::IdToLink(shift_id0),
          Shift::IdToLink(shift_id1),
          Shift::IdToLink(shift_id2),
          Shift::IdToLink(shift_id3),
          Shift::IdToLink(shift_id4),
      };
      static uint8_t buf[sizeof(Shift)];
      return new (buf) Shift(shift_ids);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Shift(ShiftId shift_id0,
                                ShiftId shift_id1,
                                ShiftId shift_id2,
                                ShiftId shift_id3,
                                ShiftId shift_id4,
                                ShiftId shift_id5)
    {
      static etl::array<ShiftIdLink, 6> shift_ids{
          Shift::IdToLink(shift_id0),
          Shift::IdToLink(shift_id1),
          Shift::IdToLink(shift_id2),
          Shift::IdToLink(shift_id3),
          Shift::IdToLink(shift_id4),
          Shift::IdToLink(shift_id5),
      };
      static uint8_t buf[sizeof(Shift)];
      return new (buf) Shift(shift_ids);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Shift(ShiftId shift_id0,
                                ShiftId shift_id1,
                                ShiftId shift_id2,
                                ShiftId shift_id3,
                                ShiftId shift_id4,
                                ShiftId shift_id5,
                                ShiftId shift_id6)
    {
      static etl::array<ShiftIdLink, 7> shift_ids{
          Shift::IdToLink(shift_id0),
          Shift::IdToLink(shift_id1),
          Shift::IdToLink(shift_id2),
          Shift::IdToLink(shift_id3),
          Shift::IdToLink(shift_id4),
          Shift::IdToLink(shift_id5),
          Shift::IdToLink(shift_id6),
      };
      static uint8_t buf[sizeof(Shift)];
      return new (buf) Shift(shift_ids);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    NotNullCommandPtr new_Shift(ShiftId shift_id0,
                                ShiftId shift_id1,
                                ShiftId shift_id2,
                                ShiftId shift_id3,
                                ShiftId shift_id4,
                                ShiftId shift_id5,
                                ShiftId shift_id6,
                                ShiftId shift_id7)
    {
      static etl::array<ShiftIdLink, 8> shift_ids{
          Shift::IdToLink(shift_id0),
          Shift::IdToLink(shift_id1),
          Shift::IdToLink(shift_id2),
          Shift::IdToLink(shift_id3),
          Shift::IdToLink(shift_id4),
          Shift::IdToLink(shift_id5),
          Shift::IdToLink(shift_id6),
          Shift::IdToLink(shift_id7),
      };
      static uint8_t buf[sizeof(Shift)];
      return new (buf) Shift(shift_ids);
    }

#define SFT(...) (Internal::new_Shift<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

    //------------------------------------------------------------------+
    // KEYMAP
    //------------------------------------------------------------------+
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3, size_t N>
    etl::span<Key> KEYMAP_impl(const Key (&keymap)[N])
    {
      static etl::vector<Key, N> result;
      result.assign(std::begin(keymap), std::end(keymap));
      return result;
    }

#define KEYMAP(...) (Internal::KEYMAP_impl<__COUNTER__, consthash::city64(__FILE__, sizeof(__FILE__)), consthash::crc64(__FILE__, sizeof(__FILE__))>(__VA_ARGS__))

    //------------------------------------------------------------------+
    // nullptr alias (_ * 7)
    //------------------------------------------------------------------+

#define _______ (nullptr)

  } // namespace Internal

} // namespace hidpg
