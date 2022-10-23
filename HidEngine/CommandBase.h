/*
  The MIT License (MIT)

  Copyright (c) 2022 ogatatsu.

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

#include "etl/intrusive_links.h"
#include "etl/intrusive_list.h"
#include "etl/optional.h"
#include "gsl/gsl-lite.hpp"

namespace hidpg::Internal
{
  using BeforeOtherKeyPressEventListenerLink = etl::bidirectional_link<0>;
  using BeforeMovePointerEventListenerLink = etl::bidirectional_link<1>;
  using BeforeRotateEncoderEventListenerLink = etl::bidirectional_link<2>;
  using BeforeGestureEventListenerLink = etl::bidirectional_link<3>;
}

namespace hidpg
{

  struct KeyShiftId
  {
    uint8_t value;

    bool operator==(const KeyShiftId &rhs) const { return value == rhs.value; }
    bool operator!=(const KeyShiftId &rhs) const { return value != rhs.value; }
  };

  struct PointingDeviceId
  {
    uint8_t value;

    bool operator==(const PointingDeviceId &rhs) const { return value == rhs.value; }
    bool operator!=(const PointingDeviceId &rhs) const { return value != rhs.value; }
  };

  struct GestureId
  {
    uint8_t value;

    bool operator==(const GestureId &rhs) const { return value == rhs.value; }
    bool operator!=(const GestureId &rhs) const { return value != rhs.value; }
  };

  struct EncoderId
  {
    uint8_t value;

    bool operator==(const EncoderId &rhs) const { return value == rhs.value; }
    bool operator!=(const EncoderId &rhs) const { return value != rhs.value; }
  };

  struct EncoderShiftId
  {
    uint8_t value;

    bool operator==(const EncoderShiftId &rhs) const { return value == rhs.value; }
    bool operator!=(const EncoderShiftId &rhs) const { return value != rhs.value; }
  };

  //------------------------------------------------------------------+
  // Command
  //------------------------------------------------------------------+
  class Command
  {
  public:
    Command();
    void press(uint8_t n_times = 1);
    uint8_t release();

    virtual void setKeyId(uint8_t key_id) { _key_id = key_id; }
    etl::optional<uint8_t> getKeyId() { return _key_id; }

  protected:
    virtual void onPress(uint8_t n_times) {}
    virtual uint8_t onRelease() { return 1; }

  private:
    etl::optional<uint8_t> _key_id;
    bool _is_pressed;
  };

  using CommandPtr = Command *;
  using NotNullCommandPtr = gsl::not_null<Command *>;

  //------------------------------------------------------------------+
  // BeforeKeyPressEventListener
  //------------------------------------------------------------------+
  class BeforeOtherKeyPressEventListener : public Internal::BeforeOtherKeyPressEventListenerLink
  {
  public:
    BeforeOtherKeyPressEventListener(Command *command);
    static void _notifyBeforeOtherKeyPress(uint8_t key_id);

  protected:
    bool startListenBeforeOtherKeyPress();
    bool stopListenBeforeOtherKeyPress();
    virtual void onBeforeOtherKeyPress(uint8_t key_id) = 0;

  private:
    using List = etl::intrusive_list<BeforeOtherKeyPressEventListener, Internal::BeforeOtherKeyPressEventListenerLink>;

    // keymap(グローバル変数)の定義で特定のコマンドがnewされたときにコンストラクタ内でstartListenBeforeOtherCommandPress()が呼ばれる、
    // _listener_listはその内部で使用するので単純なstatic変数にすると初期化順序が問題となる可能性がある。
    // https://isocpp.org/wiki/faq/ctors#static-init-order-on-first-use
    static List &_listener_list()
    {
      static List list;
      return list;
    };

    Command *_command;
    bool _is_listen;
  };

  //------------------------------------------------------------------+
  // BeforeMovePointerEventListener
  //------------------------------------------------------------------+
  class BeforeMovePointerEventListener : public Internal::BeforeMovePointerEventListenerLink
  {
  public:
    BeforeMovePointerEventListener();
    static void _notifyBeforeMovePointer(PointingDeviceId pointing_device_id, int16_t delta_x, int16_t delta_y);

  protected:
    bool startListenBeforeMovePointer();
    bool stopListenBeforeMovePointer();
    virtual void onBeforeMovePointer(PointingDeviceId pointing_device_id, int16_t delta_x, int16_t delta_y) = 0;

  private:
    using List = etl::intrusive_list<BeforeMovePointerEventListener, Internal::BeforeMovePointerEventListenerLink>;

    // Construct On First Use Idiom
    static List &_listener_list()
    {
      static List list;
      return list;
    };

    bool _is_listen;
  };

  //------------------------------------------------------------------+
  // BeforeRotateEncoderEventListener
  //------------------------------------------------------------------+
  class BeforeRotateEncoderEventListener : public Internal::BeforeRotateEncoderEventListenerLink
  {
  public:
    BeforeRotateEncoderEventListener();
    static void _notifyBeforeRotateEncoder(EncoderId encoder_id, int16_t step);

  protected:
    bool startListenBeforeRotateEncoder();
    bool stopListenBeforeRotateEncoder();
    virtual void onBeforeRotateEncoder(EncoderId encoder_id, int16_t step) = 0;

  private:
    using List = etl::intrusive_list<BeforeRotateEncoderEventListener, Internal::BeforeRotateEncoderEventListenerLink>;

    // Construct On First Use Idiom
    static List &_listener_list()
    {
      static List list;
      return list;
    };

    bool _is_listen;
  };

  //------------------------------------------------------------------+
  // BeforeGestureEventListener
  //------------------------------------------------------------------+
  class BeforeGestureEventListener : public Internal::BeforeGestureEventListenerLink
  {
  public:
    BeforeGestureEventListener();
    static void _notifyBeforeGesture(GestureId gesture_id, PointingDeviceId pointing_device_id);

  protected:
    bool startListenBeforeGesture();
    bool stopListenBeforeGesture();
    virtual void onBeforeGesture(GestureId gesture_id, PointingDeviceId pointing_device_id) = 0;

  private:
    using List = etl::intrusive_list<BeforeGestureEventListener, Internal::BeforeGestureEventListenerLink>;

    // Construct On First Use Idiom
    static List &_listener_list()
    {
      static List list;
      return list;
    };

    bool _is_listen;
  };

} // namespace hidpg
