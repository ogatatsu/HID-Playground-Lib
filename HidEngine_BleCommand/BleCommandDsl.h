/*
  The MIT License (MIT)

  Copyright (c) 2021 ogatatsu.

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

#include "BleCommand.h"
#include "hash_code.h"
#include <new>

namespace hidpg
{

  namespace Internal
  {
    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_ConnectBluetooth(uint8_t slot)
    {
      static uint8_t buf[sizeof(ConnectBluetooth)];
      return new (buf) ConnectBluetooth(slot);
    }

    template <uint64_t ID1, uint64_t ID2, uint64_t ID3>
    Command *new_ResetConnection()
    {
      static uint8_t buf[sizeof(ResetConnection)];
      return new (buf) ResetConnection();
    }

  } // namespace Internal

// ConnectBluetooth
#define BT(slot) (Internal::new_ConnectBluetooth<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>(slot))

// ResetConnection
#define RESET() (Internal::new_ResetConnection<__COUNTER__, hash_code(109, __FILE__), hash_code(103, __FILE__)>())

} // namespace hidpg
