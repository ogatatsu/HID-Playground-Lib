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

namespace hidpg
{

  struct Key
  {
    uint8_t key_id;
    Command *command;
  };

  struct SimulKey
  {
    uint8_t key_ids[MAX_SIMUL_PRESS_COUNT];
    Command *command;
    size_t key_ids_len;
  };

  struct SeqKey
  {
    uint8_t key_ids[MAX_SEQ_COUNT];
    Command *command;
    size_t key_ids_len;
  };

  struct Track
  {
    uint8_t track_id;
    uint16_t threshold_distance;
    Command *up_command;
    Command *down_command;
    Command *left_command;
    Command *right_command;
  };

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
        _simul_keymap[i].key_ids_len = getValidLength(_simul_keymap[i].key_ids, MAX_SIMUL_PRESS_COUNT);
      }
    }

    template <uint8_t seq_keymap_len>
    static void setSeqKeymap(SeqKey (&seq_keymap)[seq_keymap_len])
    {
      _seq_keymap = seq_keymap;
      _seq_keymap_len = seq_keymap_len;

      for (int i = 0; i < _seq_keymap_len; i++)
      {
        _seq_keymap[i].key_ids_len = getValidLength(_seq_keymap[i].key_ids, MAX_SEQ_COUNT);
      }
    }

    template <uint8_t trackmap_len>
    static void setTrackmap(Track (&trackmap)[trackmap_len])
    {
      _trackmap = trackmap;
      _trackmap_len = trackmap_len;
    }

    static void setHidReporter(HidReporter *hid_reporter);
    static void begin();
    static void applyToKeymap(const Set &key_ids);
    static void tapCommand(Command *command, uint8_t n_times);
    static void mouseMove(int16_t x, int16_t y);

    //------------------------------------------------------------------+
    // HidEngine inner command
    //------------------------------------------------------------------+
    class SequenceMode : public Command
    {
    protected:
      uint8_t onPress(uint8_t accumulation) override;
    };

    class Tracking : public Command
    {
    public:
      Tracking(uint8_t track_id);
      uint8_t getID();

    protected:
      uint8_t onPress(uint8_t accumulation) override;
      void onRelease() override;

    private:
      uint8_t _track_id;
    };

    class TrackTap : public Tracking
    {
    public:
      TrackTap(uint8_t track_id, Command *command);

    protected:
      void onRelease() override;

    private:
      Command *_command;
    };

  private:
    static void applyToKeymap_impl(const Set &key_ids);
    static void mouseMove_impl(int16_t x, int16_t y);
    static int match_with_seqKeymap(const uint8_t id_seq[], size_t len, SeqKey **matched);
    static size_t getValidLength(const uint8_t key_ids[], size_t max_len);

    static Key *_keymap;
    static SimulKey *_simul_keymap;
    static SeqKey *_seq_keymap;
    static Track *_trackmap;
    static uint8_t _keymap_len;
    static uint8_t _simul_keymap_len;
    static uint8_t _seq_keymap_len;
    static uint8_t _trackmap_len;

    enum class SeqModeState
    {
      Disable,
      Triggered,
      Running,
      WaitRelease,
    };

    static SeqModeState _seq_mode_state;
    static void switchSequenceMode();

    static LinkedList<Tracking *> _tracking_list;
    static int32_t _distance_x;
    static int32_t _distance_y;
    static void startTracking(HidEngineClass::Tracking *tracking);
    static void stopTracking(HidEngineClass::Tracking *tracking);
  };

  extern HidEngineClass HidEngine;

  //------------------------------------------------------------------+
  // short name inner command
  //------------------------------------------------------------------+

  // Sequence Mode
  static inline Command *SEQ_MODE() { return (new HidEngineClass::SequenceMode); }
  // Track
  static inline Command *TRC(uint8_t track_id) { return (new HidEngineClass::Tracking(track_id)); }
  // Track or Tap
  static inline Command *TRT(uint8_t track_id, Command *command) { return (new HidEngineClass::TrackTap(track_id, command)); }

} // namespace hidpg
