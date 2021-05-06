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
    LayerClass();

    // 恒久的な操作
    void toggle(uint8_t number);

    // 一時的な操作
    void on(uint8_t number);
    void off(uint8_t number);

    // OneShot
    void setOneShot(uint8_t number);
    void peekOneShot(bool (&layer)[HID_ENGINE_LAYER_SIZE]);

    // 現在の状態を取得
    void takeState(bool (&layer)[HID_ENGINE_LAYER_SIZE]);

  private:
    bool _toggle[HID_ENGINE_LAYER_SIZE];
    uint8_t _on_counters[HID_ENGINE_LAYER_SIZE];
    bool _one_shot[HID_ENGINE_LAYER_SIZE];
  };

  extern LayerClass Layer1;
  extern LayerClass Layer2;
  extern LayerClass Layer3;

} // namespace hidpg
