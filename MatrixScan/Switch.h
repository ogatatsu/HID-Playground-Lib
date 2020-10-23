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

#include "Bounce2.h"
#include "MatrixScan_config.h"
#include "Set.h"

namespace hidpg
{

  // 物理的なスイッチ1個に対応するクラス
  class Switch : Bounce
  {
  public:
    using Bounce::attach;

  public:
    // 論理的なIDをセットする
    Switch(uint8_t id, uint16_t debounce_delay_ms = MATRIX_SCAN_DEBOUNCE_DELAY_MS);
    // スキャン時に呼ばれる、押されてるかを自分でチェックして自分のIDをセットする
    void updateState(Set &switch_ids);

    uint16_t getDebounceDelay() const;

  private:
    const uint8_t _id;
  };

} // namespace hidpg
