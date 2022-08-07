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

#define GESTURE_ID_LINK_ID 2

namespace hidpg
{

  struct Key
  {
    uint8_t key_id;
    Command *command;
  };

  struct SequenceKey
  {
    uint8_t key_ids[HID_ENGINE_MAX_SEQUENCE_COUNT];
    Command *command;
    size_t key_ids_len;
  };

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
    uint8_t gesture_id;
    uint8_t mouse_id;
    uint16_t distance;
    AngleSnap angle_snap;
    Command *up_command;
    Command *down_command;
    Command *left_command;
    Command *right_command;
    Command *pre_command;
    PreCommandTiming pre_command_timing;
  };

  struct Encoder
  {
    uint8_t encoder_id;
    Command *counterclockwise_command;
    Command *clockwise_command;
  };

  typedef etl::bidirectional_link<GESTURE_ID_LINK_ID> GestureIDLink;

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

  namespace Internal
  {

    class HidEngineClass
    {
      friend class HidEngineTaskClass;

    public:
      template <uint8_t keymap_len>
      static void setKeymap(Key (&keymap)[keymap_len])
      {
        _keymap = keymap;
        _keymap_len = keymap_len;
      }

      template <uint8_t sequence_keymap_len>
      static void setSequenceKeymap(SequenceKey (&sequence_keymap)[sequence_keymap_len])
      {
        _sequence_keymap = sequence_keymap;
        _sequence_keymap_len = sequence_keymap_len;

        for (int i = 0; i < _sequence_keymap_len; i++)
        {
          _sequence_keymap[i].key_ids_len = getValidLength(_sequence_keymap[i].key_ids, HID_ENGINE_MAX_SEQUENCE_COUNT);
        }
      }

      template <uint8_t gesture_map_len>
      static void setGestureMap(Gesture (&gesture_map)[gesture_map_len])
      {
        _gesture_map = gesture_map;
        _gesture_map_len = gesture_map_len;
      }

      template <uint8_t encoder_map_len>
      static void setEncoderMap(Encoder (&encoder_map)[encoder_map_len])
      {
        _encoder_map = encoder_map;
        _encoder_map_len = encoder_map_len;
      }

      using read_mouse_delta_callback_t = void (*)(uint8_t mouse_id, int16_t &delta_x, int16_t &delta_y);
      using read_encoder_step_callback_t = void (*)(uint8_t encoder_id, int32_t &step);

      static void setHidReporter(HidReporter *hid_reporter);
      static void start();
      static void applyToKeymap(const Set &key_ids);
      static void mouseMove(uint8_t mouse_id);
      static void rotateEncoder(uint8_t encoder_id);
      static void setReadMouseDeltaCallback(read_mouse_delta_callback_t cb);
      static void setReadEncoderStepCallback(read_encoder_step_callback_t cb);

      static void switchSequenceMode();
      static void startGesture(GestureID &gesture_id);
      static void stopGesture(GestureID &gesture_id);

    private:
      static void applyToKeymap_impl(Set &key_ids);
      static void processSequenceKeymap(Set &key_ids);
      static void processKeymap(Set &key_ids);
      static void mouseMove_impl(uint8_t mouse_id);
      static void processGestureX(Gesture &gesture, GestureID &gesture_id);
      static void processGestureXSub(Gesture &gesture, GestureID &gesture_id, Command *command);
      static void processGestureY(Gesture &gesture, GestureID &gesture_id);
      static void processGestureYSub(Gesture &gesture, GestureID &gesture_id, Command *command);
      static bool processPreCommandInsteadOfFirstGesture(Gesture &gesture, GestureID &gesture_id);
      static void rotateEncoder_impl(uint8_t encoder_id);

      enum class MatchResult
      {
        NoMatch,
        PartialMatch,
        Match,
      };

      static MatchResult matchWithSequenceKeymap(const uint8_t id_seq[], size_t len, SequenceKey **matched);
      static size_t getValidLength(const uint8_t key_ids[], size_t max_len);

      static Key *_keymap;
      static SequenceKey *_sequence_keymap;
      static Gesture *_gesture_map;
      static Encoder *_encoder_map;

      static uint8_t _keymap_len;
      static uint8_t _sequence_keymap_len;
      static uint8_t _gesture_map_len;
      static uint8_t _encoder_map_len;

      static read_mouse_delta_callback_t _read_mouse_delta_cb;
      static read_encoder_step_callback_t _read_encoder_step_cb;

      enum class SequenceModeState
      {
        Disable,
        MatchProcess,
        WaitRelease,
      };

      static SequenceModeState _sequence_mode_state;

      static etl::intrusive_list<GestureID, GestureIDLink> _gesture_id_list;
      static int32_t _total_distance_x;
      static int32_t _total_distance_y;
    };

    //------------------------------------------------------------------+
    // SequenceMode
    //------------------------------------------------------------------+
    class SequenceMode : public Command
    {
    protected:
      void onPress(uint8_t n_times) override;
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
      GestureOr(uint8_t gesture_id, Command *command);

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
      Command *_command;
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
