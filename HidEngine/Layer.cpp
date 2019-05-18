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

bool Layer::_layer[LAYER_SIZE];
uint8_t Layer::_counter[LAYER_SIZE];
bool Layer::_oneShotLayer[LAYER_SIZE];

void Layer::toggle(uint8_t number)
{
  if (number >= LAYER_SIZE)
  {
    return;
  }
  _layer[number] = !_layer[number];
}

void Layer::on(uint8_t number)
{
  if (number >= LAYER_SIZE)
  {
    return;
  }
  _counter[number]++;
}

void Layer::off(uint8_t number)
{
  if (number >= LAYER_SIZE)
  {
    return;
  }
  _counter[number]--;
}

void Layer::setOneShot(uint8_t number)
{
  if (number >= LAYER_SIZE)
  {
    return;
  }
  _oneShotLayer[number] = true;
}

void Layer::peekOneShot(bool (&layer)[LAYER_SIZE])
{
  for (int i = 0; i < LAYER_SIZE; i++)
  {
    layer[i] = _oneShotLayer[i];
  }
}

void Layer::getState(bool (&layer)[LAYER_SIZE])
{
  for (int i = 0; i < LAYER_SIZE; i++)
  {
    if (_oneShotLayer[i] == true)
    {
      layer[i] = true;
      _oneShotLayer[i] = false;
      continue;
    }

    if (_counter[i] != 0)
    {
      layer[i] = true;
      continue;
    }

    layer[i] = _layer[i];
  }
  // layer 0 is always true
  layer[0] = true;
}

} // namespace hidpg
