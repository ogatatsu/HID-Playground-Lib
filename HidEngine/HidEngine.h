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

#include "Command.h"
#include "HidReporter.h"
#include "Set.h"
#include "etl/optional.h"

#define GESTURE_ID_LINK_ID 0
#define COMBO_LINK_ID 0

namespace hidpg
{
  // Key
  struct Key
  {
    Key(uint8_t key_id, NotNullCommandPtr command)
        : key_id(key_id), command(command) {}

    const uint8_t key_id;
    const NotNullCommandPtr command;
  };

  // Combo
  using ComboLink = etl::bidirectional_link<COMBO_LINK_ID>;

  struct Combo : public ComboLink
  {
    Combo(uint8_t first_key_id, uint8_t second_key_id, NotNullCommandPtr command, uint32_t combo_term_ms)
        : first_key_id(first_key_id), second_key_id(second_key_id), command(command), combo_term_ms(combo_term_ms) {}

    const uint8_t first_key_id;
    const uint8_t second_key_id;
    const NotNullCommandPtr command;
    const uint32_t combo_term_ms;
    bool first_id_rereased;
    bool second_id_rereased;
  };

  // Gesture
  enum class AngleSnap : uint8_t
  {
    Enable,
    Disable,
  };

  enum class PreCommandTiming : uint8_t
  {
    Immediately,
    InsteadOfFirstGesture,
  };

  struct Gesture
  {
    Gesture(uint8_t gesture_id,
            uint8_t mouse_id,
            uint16_t distance,
            AngleSnap angle_snap,
            Command *up_command,
            Command *down_command,
            Command *left_command,
            Command *right_command,
            Command *pre_command,
            PreCommandTiming pre_command_timing)
        : gesture_id(gesture_id),
          mouse_id(mouse_id),
          distance(distance),
          angle_snap(angle_snap),
          up_command(up_command),
          down_command(down_command),
          left_command(left_command),
          right_command(right_command),
          pre_command(pre_command),
          pre_command_timing(pre_command_timing) {}

    const uint8_t gesture_id;
    const uint8_t mouse_id;
    const uint16_t distance;
    const AngleSnap angle_snap;
    const CommandPtr up_command;
    const CommandPtr down_command;
    const CommandPtr left_command;
    const CommandPtr right_command;
    const CommandPtr pre_command;
    const PreCommandTiming pre_command_timing;
  };

  using GestureIDLink = etl::bidirectional_link<GESTURE_ID_LINK_ID>;

  struct GestureID : public GestureIDLink
  {
    GestureID(uint8_t gesture_id) : _gesture_id(gesture_id) { clear(); }
    uint8_t getID() const { return _gesture_id; }
    void setPreCommandPressFlag(bool flag) { _is_pre_command_pressed = flag; }
    bool getPreCommandPressFlag() { return _is_pre_command_pressed; }

  private:
    uint8_t _gesture_id;
    bool _is_pre_command_pressed;
  };

  // Encoder
  struct Encoder
  {
    Encoder(uint8_t encoder_id, NotNullCommandPtr counterclockwise_command, NotNullCommandPtr clockwise_command)
        : encoder_id(encoder_id), counterclockwise_command(counterclockwise_command), clockwise_command(clockwise_command) {}

    const uint8_t encoder_id;
    NotNullCommandPtr counterclockwise_command;
    NotNullCommandPtr clockwise_command;
  };

  namespace Internal
  {

    class HidEngineClass
    {
      friend class HidEngineTaskClass;

    public:
      using read_mouse_delta_callback_t = void (*)(uint8_t mouse_id, int16_t &delta_x, int16_t &delta_y);
      using read_encoder_step_callback_t = void (*)(uint8_t encoder_id, int32_t &step);

      static void setKeymap(etl::span<Key> keymap);
      static void setComboMap(etl::span<Combo> combo_map);
      static void setGestureMap(etl::span<Gesture> gesture_map);
      static void setEncoderMap(etl::span<Encoder> encoder_map);
      static void setHidReporter(HidReporter *hid_reporter);
      static void start();
      static void applyToKeymap(const Set &key_ids);
      static void mouseMove(uint8_t mouse_id);
      static void rotateEncoder(uint8_t encoder_id);
      static void setReadMouseDeltaCallback(read_mouse_delta_callback_t cb);
      static void setReadEncoderStepCallback(read_encoder_step_callback_t cb);

      static void startGesture(GestureID &gesture_id);
      static void stopGesture(GestureID &gesture_id);

    private:
      enum class Action
      {
        Press,
        Release,
        ComboTermTimer,
      };

      static void applyToKeymap_impl(Set &key_ids);
      static void processComboAndKey(Action action, etl::optional<uint8_t> key_id);
      static void performKeyPress(uint8_t key_id);
      static void performKeyRelease(uint8_t key_id);

      static void mouseMove_impl(uint8_t mouse_id);
      static void processGestureX(Gesture &gesture, GestureID &gesture_id);
      static void processGestureY(Gesture &gesture, GestureID &gesture_id);
      static void performGestureX(Gesture &gesture, GestureID &gesture_id, Command *command);
      static void performGestureY(Gesture &gesture, GestureID &gesture_id, Command *command);
      static bool processPreCommandInsteadOfFirstGesture(Gesture &gesture, GestureID &gesture_id);

      static void rotateEncoder_impl(uint8_t encoder_id);

      static etl::span<Key> _keymap;
      static etl::span<Combo> _combo_map;
      static etl::span<Gesture> _gesture_map;
      static etl::span<Encoder> _encoder_map;

      static read_mouse_delta_callback_t _read_mouse_delta_cb;
      static read_encoder_step_callback_t _read_encoder_step_cb;

      class ComboTermTimer : public TimerMixin
      {
      public:
        void startTimer(uint32_t ms) { TimerMixin::startTimer(ms); }
        void onTimer() override { processComboAndKey(Action::ComboTermTimer, etl::nullopt); }
      };
      static ComboTermTimer _combo_term_timer;
      static void startComboTermTimer(uint32_t ms) { _combo_term_timer.startTimer(ms); };

      static etl::intrusive_list<GestureID, GestureIDLink> _started_gesture_id_list;
      static int32_t _total_distance_x;
      static int32_t _total_distance_y;
    };

    //------------------------------------------------------------------+
    // GestureCommand
    //------------------------------------------------------------------+
    class GestureCommand : public Command
    {
    public:
      GestureCommand(uint8_t gesture_id);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;

    private:
      GestureID _gesture_id;
    };

    //------------------------------------------------------------------+
    // GestureOr
    //------------------------------------------------------------------+
    class GestureOr : public Command, public BeforeOtherCommandPressEventListener, public BeforeGestureEventListener
    {
    public:
      GestureOr(uint8_t gesture_id, NotNullCommandPtr command);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;
      void onBeforeOtherCommandPress(Command &command) override;
      void onBeforeGesture(uint8_t gesture_id, uint8_t mouse_id) override;
      void startListen();
      void stopListen();

    private:
      enum class State : uint8_t
      {
        Unexecuted,
        Pressed,
        OtherCommandPressed,
        Gestured,
      };

      GestureID _gesture_id;
      NotNullCommandPtr _command;
      State _state;
    };

    //------------------------------------------------------------------+
    // GestureOrNK
    //------------------------------------------------------------------+
    class GestureOrNK : public Command, public BeforeOtherCommandPressEventListener, public BeforeGestureEventListener
    {
    public:
      GestureOrNK(uint8_t gesture_id, KeyCode key_code);

    protected:
      void onPress(uint8_t n_times) override;
      uint8_t onRelease() override;
      void onBeforeOtherCommandPress(Command &command) override;
      void onBeforeGesture(uint8_t gesture_id, uint8_t mouse_id) override;
      void startListen();
      void stopListen();

    private:
      enum class State : uint8_t
      {
        Unexecuted,
        Pressed,
        PressedWithModifiers,
        OtherCommandPressed,
        Gestured,
      };

      GestureID _gesture_id;
      NormalKey _nk_command;
      State _state;
    };

  } // namespace Internal

  extern Internal::HidEngineClass HidEngine;

} // namespace hidpg
