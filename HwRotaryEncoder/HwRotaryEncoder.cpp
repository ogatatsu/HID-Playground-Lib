/*
  The MIT License (MIT)

  Copyright (c) 2020 ogatatsu.

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

#include "HwRotaryEncoder.h"
#include "HwRotaryEncoder_config.h"
#include "nrf_gpio.h"
#include "nrf_qdec.h"

namespace hidpg
{
  namespace Internal
  {

    HwRotaryEncoderClass::callback_t HwRotaryEncoderClass::_cb = nullptr;

    void HwRotaryEncoderClass::begin(uint8_t pina, uint8_t pinb)
    {
      pinMode(pina, INPUT_PULLUP);
      pinMode(pinb, INPUT_PULLUP);

      nrf_qdec_pio_assign(NRF_QDEC,
                          g_ADigitalPinMap[pina],
                          g_ADigitalPinMap[pinb],
                          NRF_QDEC_LED_NOT_CONNECTED);

      nrf_qdec_sampleper_set(NRF_QDEC, HW_ROTARY_ENCODER_SAMPLEPER);
      nrf_qdec_reportper_set(NRF_QDEC, HW_ROTARY_ENCODER_REPORTPER);

#if HW_ROTARY_ENCODER_DEBOUNCE_FILTER_ENABLE
      nrf_qdec_dbfen_enable(NRF_QDEC);
#else
      nrf_qdec_dbfen_disable(NRF_QDEC);
#endif

      // IRQ Enable
      NVIC_EnableIRQ(QDEC_IRQn);
      // QDEC Enable
      nrf_qdec_enable(NRF_QDEC);
      // QDEC Start
      nrf_qdec_task_trigger(NRF_QDEC, NRF_QDEC_TASK_START);
    }

    void HwRotaryEncoderClass::stopTask_and_setWakeUpInterrupt()
    {
      // IRQ Disable
      NVIC_DisableIRQ(QDEC_IRQn);
      // QDEC Stop
      nrf_qdec_task_trigger(NRF_QDEC, NRF_QDEC_TASK_STOP);
      // QDEC Disable
      nrf_qdec_disable(NRF_QDEC);
      // Set Sense
      int astate = nrf_gpio_pin_read(NRF_QDEC->PSELA);
      nrf_gpio_pin_sense_t sense = astate ? NRF_GPIO_PIN_SENSE_LOW : NRF_GPIO_PIN_SENSE_HIGH;
      nrf_gpio_cfg_sense_input(NRF_QDEC->PSELA, NRF_GPIO_PIN_PULLUP, sense);
    }

    void HwRotaryEncoderClass::setCallback(callback_t fp)
    {
      _cb = fp;
      if (_cb != nullptr)
      {
        nrf_qdec_event_clear(NRF_QDEC, NRF_QDEC_EVENT_REPORTRDY);
        nrf_qdec_int_enable(NRF_QDEC, QDEC_INTENSET_REPORTRDY_Msk);
      }
    }

    int32_t HwRotaryEncoderClass::readStep()
    {
      nrf_qdec_task_trigger(NRF_QDEC, NRF_QDEC_TASK_READCLRACC);
      if (_cb != nullptr)
      {
        nrf_qdec_event_clear(NRF_QDEC, NRF_QDEC_EVENT_REPORTRDY);
        nrf_qdec_int_enable(NRF_QDEC, QDEC_INTENSET_REPORTRDY_Msk);
      }
      return nrf_qdec_accread_get(NRF_QDEC);
    }

    void HwRotaryEncoderClass::_irq_handler()
    {
      if (nrf_qdec_event_check(NRF_QDEC, NRF_QDEC_EVENT_REPORTRDY))
      {
        nrf_qdec_event_clear(NRF_QDEC, NRF_QDEC_EVENT_REPORTRDY);
        // One Shot
        nrf_qdec_int_disable(NRF_QDEC, QDEC_INTENSET_REPORTRDY_Msk);
        if (_cb != nullptr)
        {
          ada_callback(NULL, 0, _cb);
        }
      }
    }

    extern "C"
    {
      void QDEC_IRQHandler()
      {
        HwRotaryEncoderClass::_irq_handler();
      }
    }

  } // namespace Internal

  Internal::HwRotaryEncoderClass HwRotaryEncoder;

} // namespace hidpg
