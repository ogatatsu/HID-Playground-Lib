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

#include "Switch.h"

namespace hidpg
{

Switch::Switch(uint8_t id, uint16_t debounceDelay) : Bounce(), _id(id)
{
  this->interval(debounceDelay);
}

void Switch::init(uint8_t pin)
{
  this->attach(pin);
}

void Switch::updateState(Set &ids)
{
  if (this->update())
  {
    if (this->read() == ACTIVE_STATE)
    {
      ids.add(_id);
    }
    else
    {
      ids.remove(_id);
    }
  }
}

uint16_t Switch::debounceDelay() const
{
  return this->interval_millis;
}

} // namespace hidpg
