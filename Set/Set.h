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

#pragma once

#include <stddef.h>
#include <stdint.h>

namespace hidpg
{

  // 0から255の値が入るBitSet
  class Set
  {
    friend bool operator==(const Set &a, const Set &b);

  public:
    Set();

    void add(uint8_t val);
    void addAll(const uint8_t vals[], size_t len);
    Set &operator|=(const Set &rhs);

    void remove(uint8_t val);
    void removeAll(const uint8_t vals[], size_t len);
    Set &operator-=(const Set &rhs);

    void clear();

    bool contains(uint8_t val) const;
    bool containsAll(const uint8_t vals[], size_t len) const;
    bool containsAny(const uint8_t vals[], size_t len) const;

    void toArray(uint8_t buf[]) const;
    uint16_t count() const;

  private:
    // 8 * 32 = 256
    uint8_t _data[32];
    mutable uint16_t _count;
    mutable bool _needs_recount;
  };

  // 比較
  bool operator==(const Set &a, const Set &b);
  bool operator!=(const Set &a, const Set &b);
  // 和集合
  Set operator|(const Set &a, const Set &b);
  // 差集合
  Set operator-(const Set &a, const Set &b);

} // namespace hidpg
