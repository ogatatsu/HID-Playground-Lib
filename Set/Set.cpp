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
#include "Arduino.h"

namespace hidpg
{

  Set::Set() : _data(), _count(0), _needs_recount(false)
  {
  }

  void Set::add(uint8_t val)
  {
    bitSet(_data[val / 8], val % 8);
    _needs_recount = true;
  }

  void Set::addAll(const uint8_t vals[], size_t len)
  {
    for (size_t i = 0; i < len; i++)
    {
      bitSet(_data[vals[i] / 8], vals[i] % 8);
    }
    _needs_recount = true;
  }

  Set &Set::operator|=(const Set &rhs)
  {
    uint32_t *data_32 = reinterpret_cast<uint32_t *>(_data);
    uint32_t *rhs_data_32 = reinterpret_cast<uint32_t *>(const_cast<uint8_t *>(rhs._data));
    for (int i = 0; i < 8; i++)
    {
      data_32[i] |= rhs_data_32[i];
    }
    _needs_recount = true;
    return *this;
  }

  void Set::remove(uint8_t val)
  {
    bitClear(_data[val / 8], val % 8);
    _needs_recount = true;
  }

  void Set::removeAll(const uint8_t vals[], size_t len)
  {
    for (size_t i = 0; i < len; i++)
    {
      bitClear(_data[vals[i] / 8], vals[i] % 8);
    }
    _needs_recount = true;
  }

  Set &Set::operator-=(const Set &rhs)
  {
    uint32_t *data_32 = reinterpret_cast<uint32_t *>(_data);
    uint32_t *rhs_data_32 = reinterpret_cast<uint32_t *>(const_cast<uint8_t *>(rhs._data));
    for (int i = 0; i < 8; i++)
    {
      data_32[i] &= ~(rhs_data_32[i]);
    }
    _needs_recount = true;
    return *this;
  }

  void Set::clear()
  {
    memset(_data, 0, sizeof(_data));
    _count = 0;
    _needs_recount = false;
  }

  bool Set::contains(uint8_t val) const
  {
    return bitRead(_data[val / 8], val % 8);
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
    uint16_t buf_size = this->count();
    uint16_t buf_size_cnt = 0;

    for (int num = 0;; num++)
    {
      if (buf_size_cnt == buf_size)
      {
        return;
      }
      if (contains(num))
      {
        buf[buf_size_cnt++] = num;
      }
    }
  }

  uint16_t Set::count() const
  {
    if (_needs_recount)
    {
      _count = 0;
      uint32_t *data_32 = reinterpret_cast<uint32_t *>(const_cast<uint8_t *>(_data));
      for (int i = 0; i < 8; i++)
      {
        _count += __builtin_popcountl(data_32[i]);
      }
      _needs_recount = false;
    }
    return _count;
  }

  bool operator==(const Set &a, const Set &b)
  {
    if (a.count() != b.count())
    {
      return false;
    }
    return memcmp(a._data, b._data, 32) ? false : true;
  }

  bool operator!=(const Set &a, const Set &b)
  {
    return !(a == b);
  }

  Set operator|(const Set &a, const Set &b)
  {
    Set result;
    result |= a;
    result |= b;
    return result;
  }

  Set operator-(const Set &a, const Set &b)
  {
    Set result;
    result |= a;
    result -= b;
    return result;
  }

} // namespace hidpg
