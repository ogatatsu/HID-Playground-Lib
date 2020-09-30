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

#include "HidEngine_config.h"
#include <stdint.h>

namespace hidpg
{

  class LayerClass
  {
  public:
    // 恒久的な操作
    static void toggle(uint8_t number);

    // 一時的な操作
    static void on(uint8_t number);
    static void off(uint8_t number);

    // OneShot
    static void setOneShot(uint8_t number);
    static void peekOneShot(bool (&layer)[LAYER_SIZE]);

    // 現在の状態を取得
    static void takeState(bool (&layer)[LAYER_SIZE]);

  private:
    static bool _toggle[LAYER_SIZE];
    static uint8_t _on_counters[LAYER_SIZE];
    static bool _one_shot[LAYER_SIZE];
  };

  extern LayerClass Layer;

} // namespace hidpg
