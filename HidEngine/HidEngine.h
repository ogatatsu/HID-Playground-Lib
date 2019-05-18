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

struct ID_and_Command
{
  uint8_t id;
  Command *command;
};

struct SimulIDs_and_Command
{
  uint8_t ids[MAX_SIMUL_PRESS_COUNT];
  Command *command;
  size_t idsLength;
};

struct SeqIDs_and_Command
{
  uint8_t ids[MAX_SEQ_COUNT];
  Command *command;
  size_t idsLength;
};

struct TrackID_and_Command
{
  uint8_t trackID;
  uint16_t distance;
  Command *upCommand;
  Command *downCommand;
  Command *leftCommand;
  Command *rightCommand;
};

/*------------------------------------------------------------------*/
/*  define special command
 *------------------------------------------------------------------*/
class SequenceMode : public Command
{
protected:
  uint8_t onPress(uint8_t accrued) override;
};

/*------------------------------------------------------------------*/
class Tracking : public Command
{
public:
  Tracking(uint8_t id);
  uint8_t getID();

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  uint8_t _id;
};

/*------------------------------------------------------------------*/
class TrackTap : public Tracking
{
public:
  TrackTap(uint8_t id, Command *command);

protected:
  uint8_t onPress(uint8_t accrued) override;
  void onRelease() override;

private:
  Command *_command;
};

/*------------------------------------------------------------------*/
/*  define HidEngine class
 *------------------------------------------------------------------*/
class HidEngine
{
  friend class HidEngineTask;
  friend class SequenceMode;
  friend class Tracking;

public:
  template <uint8_t keymapLength>
  static void setKeymap(ID_and_Command (&keymap)[keymapLength])
  {
    _keymap = keymap;
    _keymapLength = keymapLength;
  }

  template <uint8_t simulKeymapLength>
  static void setSimulKeymap(SimulIDs_and_Command (&simulKeymap)[simulKeymapLength])
  {
    _simulKeymap = simulKeymap;
    _simulKeymapLength = simulKeymapLength;

    for (int i = 0; i < _simulKeymapLength; i++)
    {
      _simulKeymap[i].idsLength = getValidLength(_simulKeymap[i].ids, MAX_SIMUL_PRESS_COUNT);
    }
  }

  template <uint8_t seqKeymapLength>
  static void setSeqKeymap(SeqIDs_and_Command (&seqKeymap)[seqKeymapLength])
  {
    _seqKeymap = seqKeymap;
    _seqKeymapLength = seqKeymapLength;

    for (int i = 0; i < _seqKeymapLength; i++)
    {
      _seqKeymap[i].idsLength = getValidLength(_seqKeymap[i].ids, MAX_SEQ_COUNT);
    }
  }

  template <uint8_t trackmapLength>
  static void setTrackmap(TrackID_and_Command (&trackmap)[trackmapLength])
  {
    _trackmap = trackmap;
    _trackmapLength = trackmapLength;
  }

  static void init(HidReporter *hidReporter);
  static void startTask();
  static void applyToKeymap(const Set &ids);
  static void mouseMove(int8_t x, int8_t y);

private:
  static void applyToKeymap_impl(const Set &ids);
  static void mouseMove_impl(int8_t x, int8_t y);
  static int match_with_seqKeymap(const uint8_t ids[], size_t len, SeqIDs_and_Command **matched);
  static size_t getValidLength(const uint8_t ids[], size_t maxLength);

  static ID_and_Command *_keymap;
  static SimulIDs_and_Command *_simulKeymap;
  static SeqIDs_and_Command *_seqKeymap;
  static TrackID_and_Command *_trackmap;
  static uint8_t _keymapLength;
  static uint8_t _simulKeymapLength;
  static uint8_t _seqKeymapLength;
  static uint8_t _trackmapLength;

  enum class SeqModeState
  {
    Disable,
    Triggered,
    Running,
    WaitRelease,
  };

  static SeqModeState _seqModeState;
  static void switchSequenceMode();

  static LinkedList<Tracking *> _trackingList;
  static int _distanceX;
  static int _distanceY;
  static void startTracking(Tracking *tracking);
  static void stopTracking(Tracking *tracking);
};

/*------------------------------------------------------------------*/
/*  define short name special command
 *------------------------------------------------------------------*/
// Sequence Mode
#define SEQ_MODE (static_cast<Command *>(new SequenceMode))
// Track
static inline Command *TRC(uint8_t trackID) { return (new Tracking(trackID)); }
// Track or Tap
static inline Command *TRT(uint8_t trackID, Command *command) { return (new TrackTap(trackID, command)); }

} // namespace hidpg
