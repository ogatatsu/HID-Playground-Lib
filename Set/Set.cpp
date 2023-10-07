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

#include "Set.h"
#include "ArduinoMacro.h"
#include <string.h>

namespace hidpg
{

  Set::Set() : _data(), _count(0)
  {
  }

  bool Set::add(uint8_t val)
  {
    if (contains(val))
    {
      return false;
    }

    bitSet(_data[val / _data_bit_size], val % _data_bit_size);
    _count++;
    return true;
  }

  void Set::addAll(const uint8_t vals[], size_t len)
  {
    for (size_t i = 0; i < len; i++)
    {
      add(vals[i]);
    }
  }

  Set &Set::operator|=(const Set &rhs)
  {
    _count = 0;

    for (size_t i = 0; i < _data_size; i++)
    {
      _data[i] |= rhs._data[i];
      _count += __builtin_popcountl(_data[i]);
    }
    return *this;
  }

  bool Set::remove(uint8_t val)
  {
    if (contains(val) == false)
    {
      return false;
    }

    bitClear(_data[val / _data_bit_size], val % _data_bit_size);
    _count--;
    return true;
  }

  void Set::removeAll(const uint8_t vals[], size_t len)
  {
    for (size_t i = 0; i < len; i++)
    {
      remove(vals[i]);
    }
  }

  Set &Set::operator-=(const Set &rhs)
  {
    _count = 0;
    for (size_t i = 0; i < _data_size; i++)
    {
      _data[i] &= ~(rhs._data[i]);
      _count += __builtin_popcountl(_data[i]);
    }
    return *this;
  }

  bool Set::update(uint8_t val, bool b)
  {
    if (b)
    {
      return add(val);
    }
    else
    {
      return remove(val);
    }
  }

  void Set::clear()
  {
    memset(_data, 0, sizeof(_data));
    _count = 0;
  }

  bool Set::contains(uint8_t val) const
  {
    return bitRead(_data[val / _data_bit_size], val % _data_bit_size);
  }

  bool Set::containsAll(const uint8_t vals[], size_t len) const
  {
    for (size_t i = 0; i < len; i++)
    {
      if (contains(vals[i]) == false)
      {
        return false;
      }
    }
    return true;
  }

  bool Set::containsAny(const uint8_t vals[], size_t len) const
  {
    for (size_t i = 0; i < len; i++)
    {
      if (contains(vals[i]))
      {
        return true;
      }
    }
    return false;
  }

  void Set::toArray(uint8_t buf[]) const
  {
    if (_count == 0)
      return;

    uint16_t buf_cnt = 0;

    for (size_t i = 0; i < _data_size; i++)
    {
      if (_data[i] == 0)
      {
        continue;
      }
      for (size_t j = 0; j < _data_bit_size; j++)
      {
        uint8_t val = i * _data_bit_size + j;
        if (contains(val))
        {
          buf[buf_cnt++] = val;

          if (buf_cnt == _count)
          {
            return;
          }
        }
      }
    }
  }

  uint16_t Set::count() const
  {
    return _count;
  }

  bool operator==(const Set &a, const Set &b)
  {
    if (a.count() != b.count())
    {
      return false;
    }
    return memcmp(a._data, b._data, sizeof(Set::_data)) ? false : true;
  }

  bool operator!=(const Set &a, const Set &b)
  {
    return !(a == b);
  }

  Set operator|(const Set &a, const Set &b)
  {
    Set result;

    for (size_t i = 0; i < Set::_data_size; i++)
    {
      result._data[i] |= a._data[i];
      result._data[i] |= b._data[i];
      result._count += __builtin_popcountl(result._data[i]);
    }
    return result;
  }

  Set operator-(const Set &a, const Set &b)
  {
    Set result;

    for (size_t i = 0; i < Set::_data_size; i++)
    {
      result._data[i] |= a._data[i];
      result._data[i] &= ~(b._data[i]);
      result._count += __builtin_popcountl(result._data[i]);
    }
    return result;
  }

} // namespace hidpg
