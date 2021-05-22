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

#include <stdint.h>

namespace hidpg
{
  // clang-format off

  // Normal KeyCode
  enum class KeyCode : uint8_t
  {
    None            = 0,

    A               = 4,
    B               = 5,
    C               = 6,
    D               = 7,
    E               = 8,
    F               = 9,
    G               = 10,
    H               = 11,
    I               = 12,
    J               = 13,
    K               = 14,
    L               = 15,
    M               = 16,
    N               = 17,
    O               = 18,
    P               = 19,
    Q               = 20,
    R               = 21,
    S               = 22,
    T               = 23,
    U               = 24,
    V               = 25,
    W               = 26,
    X               = 27,
    Y               = 28,
    Z               = 29,
    _1              = 30,
    _2              = 31,
    _3              = 32,
    _4              = 33,
    _5              = 34,
    _6              = 35,
    _7              = 36,
    _8              = 37,
    _9              = 38,
    _0              = 39,
    Enter           = 40,
    Escape          = 41,
    Backspace       = 42,
    Tab             = 43,
    Space           = 44,
    Minus           = 45,
    Equal           = 46,
    BracketLeft     = 47,
    BracketRight    = 48,
    Backslash       = 49,
    NonUsNumberSign = 50,
    Semicolon       = 51,
    Quote           = 52,
    Grave           = 53,
    Comma           = 54,
    Period          = 55,
    Slash           = 56,
    CapsLock        = 57,
    F1              = 58,
    F2              = 59,
    F3              = 60,
    F4              = 61,
    F5              = 62,
    F6              = 63,
    F7              = 64,
    F8              = 65,
    F9              = 66,
    F10             = 67,
    F11             = 68,
    F12             = 69,
    PrintScreen     = 70,
    ScrollLock      = 71,
    Pause           = 72,
    Insert          = 73,
    Home            = 74,
    PageUp          = 75,
    Delete          = 76,
    End             = 77,
    PageDown        = 78,
    ArrowRight      = 79,
    ArrowLeft       = 80,
    ArrowDown       = 81,
    ArrowUp         = 82,
    NumLock         = 83,
    KeypadDivide    = 84,
    KeypadMultiply  = 85,
    KeypadSubtract  = 86,
    KeypadAdd       = 87,
    KeypadEnter     = 88,
    Keypad1         = 89,
    Keypad2         = 90,
    Keypad3         = 91,
    Keypad4         = 92,
    Keypad5         = 93,
    Keypad6         = 94,
    Keypad7         = 95,
    Keypad8         = 96,
    Keypad9         = 97,
    Keypad0         = 98,
    KeypadPeriod    = 99,
    NonUsBackslash  = 100,
    Application     = 101,
    Power           = 102,
    KeypadEqual     = 103,
    F13             = 104,
    F14             = 105,
    F15             = 106,
    F16             = 107,
    F17             = 108,
    F18             = 109,
    F19             = 110,
    F20             = 111,
    F21             = 112,
    F22             = 113,
    F23             = 114,
    F24             = 115,
    KeypadComma     = 133,
    Int1            = 135,
    Int2            = 136,
    Int3            = 137,
    Int4            = 138,
    Int5            = 139,
    Int6            = 140,
    Int7            = 141,
    Int8            = 142,
    Int9            = 143,
    Lang1           = 144,
    Lang2           = 145,
    Lang3           = 146,
    Lang4           = 147,
    Lang5           = 148,
    Lang6           = 149,
    Lang7           = 150,
    Lang8           = 151,
    Lang9           = 152,
  };

  // ModifierKeys
  enum class Modifier : uint8_t
  {
    LeftCtrl   = 1,
    LeftShift  = 2,
    LeftAlt    = 4,
    LeftGui    = 8,
    RightCtrl  = 16,
    RightShift = 32,
    RightAlt   = 64,
    RightGui   = 128,
  };

  constexpr Modifier operator+(const Modifier &a, const Modifier &b)
  {
    return static_cast<Modifier>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
  }

  // Mouse
  enum class MouseButton : uint8_t
  {
    Left     = 1,
    Right    = 2,
    Middle   = 4,
    Backward = 8,
    Forward  = 16,
  };

  constexpr MouseButton operator+(const MouseButton &a, const MouseButton &b)
  {
    return static_cast<MouseButton>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
  }

  // Consumer Control
  enum class ConsumerControlCode : uint16_t
  {
    PlayPause   = 205,
    FastForward = 179,
    Rewind      = 180,
    NextTrack   = 181,
    PrevTrack   = 182,
    Mute        = 226,
    VolumeUp    = 233,
    VolumeDown  = 234,

    LaunchMedia = 387,
    LaunchMail  = 394,
    LaunchApp1  = 404,
    LaunchApp2  = 402,
  };

  // System Control
  enum class SystemControlCode : uint8_t
  {
    SystemPowerDown = 1,
    SystemSleep     = 2,
    SystemWakeUp    = 3,
  };

} // namespace hidpg
