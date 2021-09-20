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

#define TRACKING_LINK_ID 2

namespace hidpg
{

  struct Key
  {
    uint8_t key_id;
    Command *command;
  };

  struct SimulKey
  {
    uint8_t key_ids[HID_ENGINE_MAX_SIMUL_PRESS_COUNT];
    Command *command;
    size_t key_ids_len;
  };

  struct SeqKey
  {
    uint8_t key_ids[HID_ENGINE_MAX_SEQ_COUNT];
    Command *command;
    size_t key_ids_len;
  };

  enum class AngleSnap : uint8_t
  {
    Enable,
    Disable,
  };

  struct Track
  {
    uint8_t track_id;
    uint16_t threshold_distance;
    AngleSnap angle_snap;
    Command *up_command;
    Command *down_command;
    Command *left_command;
    Command *right_command;
  };

  struct Encoder
  {
    uint8_t encoder_id;
    Command *counterclockwise_command;
    Command *clockwise_command;
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

      template <uint8_t simul_keymap_len>
      static void setSimulKeymap(SimulKey (&simul_keymap)[simul_keymap_len])
      {
        _simul_keymap = simul_keymap;
        _simul_keymap_len = simul_keymap_len;

        for (int i = 0; i < _simul_keymap_len; i++)
        {
          _simul_keymap[i].key_ids_len = getValidLength(_simul_keymap[i].key_ids, HID_ENGINE_MAX_SIMUL_PRESS_COUNT);
        }
      }

      template <uint8_t seq_keymap_len>
      static void setSeqKeymap(SeqKey (&seq_keymap)[seq_keymap_len])
      {
        _seq_keymap = seq_keymap;
        _seq_keymap_len = seq_keymap_len;

        for (int i = 0; i < _seq_keymap_len; i++)
        {
          _seq_keymap[i].key_ids_len = getValidLength(_seq_keymap[i].key_ids, HID_ENGINE_MAX_SEQ_COUNT);
        }
      }

      template <uint8_t track_map_len>
      static void setTrackMap(Track (&track_map)[track_map_len])
      {
        _track_map = track_map;
        _track_map_len = track_map_len;
      }

      template <uint8_t encoder_map_len>
      static void setEncoderMap(Encoder (&encoder_map)[encoder_map_len])
      {
        _encoder_map = encoder_map;
        _encoder_map_len = encoder_map_len;
      }

      using read_mouse_delta_callback_t = void (*)(int16_t &delta_x, int16_t &delta_y);
      using read_encoder_step_callback_t = void (*)(uint8_t encoder_id, int32_t &step);

      static void setHidReporter(HidReporter *hid_reporter);
      static void start();
      static void applyToKeymap(const Set &key_ids);
      static void mouseMove();
      static void rotateEncoder(uint8_t encoder_id);
      static void setReadMouseDeltaCallback(read_mouse_delta_callback_t cb);
      static void setReadEncoderStepCallback(read_encoder_step_callback_t cb);

    private:
      static void applyToKeymap_impl(Set &key_ids);
      static void processSeqKeymap(Set &key_ids);
      static void processSimulKeymap(Set &key_ids);
      static void processKeymap(Set &key_ids);
      static void mouseMove_impl();
      static void processTrackX(size_t track_map_idx);
      static void processTrackY(size_t track_map_idx);
      static void rotateEncoder_impl(uint8_t encoder_id);
      static int match_with_seqKeymap(const uint8_t id_seq[], size_t len, SeqKey **matched);
      static size_t getValidLength(const uint8_t key_ids[], size_t max_len);

      static Key *_keymap;
      static SimulKey *_simul_keymap;
      static SeqKey *_seq_keymap;
      static Track *_track_map;
      static Encoder *_encoder_map;

      static uint8_t _keymap_len;
      static uint8_t _simul_keymap_len;
      static uint8_t _seq_keymap_len;
      static uint8_t _track_map_len;
      static uint8_t _encoder_map_len;

      static read_mouse_delta_callback_t _read_mouse_delta_cb;
      static read_encoder_step_callback_t _read_encoder_step_cb;

      //------------------------------------------------------------------+
      // HidEngine inner command
      //------------------------------------------------------------------+
    public:
      class SequenceMode : public Command
      {
      protected:
        void onPress(uint8_t n_times) override;
      };

      typedef etl::bidirectional_link<TRACKING_LINK_ID> TrackingLink;

      class Tracking : public Command, public TrackingLink
      {
      public:
        Tracking(uint8_t track_id);
        uint8_t getID();

      protected:
        void onPress(uint8_t n_times) override;
        uint8_t onRelease() override;

      private:
        uint8_t _track_id;
      };

      class TrackTap : public Tracking
      {
      public:
        TrackTap(uint8_t track_id, Command *command);

      protected:
        uint8_t onRelease() override;

      private:
        Command *_command;
      };

    private:
      enum class SeqModeState
      {
        Disable,
        Running,
        WaitRelease,
      };

      static void switchSequenceMode();
      static SeqModeState _seq_mode_state;

      static void startTracking(HidEngineClass::Tracking &tracking);
      static void stopTracking(HidEngineClass::Tracking &tracking);
      static etl::intrusive_list<Tracking, TrackingLink> _tracking_list;

      static int32_t _distance_x;
      static int32_t _distance_y;
    };

  } // namespace Internal

  extern Internal::HidEngineClass HidEngine;

} // namespace hidpg
