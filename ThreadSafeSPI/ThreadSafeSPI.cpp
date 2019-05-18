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

#include "ThreadSafeSPI.h"

namespace hidpg
{

ThreadSafeSPIClass::ThreadSafeSPIClass(SPIClass &spi) : _spi(spi), _initialized(false)
{
}

void ThreadSafeSPIClass::begin()
{
  if (_initialized == false)
  {
    _spi.begin();
    _mutex = xSemaphoreCreateMutex();
    _initialized = true;
  }
}

void ThreadSafeSPIClass::end()
{
  if (_initialized)
  {
    _spi.end();
    vSemaphoreDelete(_mutex);
    _initialized = false;
  }
}

void ThreadSafeSPIClass::usingInterrupt(int interruptNumber)
{
  _spi.usingInterrupt(interruptNumber);
}

void ThreadSafeSPIClass::beginTransaction(SPISettings &settings)
{
  xSemaphoreTake(_mutex, portMAX_DELAY);
  _spi.beginTransaction(settings);
}

void ThreadSafeSPIClass::endTransaction()
{
  _spi.endTransaction();
  xSemaphoreGive(_mutex);
}

uint8_t ThreadSafeSPIClass::transfer(uint8_t data)
{
  return _spi.transfer(data);
}

uint16_t ThreadSafeSPIClass::transfer16(uint16_t data)
{
  return _spi.transfer16(data);
}

void ThreadSafeSPIClass::transfer(void *buf, size_t count)
{
  _spi.transfer(buf, count);
}

ThreadSafeSPIClass ThreadSafeSPI(SPI);

} // namespace hidpg
