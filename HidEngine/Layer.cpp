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
#include "Arduino.h"
#include <string.h>

namespace hidpg
{

  LayerClass::LayerClass() : _on_counters(), _toggle(0), _one_shot(0), _callback(nullptr)
  {
  }

  void LayerClass::on(uint8_t number)
  {
    if (number >= HID_ENGINE_LAYER_SIZE)
    {
      return;
    }

    if (_callback != nullptr && _on_counters[number] == 0)
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

    if (_callback != nullptr && _on_counters[number] == 1)
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

  void LayerClass::setOneShot(uint8_t number)
  {
    if (number >= HID_ENGINE_LAYER_SIZE)
    {
      return;
    }

    if (_callback != nullptr && bitRead(_one_shot, number) == 0)
    {
      layer_bitmap_t prev_state = getState();
      bitSet(_one_shot, number);
      layer_bitmap_t state = getState();

      if (prev_state != state)
      {
        _callback(prev_state, state);
      }
    }
    else
    {
      bitSet(_one_shot, number);
    }
  }

  layer_bitmap_t LayerClass::getOneShotState()
  {
    return _one_shot;
  }

  void LayerClass::clearOneShot()
  {
    if (_callback != nullptr)
    {
      layer_bitmap_t prev_state = getState();
      _one_shot = 0;
      layer_bitmap_t state = getState();

      if (prev_state != state)
      {
        _callback(prev_state, state);
      }
    }
    else
    {
      _one_shot = 0;
    }
  }

  layer_bitmap_t LayerClass::getState()
  {
    // layer 0 is always true
    layer_bitmap_t result = 1 | _toggle | _one_shot;

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
