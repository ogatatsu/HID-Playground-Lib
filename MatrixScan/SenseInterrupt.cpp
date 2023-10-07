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

#include "MatrixScan_config.h"

#if MATRIX_SCAN_USE_NRF52_SENSE_INTERRUPT

#include "SenseInterrupt.h"
#include "nrf_egu.h"
#include "nrf_gpiote.h"
#include "nrf_ppi.h"

namespace hidpg::Internal
{

  static voidFuncPtr senseCallback = NULL;
  static bool enabled = false;

  static void init()
  {
    NVIC_DisableIRQ(GPIOTE_IRQn);
    NVIC_ClearPendingIRQ(GPIOTE_IRQn);
    NVIC_SetPriority(GPIOTE_IRQn, 3);
    NVIC_EnableIRQ(GPIOTE_IRQn);

    NVIC_DisableIRQ(SWI3_EGU3_IRQn);
    NVIC_ClearPendingIRQ(SWI3_EGU3_IRQn);
    NVIC_SetPriority(SWI3_EGU3_IRQn, 3);
    NVIC_EnableIRQ(SWI3_EGU3_IRQn);
  }

  void attachSenseInterrupt(voidFuncPtr callback)
  {
    if (!enabled)
    {
      init();
      enabled = true;
    }

    senseCallback = callback;

    nrf_ppi_channel_endpoint_setup(
        NRF_PPI,
        NRF_PPI_CHANNEL0,
        nrf_gpiote_event_address_get(NRF_GPIOTE, NRF_GPIOTE_EVENT_PORT),
        nrf_egu_task_address_get(NRF_EGU3, NRF_EGU_TASK_TRIGGER0));

    nrf_ppi_channel_enable(NRF_PPI, NRF_PPI_CHANNEL0);

    nrf_egu_event_clear(NRF_EGU3, NRF_EGU_EVENT_TRIGGERED0);
    nrf_egu_int_enable(NRF_EGU3, EGU_INTENSET_TRIGGERED0_Msk);

    nrf_gpiote_event_clear(NRF_GPIOTE, NRF_GPIOTE_EVENT_PORT);
    nrf_gpiote_int_enable(NRF_GPIOTE, GPIOTE_INTENSET_PORT_Msk);
  }

  void detachSenseInterrupt()
  {
    nrf_gpiote_int_disable(NRF_GPIOTE, GPIOTE_INTENCLR_PORT_Msk);
    nrf_egu_int_disable(NRF_EGU3, EGU_INTENCLR_TRIGGERED0_Msk);
    nrf_ppi_channel_disable(NRF_PPI, NRF_PPI_CHANNEL0);
    nrf_ppi_channel_endpoint_setup(NRF_PPI, NRF_PPI_CHANNEL0, 0, 0);

    senseCallback = NULL;
  }

  extern "C"
  {
    void SWI3_EGU3_IRQHandler()
    {
      if (nrf_gpiote_event_check(NRF_GPIOTE, NRF_GPIOTE_EVENT_PORT))
      {
        if (senseCallback != NULL)
        {
          senseCallback();
        }
        nrf_gpiote_event_clear(NRF_GPIOTE, NRF_GPIOTE_EVENT_PORT);
      }

      if (nrf_egu_event_check(NRF_EGU3, NRF_EGU_EVENT_TRIGGERED0))
      {
        nrf_egu_event_clear(NRF_EGU3, NRF_EGU_EVENT_TRIGGERED0);
      }
    }
  }

  extern "C"
  {
    __attribute__((weak)) void GPIOTE_IRQHandler() {}
  }

} // namespace hidpg::Internal

#endif
