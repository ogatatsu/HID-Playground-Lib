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

#include "Layer.h"
#include "ArduinoMacro.h"

namespace hidpg
{

  LayerClass::LayerClass() : _base(0), _on_counters(), _toggle(0), _callback(nullptr)
  {
  }

  void LayerClass::on(uint8_t number)
  {
    if (number >= HID_ENGINE_LAYER_SIZE)
    {
      return;
    }

    if (_callback != nullptr)
    {
      layer_bitmap_t prev_state = getState();
      _on_counters[number]++;
      layer_bitmap_t state = getState();

      if (prev_state != state)
      {
        _callback(prev_state, state);
      }
    }
    else
    {
      _on_counters[number]++;
    }
  }

  void LayerClass::off(uint8_t number)
  {
    if (number >= HID_ENGINE_LAYER_SIZE)
    {
      return;
    }

    if (_callback != nullptr)
    {
      layer_bitmap_t prev_state = getState();
      _on_counters[number]--;
      layer_bitmap_t state = getState();

      if (prev_state != state)
      {
        _callback(prev_state, state);
      }
    }
    else
    {
      _on_counters[number]--;
    }
  }

  void LayerClass::toggle(uint8_t number)
  {
    if (number >= HID_ENGINE_LAYER_SIZE)
    {
      return;
    }

    if (_callback != nullptr)
    {
      layer_bitmap_t prev_state = getState();
      _toggle ^= 1UL << number;
      layer_bitmap_t state = getState();

      if (prev_state != state)
      {
        _callback(prev_state, state);
      }
    }
    else
    {
      _toggle ^= 1UL << number;
    }
  }

  void LayerClass::addToBase(int8_t i)
  {
    if (_callback != nullptr)
    {
      layer_bitmap_t prev_state = getState();
      _base += i;
      layer_bitmap_t state = getState();

      if (prev_state != state)
      {
        _callback(prev_state, state);
      }
    }
    else
    {
      _base += i;
    }
  }

  layer_bitmap_t LayerClass::getState()
  {
    uint8_t base = constrain(_base, 0, HID_ENGINE_LAYER_SIZE - 1);

    layer_bitmap_t result = (1UL << base) | _toggle;

    for (int i = 0; i < HID_ENGINE_LAYER_SIZE; i++)
    {
      if (_on_counters[i] != 0)
      {
        bitSet(result, i);
      }
    }

    return result;
  }

  void LayerClass::setCallback(callback_t callback)
  {
    _callback = callback;
  }

  LayerClass Layer;
  LayerClass Layer1;
  LayerClass Layer2;

} // namespace hidpg
