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

namespace hidpg
{

  LayerClass::LayerClass() : _toggle(), _on_counters(), _one_shot()
  {
  }

  void LayerClass::toggle(uint8_t number)
  {
    if (number >= HID_ENGINE_LAYER_SIZE)
    {
      return;
    }
    _toggle[number] = !_toggle[number];
  }

  void LayerClass::on(uint8_t number)
  {
    if (number >= HID_ENGINE_LAYER_SIZE)
    {
      return;
    }
    _on_counters[number]++;
  }

  void LayerClass::off(uint8_t number)
  {
    if (number >= HID_ENGINE_LAYER_SIZE)
    {
      return;
    }
    _on_counters[number]--;
  }

  void LayerClass::setOneShot(uint8_t number)
  {
    if (number >= HID_ENGINE_LAYER_SIZE)
    {
      return;
    }
    _one_shot[number] = true;
  }

  void LayerClass::peekOneShot(bool (&layer)[HID_ENGINE_LAYER_SIZE])
  {
    for (int i = 0; i < HID_ENGINE_LAYER_SIZE; i++)
    {
      layer[i] = _one_shot[i];
    }
  }

  void LayerClass::takeState(bool (&layer)[HID_ENGINE_LAYER_SIZE])
  {
    for (int i = 0; i < HID_ENGINE_LAYER_SIZE; i++)
    {
      if (_one_shot[i] == true)
      {
        layer[i] = true;
        _one_shot[i] = false;
        continue;
      }

      if (_on_counters[i] != 0)
      {
        layer[i] = true;
        continue;
      }

      layer[i] = _toggle[i];
    }
    // layer 0 is always true
    layer[0] = true;
  }

  LayerClass Layer1;
  LayerClass Layer2;

} // namespace hidpg
