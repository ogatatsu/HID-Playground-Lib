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

#include "HidEngine.h"
#include "KeyCode.h"
#include "Layer.h"
#include "etl/variant.h"

namespace hidpg::Internal
{

  //------------------------------------------------------------------+
  // NormalKey
  //------------------------------------------------------------------+
  class NormalKey : public Command
  {
  public:
    NormalKey(KeyCode key_code);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const KeyCode _key_code;
  };

  //------------------------------------------------------------------+
  // ModifierKey
  //------------------------------------------------------------------+
  class ModifierKey : public Command
  {
  public:
    ModifierKey(Modifiers modifiers);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const Modifiers _modifiers;
  };

  //------------------------------------------------------------------+
  // CombinationKey
  //------------------------------------------------------------------+
  class CombinationKey : public Command
  {
  public:
    CombinationKey(Modifiers modifiers, KeyCode key_code);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const Modifiers _modifiers;
    const KeyCode _key_code;
  };

  //------------------------------------------------------------------+
  // Layering
  //------------------------------------------------------------------+
  class Layering : public Command
  {
  public:
    Layering(LayerClass &layer, etl::span<CommandPtr> commands);
    void setKeyId(uint8_t) override;

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    LayerClass &_layer;
    const etl::span<CommandPtr> _commands;
    Command *_running_command;
  };

  //------------------------------------------------------------------+
  // ToggleLayer
  //------------------------------------------------------------------+
  class ToggleLayer : public Command
  {
  public:
    ToggleLayer(LayerClass &layer, uint8_t layer_number);

  protected:
    void onPress(uint8_t n_times) override;

  private:
    LayerClass &_layer;
    const uint8_t _layer_number;
  };

  //------------------------------------------------------------------+
  // SwitchLayer
  //------------------------------------------------------------------+
  class SwitchLayer : public Command
  {
  public:
    SwitchLayer(LayerClass &layer, uint8_t layer_number);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    LayerClass &_layer;
    const uint8_t _layer_number;
  };

  //------------------------------------------------------------------+
  // UpDefaultLayer
  //------------------------------------------------------------------+
  class UpDefaultLayer : public Command
  {
  public:
    UpDefaultLayer(LayerClass &layer, uint8_t i);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    LayerClass &_layer;
    const uint8_t _i;
  };

  // ------------------------------------------------------------------+
  // Tap
  // ------------------------------------------------------------------+
  class Tap : public Command
  {
  public:
    Tap(NotNullCommandPtr command, uint8_t n_times, uint16_t tap_speed_ms);
    void setKeyId(uint8_t) override;

  protected:
    void onPress(uint8_t n_times) override;

  private:
    const NotNullCommandPtr _command;
    const uint8_t _n_times;
    const uint16_t _tap_speed_ms;
  };

  // ------------------------------------------------------------------+
  // TapWhenReleased
  // ------------------------------------------------------------------+
  class TapWhenReleased : public Command
  {
  public:
    TapWhenReleased(NotNullCommandPtr command, uint8_t n_times, uint16_t tap_speed_ms);
    void setKeyId(uint8_t) override;

  protected:
    uint8_t onRelease() override;

  private:
    const NotNullCommandPtr _command;
    const uint8_t _n_times;
    const uint16_t _tap_speed_ms;
  };

  //------------------------------------------------------------------+
  // TapDance
  //------------------------------------------------------------------+
  class TapDance : public Command,
                   public TimerMixin,
                   public BeforeOtherKeyPressEventListener,
                   public BeforeMovePointerEventListener,
                   public BeforeRotateEncoderEventListener
  {
  public:
    struct Pair
    {
      Pair(NotNullCommandPtr hold_command, CommandPtr tap_command = nullptr) : hold_command(hold_command), tap_command(tap_command) {}
      const NotNullCommandPtr hold_command;
      const CommandPtr tap_command;
    };

    TapDance(etl::span<Pair> pairs, etl::span<PointingDeviceId> pointing_device_ids, uint16_t move_threshold);
    void setKeyId(uint8_t) override;

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;
    void onTimer() override;
    void onBeforeOtherKeyPress(uint8_t key_id) override;
    void onBeforeMovePointer(PointingDeviceId pointing_device_id, int16_t delta_x, int16_t delta_y) override;
    void onBeforeRotateEncoder(EncoderId encoder_id, int16_t step) override;

  private:
    struct BeforeOtherKeyPressArgs
    {
      const uint8_t key_id;
    };

    struct BeforeMovePointerArgs
    {
      const PointingDeviceId pointing_device_id;
      const int16_t delta_x;
      const int16_t delta_y;
    };

    struct BeforeRotateEncoderArgs
    {
      const EncoderId encoder_id;
      const int16_t step;
    };

    using ArgsType = etl::variant<BeforeOtherKeyPressArgs, BeforeMovePointerArgs, BeforeRotateEncoderArgs, nullptr_t>;

    // clang-format off
      enum class Action : uint32_t
      {
        Press                   = 0b1,
        Release                 = 0b10,
        Timer                   = 0b100,
        BeforeOtherKeyPress     = 0b1000,
        BeforeMouseMove         = 0b10000,
        BeforeRotateEncoder     = 0b100000,
      };

      enum class State : uint32_t
      {
        Unexecuted              = 0b1000000,
        Pressed                 = 0b10000000,
        TapOrNextCommand        = 0b100000000,
        DecidedToHold           = 0b1000000000,
      };
    // clang-format on

    static constexpr uint32_t Context(Action action, State state)
    {
      return static_cast<uint32_t>(static_cast<uint32_t>(action) | static_cast<uint32_t>(state));
    }

    void performTap();
    void performHoldPress();
    void performHoldRelease();
    bool checkMoveThreshold(BeforeMovePointerArgs &args);
    void processTapDance(Action action, ArgsType args);

    Command *_running_command;
    Command *_hooked_command;
    const etl::span<Pair> _pairs;
    const etl::span<PointingDeviceId> _pointing_device_ids;
    const uint16_t _move_threshold;
    int16_t _delta_x_sum;
    int16_t _delta_y_sum;
    size_t _idx_count;
    State _state;
  };

  //------------------------------------------------------------------+
  // TapOrHold
  //------------------------------------------------------------------+
  class TapOrHold : public Command, public TimerMixin
  {
  public:
    TapOrHold(NotNullCommandPtr tap_command, unsigned int ms, NotNullCommandPtr hold_command);
    void setKeyId(uint8_t) override;

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;
    void onTimer() override;

  private:
    enum class State : uint8_t
    {
      Unexecuted,
      Pressed,
      FixedToHold,
    };

    const NotNullCommandPtr _tap_command;
    const NotNullCommandPtr _hold_command;
    const unsigned int _ms;
    State _state;
  };

  //------------------------------------------------------------------+
  // ConsumerControl
  //------------------------------------------------------------------+
  class ConsumerControl : public Command
  {
  public:
    ConsumerControl(ConsumerControlCode usage_code);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const ConsumerControlCode _usage_code;
  };

  //------------------------------------------------------------------+
  // SystemControl
  //------------------------------------------------------------------+
  class SystemControl : public Command
  {
  public:
    SystemControl(SystemControlCode usage_code);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const SystemControlCode _usage_code;
  };

  //------------------------------------------------------------------+
  // MouseMove
  //------------------------------------------------------------------+
  class MouseMove : public Command
  {
  public:
    MouseMove(int16_t x, int16_t y);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const int16_t _x;
    const int16_t _y;
    const uint8_t _max_n_times;
    uint8_t _actual_n_times;
  };

  //------------------------------------------------------------------+
  // MouseScroll
  //------------------------------------------------------------------+
  class MouseScroll : public Command
  {
  public:
    MouseScroll(int8_t scroll, int8_t horiz);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const int8_t _scroll;
    const int8_t _horiz;
    const uint8_t _max_n_times;
    uint8_t _actual_n_times;
  };

  //------------------------------------------------------------------+
  // MouseClick
  //------------------------------------------------------------------+
  class MouseClick : public Command
  {
  public:
    MouseClick(MouseButtons buttons);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const MouseButtons _buttons;
  };

  //------------------------------------------------------------------+
  // RadialClick
  //------------------------------------------------------------------+
  class RadialClick : public Command
  {
  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;
  };

  //------------------------------------------------------------------+
  // RadialRotate
  //------------------------------------------------------------------+
  class RadialRotate : public Command
  {
  public:
    RadialRotate(int16_t deci_degree);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const int16_t _deci_degree;
    const uint8_t _max_n_times;
    uint8_t _actual_n_times;
  };

  //------------------------------------------------------------------+
  // OnceEvery
  //------------------------------------------------------------------+
  class OnceEvery : public Command
  {
  public:
    OnceEvery(NotNullCommandPtr command, uint32_t ms);
    void setKeyId(uint8_t) override;

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const NotNullCommandPtr _command;
    const uint32_t _ms;
    uint32_t _last_press_millis;
    bool _has_pressed;
  };

  //------------------------------------------------------------------+
  // NTimesEvery
  //------------------------------------------------------------------+
  class NTimesEvery : public Command
  {
  public:
    NTimesEvery(NotNullCommandPtr command, uint32_t _ms);
    void setKeyId(uint8_t) override;

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const NotNullCommandPtr _command;
    const uint32_t _ms;
    uint32_t _last_press_millis;
    bool _has_pressed;
  };

  //------------------------------------------------------------------+
  // If
  //------------------------------------------------------------------+
  class If : public Command
  {
  public:
    If(bool (*func)(), NotNullCommandPtr true_command, NotNullCommandPtr false_command);
    void setKeyId(uint8_t) override;

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    bool (*const _func)();
    const NotNullCommandPtr _true_command;
    const NotNullCommandPtr _false_command;
    Command *_running_command;
  };

  //------------------------------------------------------------------+
  // Multi
  //------------------------------------------------------------------+
  class Multi : public Command
  {
  public:
    Multi(etl::span<NotNullCommandPtr> commands);
    void setKeyId(uint8_t key_id) override;

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const etl::span<NotNullCommandPtr> _commands;
  };

  //------------------------------------------------------------------+
  // Toggle
  //------------------------------------------------------------------+
  class Toggle : public Command
  {
  public:
    Toggle(NotNullCommandPtr command);
    void setKeyId(uint8_t) override;

  protected:
    void onPress(uint8_t n_times) override;

  private:
    const NotNullCommandPtr _command;
    bool _is_pressed;
  };

  //------------------------------------------------------------------+
  // Repeat
  //------------------------------------------------------------------+
  class Repeat : public Command, public TimerMixin
  {
  public:
    Repeat(NotNullCommandPtr command, uint32_t delay_ms, uint32_t interval_ms);
    void setKeyId(uint8_t) override;

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;
    void onTimer() override;

  private:
    const NotNullCommandPtr _command;
    const uint32_t _delay_ms;
    const uint32_t _interval_ms;
  };

  //------------------------------------------------------------------+
  // Cycle
  //------------------------------------------------------------------+
  class Cycle : public Command
  {
  public:
    Cycle(etl::span<NotNullCommandPtr> commands);
    void setKeyId(uint8_t) override;

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const etl::span<NotNullCommandPtr> _commands;
    size_t _idx;
  };

  //------------------------------------------------------------------+
  // CyclePhaseShift
  //------------------------------------------------------------------+
  class CyclePhaseShift : public Command
  {
  public:
    CyclePhaseShift(etl::span<NotNullCommandPtr> commands);
    void setKeyId(uint8_t) override;

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const etl::span<NotNullCommandPtr> _commands;
    size_t _idx;
  };

  //------------------------------------------------------------------+
  // NoOperation
  //------------------------------------------------------------------+
  class NoOperation : public Command
  {

  protected:
    uint8_t onRelease() override;
  };

  //------------------------------------------------------------------+
  // Shift
  //------------------------------------------------------------------+
  using ShiftId = etl::variant<KeyShiftId, EncoderShiftId, GestureId>;
  using ShiftIdLink = etl::variant<KeyShiftIdLink, EncoderShiftIdLink, GestureIdLink>;

  struct ShiftIds1
  {
    ShiftIds1(ShiftId shift_id0)
        : id0(shift_id0) {}

    ShiftId id0;
  };

  struct ShiftIds2
  {
    ShiftIds2(ShiftId shift_id0,
              ShiftId shift_id1)
        : id0(shift_id0),
          id1(shift_id1) {}

    ShiftId id0;
    ShiftId id1;
  };

  struct ShiftIds3
  {
    ShiftIds3(ShiftId shift_id0,
              ShiftId shift_id1,
              ShiftId shift_id2)
        : id0(shift_id0),
          id1(shift_id1),
          id2(shift_id2) {}

    ShiftId id0;
    ShiftId id1;
    ShiftId id2;
  };

  struct ShiftIds4
  {
    ShiftIds4(ShiftId shift_id0,
              ShiftId shift_id1,
              ShiftId shift_id2,
              ShiftId shift_id3)
        : id0(shift_id0),
          id1(shift_id1),
          id2(shift_id2),
          id3(shift_id3) {}

    ShiftId id0;
    ShiftId id1;
    ShiftId id2;
    ShiftId id3;
  };

  struct ShiftIds5
  {
    ShiftIds5(ShiftId shift_id0,
              ShiftId shift_id1,
              ShiftId shift_id2,
              ShiftId shift_id3,
              ShiftId shift_id4)
        : id0(shift_id0),
          id1(shift_id1),
          id2(shift_id2),
          id3(shift_id3),
          id4(shift_id4) {}

    ShiftId id0;
    ShiftId id1;
    ShiftId id2;
    ShiftId id3;
    ShiftId id4;
  };

  class Shift : public Command
  {
  public:
    Shift(etl::span<ShiftIdLink> shift_id_links);
    static ShiftIdLink IdToLink(ShiftId shift_id);

  protected:
    void onPress(uint8_t n_times) override;
    uint8_t onRelease() override;

  private:
    const etl::span<ShiftIdLink> _shift_id_links;
  };

} // namespace hidpg::Internal
