#include "RotaryEncoder.h"

namespace hidpg
{

  RotaryEncoder::RotaryEncoder(uint16_t pin_a,
                               uint16_t pin_b,
                               TaskHandle_t &task_handle,
                               voidFuncPtr interrupt_callback,
                               SimpleHacks::QDecoder &qdec,
                               int32_t &step,
                               bool &needs_call)
      : _task_handle(task_handle),
        _interrupt_callback(interrupt_callback),
        _qdec(qdec),
        _step(step),
        _needs_call(needs_call),
        _callback(nullptr)
  {
    _qdec.setPinA(pin_a);
    _qdec.setPinB(pin_b);
  }

  void RotaryEncoder::useFullStep(bool is_full_step)
  {
    _qdec.setFullStep(is_full_step);
  }

  void RotaryEncoder::start()
  {
    if (_qdec.getIsStarted())
    {
      return;
    }

    _qdec.begin();

    attachInterrupt(digitalPinToInterrupt(_qdec.getPinA()), _interrupt_callback, CHANGE);
    attachInterrupt(digitalPinToInterrupt(_qdec.getPinB()), _interrupt_callback, CHANGE);

    if (_task_handle == nullptr)
    {
      _task_handle = xTaskCreateStatic(task, "RotaryEncoder", ROTARY_ENCODER_TASK_STACK_SIZE, this, ROTARY_ENCODER_TASK_PRIO, _task_stack, &_task_tcb);
    }
    else
    {
      vTaskResume(_task_handle);
    }
  }

  void RotaryEncoder::setCallback(callback_t cb)
  {
    _callback = cb;
  }

  int32_t RotaryEncoder::readStep()
  {
    taskENTER_CRITICAL();
    int32_t result = _step;
    _step = 0;
    _needs_call = true;
    taskEXIT_CRITICAL();

    return result;
  }

  void RotaryEncoder::stop()
  {
    if (_qdec.getIsStarted() == false)
    {
      return;
    }

    vTaskSuspend(_task_handle);

    detachInterrupt(digitalPinToInterrupt(_qdec.getPinA()));
    detachInterrupt(digitalPinToInterrupt(_qdec.getPinB()));

    pinMode(_qdec.getPinA(), INPUT);
    pinMode(_qdec.getPinB(), INPUT);

    _qdec.end();
  }

  void RotaryEncoder::task(void *pvParameters)
  {
    RotaryEncoder *that = static_cast<RotaryEncoder *>(pvParameters);

    while (true)
    {
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
      if (that->_callback != nullptr)
      {
        that->_callback();
      }
    }
  }

} // namespace hidpg
