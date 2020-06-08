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

  bool Layer_::_toggle[LAYER_SIZE] = {};
  uint8_t Layer_::_on_counters[LAYER_SIZE] = {};
  bool Layer_::_one_shot[LAYER_SIZE] = {};

  void Layer_::toggle(uint8_t number)
  {
    if (number >= LAYER_SIZE)
    {
      return;
    }
    _toggle[number] = !_toggle[number];
  }

  void Layer_::on(uint8_t number)
  {
    if (number >= LAYER_SIZE)
    {
      return;
    }
    _on_counters[number]++;
  }

  void Layer_::off(uint8_t number)
  {
    if (number >= LAYER_SIZE)
    {
      return;
    }
    _on_counters[number]--;
  }

  void Layer_::setOneShot(uint8_t number)
  {
    if (number >= LAYER_SIZE)
    {
      return;
    }
    _one_shot[number] = true;
  }

  void Layer_::peekOneShot(bool (&layer)[LAYER_SIZE])
  {
    for (int i = 0; i < LAYER_SIZE; i++)
    {
      layer[i] = _one_shot[i];
    }
  }

  void Layer_::getState(bool (&layer)[LAYER_SIZE])
  {
    for (int i = 0; i < LAYER_SIZE; i++)
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

  Layer_ Layer;

} // namespace hidpg
