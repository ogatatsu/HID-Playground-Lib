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

  Set::Set() : _data(), _count(0)
  {
  }

  bool Set::add(uint8_t val)
  {
    if (contains(val))
    {
      return false;
    }

    bitSet(_data[val / 8], val % 8);
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
    uint32_t *data_32 = reinterpret_cast<uint32_t *>(_data);
    uint32_t *rhs_data_32 = reinterpret_cast<uint32_t *>(const_cast<uint8_t *>(rhs._data));
    _count = 0;
    for (int i = 0; i < 8; i++)
    {
      data_32[i] |= rhs_data_32[i];
      _count += __builtin_popcountl(data_32[i]);
    }
    return *this;
  }

  bool Set::remove(uint8_t val)
  {
    if (contains(val) == false)
    {
      return false;
    }

    bitClear(_data[val / 8], val % 8);
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
    uint32_t *data_32 = reinterpret_cast<uint32_t *>(_data);
    uint32_t *rhs_data_32 = reinterpret_cast<uint32_t *>(const_cast<uint8_t *>(rhs._data));
    _count = 0;
    for (int i = 0; i < 8; i++)
    {
      data_32[i] &= ~(rhs_data_32[i]);
      _count += __builtin_popcountl(data_32[i]);
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
    if (_count == 0)
      return;

    uint16_t buf_cnt = 0;
    uint32_t *data_32 = reinterpret_cast<uint32_t *>(const_cast<uint8_t *>(_data));

    for (int i = 0; i < 8; i++)
    {
      if (data_32[i] == 0)
      {
        continue;
      }
      for (int j = 0; j < 32; j++)
      {
        uint8_t val = i * 32 + j;
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
    return memcmp(a._data, b._data, 32) ? false : true;
  }

  bool operator!=(const Set &a, const Set &b)
  {
    return !(a == b);
  }

  Set operator|(const Set &a, const Set &b)
  {
    Set result;

    uint32_t *a_data_32 = reinterpret_cast<uint32_t *>(const_cast<uint8_t *>(a._data));
    uint32_t *b_data_32 = reinterpret_cast<uint32_t *>(const_cast<uint8_t *>(b._data));
    uint32_t *r_data_32 = reinterpret_cast<uint32_t *>(const_cast<uint8_t *>(result._data));

    for (int i = 0; i < 8; i++)
    {
      r_data_32[i] |= a_data_32[i];
      r_data_32[i] |= b_data_32[i];
      result._count += __builtin_popcountl(r_data_32[i]);
    }
    return result;
  }

  Set operator-(const Set &a, const Set &b)
  {
    Set result;

    uint32_t *a_data_32 = reinterpret_cast<uint32_t *>(const_cast<uint8_t *>(a._data));
    uint32_t *b_data_32 = reinterpret_cast<uint32_t *>(const_cast<uint8_t *>(b._data));
    uint32_t *r_data_32 = reinterpret_cast<uint32_t *>(const_cast<uint8_t *>(result._data));

    for (int i = 0; i < 8; i++)
    {
      r_data_32[i] |= a_data_32[i];
      r_data_32[i] &= ~(b_data_32[i]);
      result._count += __builtin_popcountl(r_data_32[i]);
    }
    return result;
  }

} // namespace hidpg
