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

#include "CommandBase.h"
#include "HidReporter.h"
#include "Set.h"
#include "TimerMixin.h"
#include "etl/optional.h"
#include "etl/span.h"
#include "etl/vector.h"
#include <tuple>

namespace hidpg
{
  // ------------------------------------------------------------------+
  // Key
  // ------------------------------------------------------------------+
  enum class Timing : uint8_t
  {
    Immediately,
    JustBeforeFirstAction,
    InsteadOfFirstAction,
  };

  enum class KeymapOverlay : uint8_t
  {
    Enable,
    Disable,
  };

  struct PreCommand
  {
    PreCommand(NotNullCommandPtr command, Timing timing)
        : command(command), timing(timing), is_pressed(false) {}

    const NotNullCommandPtr command;
    const Timing timing;

    bool is_pressed;
  };

  struct Key : public etl::bidirectional_link<0>
  {
    Key(uint8_t key_id, NotNullCommandPtr command)
        : etl::bidirectional_link<0>(), key_id(key_id), command(command)
    {
      command->setKeyId(key_id);
    }

    const uint8_t key_id;
    const NotNullCommandPtr command;
  };

  struct KeyShift
  {
    KeyShift(KeyShiftId key_shift_id,
             KeymapOverlay keymap_overlay,
             etl::span<Key> keymap,
             etl::optional<PreCommand> pre_command = etl::nullopt)
        : key_shift_id(key_shift_id),
          keymap_overlay(keymap_overlay),
          keymap(keymap),
          pre_command(pre_command)
    {
      for (auto &key : keymap)
      {
        key.command->setKeyId(key.key_id);
      }
    }

    const KeyShiftId key_shift_id;
    const KeymapOverlay keymap_overlay;
    const etl::span<Key> keymap;
    etl::optional<PreCommand> pre_command;
  };

  struct KeyShiftIdLink : public etl::bidirectional_link<0>
  {
    KeyShiftIdLink(uint8_t value = 0) : etl::bidirectional_link<0>(), value(value) { clear(); }

    const uint8_t value;
  };

  // ------------------------------------------------------------------+
  // Combo
  // ------------------------------------------------------------------+
  enum class ComboBehavior : uint8_t
  {
    AnyOrder_SlowRelease = 0b00,
    AnyOrder_FastRelease = 0b01,
    SpecifiedOrder_SlowRelease = 0b10,
    SpecifiedOrder_FastRelease = 0b11,
  };

  struct Combo : public etl::bidirectional_link<0>
  {
    Combo(uint8_t first_key_id,
          uint8_t second_key_id,
          NotNullCommandPtr command,
          uint32_t combo_term_ms,
          ComboBehavior combo_behavior)
        : first_id(first_key_id),
          second_id(second_key_id),
          command(command),
          combo_term_ms(combo_term_ms),
          behavior(combo_behavior)
    {
    }

    const uint8_t first_id;
    const uint8_t second_id;
    const NotNullCommandPtr command;
    const uint32_t combo_term_ms;
    const ComboBehavior behavior;

    bool first_id_rereased;
    bool second_id_rereased;

    bool isSpecifiedOrder() { return static_cast<uint8_t>(behavior) & 0b10; }
    bool isAnyOrder() { return !isSpecifiedOrder(); }
    bool isFastRelease() { return static_cast<uint8_t>(behavior) & 0b01; }
    bool isSlowRelease() { return !isFastRelease(); }

    bool isMatchFirstId(uint8_t id)
    {
      if (isSpecifiedOrder())
      {
        return first_id == id;
      }
      return first_id == id || second_id == id;
    }

    bool isMatchIds(uint8_t fst, uint8_t snd)
    {
      if (isSpecifiedOrder())
      {
        return first_id == fst && second_id == snd;
      }
      return (first_id == fst && second_id == snd) || (first_id == snd && second_id == fst);
    }
  };

  // ------------------------------------------------------------------+
  // Gesture
  // ------------------------------------------------------------------+
  enum class AngleSnap : uint8_t
  {
    Enable,
    Disable,
  };

  struct Gesture
  {
    Gesture(GestureId gesture_id,
            PointingDeviceId pointing_device_id,
            uint16_t distance,
            AngleSnap angle_snap,
            CommandPtr up_command,
            CommandPtr down_command,
            CommandPtr left_command,
            CommandPtr right_command,
            etl::optional<PreCommand> pre_command = etl::nullopt)
        : gesture_id(gesture_id),
          pointing_device_id(pointing_device_id),
          distance(distance),
          angle_snap(angle_snap),
          up_command(up_command),
          down_command(down_command),
          left_command(left_command),
          right_command(right_command),
          pre_command(pre_command),
          total_distance_x(0),
          total_distance_y(0),
          instead_of_first_gesture_millis(etl::nullopt)
    {
    }

    const GestureId gesture_id;
    const PointingDeviceId pointing_device_id;
    const uint16_t distance;
    const AngleSnap angle_snap;
    const CommandPtr up_command;
    const CommandPtr down_command;
    const CommandPtr left_command;
    const CommandPtr right_command;

    etl::optional<PreCommand> pre_command;
    int32_t total_distance_x;
    int32_t total_distance_y;
    etl::optional<uint32_t> instead_of_first_gesture_millis;
  };

  struct GestureIdLink : public etl::bidirectional_link<0>
  {
    GestureIdLink(uint8_t value = 0) : value(value) { clear(); }

    const uint8_t value;
  };

  // ------------------------------------------------------------------+
  // Encoder
  // ------------------------------------------------------------------+
  struct EncoderShift
  {
    EncoderShift(EncoderShiftId encoder_shift_id,
                 EncoderId encoder_id,
                 uint8_t step,
                 NotNullCommandPtr counterclockwise_command,
                 NotNullCommandPtr clockwise_command,
                 etl::optional<PreCommand> pre_command = etl::nullopt)
        : encoder_shift_id(encoder_shift_id),
          encoder_id(encoder_id),
          step(step),
          counterclockwise_command(counterclockwise_command),
          clockwise_command(clockwise_command),
          pre_command(pre_command)
    {
    }

    const EncoderShiftId encoder_shift_id;
    const EncoderId encoder_id;
    const uint8_t step;
    const NotNullCommandPtr counterclockwise_command;
    const NotNullCommandPtr clockwise_command;

    int32_t total_step;
    etl::optional<PreCommand> pre_command;
  };

  struct Encoder : public EncoderShift
  {
    Encoder(EncoderId encoder_id,
            uint8_t step,
            NotNullCommandPtr counterclockwise_command,
            NotNullCommandPtr clockwise_command)
        : EncoderShift(EncoderShiftId{0},
                       encoder_id,
                       step,
                       counterclockwise_command,
                       clockwise_command,
                       etl::nullopt)
    {
    }
  };

  struct EncoderShiftIdLink : public etl::bidirectional_link<0>
  {
    EncoderShiftIdLink(uint8_t value = 0) : value(value) { clear(); }

    const uint8_t value;
  };

  // ------------------------------------------------------------------+
  // HidEngineClass
  // ------------------------------------------------------------------+
  namespace Internal
  {

    class HidEngineClass
    {
      friend class HidEngineTaskClass;

    public:
      using read_pointer_delta_callback_t = void (*)(PointingDeviceId pointing_device_id, int16_t &delta_x, int16_t &delta_y);
      using read_encoder_step_callback_t = void (*)(EncoderId encoder_id, int16_t &step);

      static void setKeymap(etl::span<Key> keymap);
      static void setKeyShiftMap(etl::span<KeyShift> key_shift_map);
      static void setComboMap(etl::span<Combo> combo_map);
      static void setGestureMap(etl::span<Gesture> gesture_map);
      static void setEncoderMap(etl::span<Encoder> encoder_map);
      static void setEncoderShiftMap(etl::span<hidpg::EncoderShift> encoder_shift_map);
      static void setHidReporter(HidReporter *hid_reporter);
      static void start();
      static void applyToKeymap(const Set &key_ids);
      static void movePointer(PointingDeviceId pointing_device_id);
      static void rotateEncoder(EncoderId encoder_id);
      static void setReadPointerDeltaCallback(read_pointer_delta_callback_t cb);
      static void setReadEncoderStepCallback(read_encoder_step_callback_t cb);

      static void startKeyShift(KeyShiftIdLink &key_shift_id);
      static void stopKeyShift(KeyShiftIdLink &key_shift_id);
      static void startGesture(GestureIdLink &gesture_id);
      static void stopGesture(GestureIdLink &gesture_id);
      static void startEncoderShift(EncoderShiftIdLink &encoder_shift_id);
      static void stopEncoderShift(EncoderShiftIdLink &encoder_shift_id);

    private:
      enum class Action
      {
        Press,
        Release,
        ComboInterruption,
      };

      static void applyToKeymap_impl(Set &key_ids);
      static void processComboAndKey(Action action, etl::optional<uint8_t> key_id);
      static void performKeyPress(uint8_t key_id);
      static void performKeyRelease(uint8_t key_id);
      static std::tuple<KeyShift *, Key *> getCurrentKey(uint8_t key_id);

      static void movePointer_impl(PointingDeviceId pointing_device_id);
      static Gesture *getCurrentGesture(PointingDeviceId pointing_device_id);
      static void processGesture(Gesture &gesture, int16_t delta_x, int16_t delta_y);
      static void processGestureX(Gesture &gesture);
      static void processGestureY(Gesture &gesture);
      static void processGesturePreCommand(Gesture &gesture, uint8_t &n_times);

      static void rotateEncoder_impl(EncoderId encoder_id);
      static EncoderShift *getCurrentEncoder(EncoderId encoder_id);

      static etl::span<Key> _keymap;
      static etl::span<KeyShift> _key_shift_map;
      static etl::span<Combo> _combo_map;
      static etl::span<Gesture> _gesture_map;
      static etl::span<Encoder> _encoder_map;
      static etl::span<EncoderShift> _encoder_shift_map;

      static read_pointer_delta_callback_t _read_pointer_delta_cb;
      static read_encoder_step_callback_t _read_encoder_step_cb;

      class ComboInterruptionEvent : public TimerMixin,
                                     public BeforeMovePointerEventListener,
                                     public BeforeRotateEncoderEventListener
      {
      public:
        void start(unsigned int combo_term_ms)
        {
          _delta_x_sum = 0;
          _delta_y_sum = 0;
          startTimer(combo_term_ms);
          startListenBeforeMovePointer();
          startListenBeforeRotateEncoder();
        }

        void stop()
        {
          stopTimer();
          stopListenBeforeMovePointer();
          stopListenBeforeRotateEncoder();
        }

      protected:
        void onTimer() override { processComboAndKey(Action::ComboInterruption, etl::nullopt); }
        void onBeforeRotateEncoder(EncoderId, int16_t) override { processComboAndKey(Action::ComboInterruption, etl::nullopt); }
        void onBeforeMovePointer(PointingDeviceId, int16_t delta_x, int16_t delta_y) override
        {
          _delta_x_sum = etl::clamp(_delta_x_sum + delta_x, INT16_MIN, INT16_MAX);
          _delta_y_sum = etl::clamp(_delta_y_sum + delta_y, INT16_MIN, INT16_MAX);
          if (abs(_delta_x_sum) >= HID_ENGINE_COMBO_INTERRUPTION_MOVE_POINTER_DELTA ||
              abs(_delta_y_sum) >= HID_ENGINE_COMBO_INTERRUPTION_MOVE_POINTER_DELTA)
          {
            processComboAndKey(Action::ComboInterruption, etl::nullopt);
          }
        }

      private:
        int16_t _delta_x_sum;
        int16_t _delta_y_sum;
      };
      static ComboInterruptionEvent _combo_interruption_event;

      static etl::intrusive_list<Key> _pressed_key_list;
      static etl::intrusive_list<KeyShiftIdLink> _started_key_shift_id_list;
      static etl::intrusive_list<GestureIdLink> _started_gesture_id_list;
      static etl::intrusive_list<EncoderShiftIdLink> _started_encoder_shift_id_list;
    };

  } // namespace Internal

  extern Internal::HidEngineClass HidEngine;

} // namespace hidpg
