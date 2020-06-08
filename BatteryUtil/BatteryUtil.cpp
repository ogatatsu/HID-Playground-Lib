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

/*
  analogReadVdd() function was written with reference to this example.
  https://github.com/andenore/NordicSnippets/tree/master/examples/saadc

  Copyright (c) 2015, Anders Nore
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the name of NordicSnippets nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "BatteryUtil.h"
#include "Arduino.h"
#include "BatteryUtil_config.h"

namespace hidpg
{

  // 1/6 gain (GND ~ 3.6V) and 14bit (0 ~ 16383)
  static const long MIN_ANALOG_VALUE = MIN_BATTERY_VOLTAGE / 3.6 * 16383;
  static const long MAX_ANALOG_VALUE = MAX_BATTERY_VOLTAGE / 3.6 * 16383;

  static uint16_t analogReadVdd()
  {
    volatile uint16_t result = 0;

    // Configure SAADC singled-ended channel, Internal reference (0.6V) and 1/6 gain.
    NRF_SAADC->CH[0].CONFIG = (SAADC_CH_CONFIG_GAIN_Gain1_6 << SAADC_CH_CONFIG_GAIN_Pos) |
                              (SAADC_CH_CONFIG_MODE_SE << SAADC_CH_CONFIG_MODE_Pos) |
                              (SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_REFSEL_Pos) |
                              (SAADC_CH_CONFIG_RESN_Bypass << SAADC_CH_CONFIG_RESN_Pos) |
                              (SAADC_CH_CONFIG_RESP_Bypass << SAADC_CH_CONFIG_RESP_Pos) |
                              (SAADC_CH_CONFIG_TACQ_3us << SAADC_CH_CONFIG_TACQ_Pos);

    // Configure the SAADC channel with VDD as positive input, no negative input(single ended).
    NRF_SAADC->CH[0].PSELP = SAADC_CH_PSELP_PSELP_VDD << SAADC_CH_PSELP_PSELP_Pos;
    NRF_SAADC->CH[0].PSELN = SAADC_CH_PSELN_PSELN_NC << SAADC_CH_PSELN_PSELN_Pos;

    // Configure the SAADC resolution.
    NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_14bit << SAADC_RESOLUTION_VAL_Pos;

    // Configure result to be put in RAM at the location of "result" variable.
    NRF_SAADC->RESULT.MAXCNT = 1;
    NRF_SAADC->RESULT.PTR = (uint32_t)&result;

    // No automatic sampling, will trigger with TASKS_SAMPLE.
    NRF_SAADC->SAMPLERATE = SAADC_SAMPLERATE_MODE_Task << SAADC_SAMPLERATE_MODE_Pos;

    // Enable SAADC (would capture analog pins if they were used in CH[0].PSELP)
    NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos;

    // Calibrate the SAADC (only needs to be done once in a while)
    NRF_SAADC->TASKS_CALIBRATEOFFSET = 1;
    while (NRF_SAADC->EVENTS_CALIBRATEDONE == 0)
      ;
    NRF_SAADC->EVENTS_CALIBRATEDONE = 0;
    while (NRF_SAADC->STATUS == (SAADC_STATUS_STATUS_Busy << SAADC_STATUS_STATUS_Pos))
      ;

    // Start the SAADC and wait for the started event.
    NRF_SAADC->TASKS_START = 1;
    while (NRF_SAADC->EVENTS_STARTED == 0)
      ;
    NRF_SAADC->EVENTS_STARTED = 0;

    // Do a SAADC sample, will put the result in the configured RAM buffer.
    NRF_SAADC->TASKS_SAMPLE = 1;
    while (NRF_SAADC->EVENTS_END == 0)
      ;
    NRF_SAADC->EVENTS_END = 0;

    // Stop the SAADC, since it's not used anymore.
    NRF_SAADC->TASKS_STOP = 1;
    while (NRF_SAADC->EVENTS_STOPPED == 0)
      ;
    NRF_SAADC->EVENTS_STOPPED = 0;

    NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Disabled << SAADC_ENABLE_ENABLE_Pos;

    return result;
  }

  uint8_t BatteryUtil_::readBatteryLevel()
  {
#ifdef BATTERY_READ_PIN
    analogReference(AR_INTERNAL);
    analogReadResolution(14);
    uint16_t val = analogRead(BATTERY_READ_PIN);
#else
    uint16_t val = analogReadVdd();
#endif

    int level = map(val, MIN_ANALOG_VALUE, MAX_ANALOG_VALUE, 0, 100);
    return constrain(level, 0, 100);
  }

  BatteryUtil_ BatteryUtil;

} // namespace hidpg
