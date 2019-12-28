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

// undef ctype.h macro
#undef _U
#undef _L
#undef _N
#undef _S
#undef _P
#undef _C
#undef _X
#undef _B

static constexpr KeyCode _None = KeyCode::None;

static constexpr KeyCode _A               = KeyCode::A;
static constexpr KeyCode _B               = KeyCode::B;
static constexpr KeyCode _C               = KeyCode::C;
static constexpr KeyCode _D               = KeyCode::D;
static constexpr KeyCode _E               = KeyCode::E;
static constexpr KeyCode _F               = KeyCode::F;
static constexpr KeyCode _G               = KeyCode::G;
static constexpr KeyCode _H               = KeyCode::H;
static constexpr KeyCode _I               = KeyCode::I;
static constexpr KeyCode _J               = KeyCode::J;
static constexpr KeyCode _K               = KeyCode::K;
static constexpr KeyCode _L               = KeyCode::L;
static constexpr KeyCode _M               = KeyCode::M;
static constexpr KeyCode _N               = KeyCode::N;
static constexpr KeyCode _O               = KeyCode::O;
static constexpr KeyCode _P               = KeyCode::P;
static constexpr KeyCode _Q               = KeyCode::Q;
static constexpr KeyCode _R               = KeyCode::R;
static constexpr KeyCode _S               = KeyCode::S;
static constexpr KeyCode _T               = KeyCode::T;
static constexpr KeyCode _U               = KeyCode::U;
static constexpr KeyCode _V               = KeyCode::V;
static constexpr KeyCode _W               = KeyCode::W;
static constexpr KeyCode _X               = KeyCode::X;
static constexpr KeyCode _Y               = KeyCode::Y;
static constexpr KeyCode _Z               = KeyCode::Z;

static constexpr KeyCode _1               = KeyCode::_1;
static constexpr KeyCode _2               = KeyCode::_2;
static constexpr KeyCode _3               = KeyCode::_3;
static constexpr KeyCode _4               = KeyCode::_4;
static constexpr KeyCode _5               = KeyCode::_5;
static constexpr KeyCode _6               = KeyCode::_6;
static constexpr KeyCode _7               = KeyCode::_7;
static constexpr KeyCode _8               = KeyCode::_8;
static constexpr KeyCode _9               = KeyCode::_9;
static constexpr KeyCode _0               = KeyCode::_0;

static constexpr KeyCode _Enter           = KeyCode::Enter;
static constexpr KeyCode _Escape          = KeyCode::Escape;
static constexpr KeyCode _Backspace       = KeyCode::Backspace;
static constexpr KeyCode _Tab             = KeyCode::Tab;
static constexpr KeyCode _Space           = KeyCode::Space;
static constexpr KeyCode _Minus           = KeyCode::Minus;
static constexpr KeyCode _Equal           = KeyCode::Equal;
static constexpr KeyCode _BracketLeft     = KeyCode::BracketLeft;
static constexpr KeyCode _BracketRight    = KeyCode::BracketRight;
static constexpr KeyCode _Backslash       = KeyCode::Backslash;
static constexpr KeyCode _NonUsNumberSign = KeyCode::NonUsNumberSign;
static constexpr KeyCode _Semicolon       = KeyCode::Semicolon;
static constexpr KeyCode _Quote           = KeyCode::Quote;
static constexpr KeyCode _Grave           = KeyCode::Grave;
static constexpr KeyCode _Comma           = KeyCode::Comma;
static constexpr KeyCode _Period          = KeyCode::Period;
static constexpr KeyCode _Slash           = KeyCode::Slash;
static constexpr KeyCode _CapsLock        = KeyCode::CapsLock;

static constexpr KeyCode _F1              = KeyCode::F1;
static constexpr KeyCode _F2              = KeyCode::F2;
static constexpr KeyCode _F3              = KeyCode::F3;
static constexpr KeyCode _F4              = KeyCode::F4;
static constexpr KeyCode _F5              = KeyCode::F5;
static constexpr KeyCode _F6              = KeyCode::F6;
static constexpr KeyCode _F7              = KeyCode::F7;
static constexpr KeyCode _F8              = KeyCode::F8;
static constexpr KeyCode _F9              = KeyCode::F9;
static constexpr KeyCode _F10             = KeyCode::F10;
static constexpr KeyCode _F11             = KeyCode::F11;
static constexpr KeyCode _F12             = KeyCode::F12;

static constexpr KeyCode _PrintScreen     = KeyCode::PrintScreen;
static constexpr KeyCode _ScrollLock      = KeyCode::ScrollLock;
static constexpr KeyCode _Pause           = KeyCode::Pause;
static constexpr KeyCode _Insert          = KeyCode::Insert;
static constexpr KeyCode _Home            = KeyCode::Home;
static constexpr KeyCode _PageUp          = KeyCode::PageUp;
static constexpr KeyCode _Delete          = KeyCode::Delete;
static constexpr KeyCode _End             = KeyCode::End;
static constexpr KeyCode _PageDown        = KeyCode::PageDown;
static constexpr KeyCode _ArrowRight      = KeyCode::ArrowRight;
static constexpr KeyCode _ArrowLeft       = KeyCode::ArrowLeft;
static constexpr KeyCode _ArrowDown       = KeyCode::ArrowDown;
static constexpr KeyCode _ArrowUp         = KeyCode::ArrowUp;
static constexpr KeyCode _NumLock         = KeyCode::NumLock;
static constexpr KeyCode _KeypadDivide    = KeyCode::KeypadDivide;
static constexpr KeyCode _KeypadMultiply  = KeyCode::KeypadMultiply;
static constexpr KeyCode _KeypadSubtract  = KeyCode::KeypadSubtract;
static constexpr KeyCode _KeypadAdd       = KeyCode::KeypadAdd;
static constexpr KeyCode _KeypadEnter     = KeyCode::KeypadEnter;
static constexpr KeyCode _Keypad1         = KeyCode::Keypad1;
static constexpr KeyCode _Keypad2         = KeyCode::Keypad2;
static constexpr KeyCode _Keypad3         = KeyCode::Keypad3;
static constexpr KeyCode _Keypad4         = KeyCode::Keypad4;
static constexpr KeyCode _Keypad5         = KeyCode::Keypad5;
static constexpr KeyCode _Keypad6         = KeyCode::Keypad6;
static constexpr KeyCode _Keypad7         = KeyCode::Keypad7;
static constexpr KeyCode _Keypad8         = KeyCode::Keypad8;
static constexpr KeyCode _Keypad9         = KeyCode::Keypad9;
static constexpr KeyCode _Keypad0         = KeyCode::Keypad0;
static constexpr KeyCode _KeypadPeriod    = KeyCode::KeypadPeriod;
static constexpr KeyCode _NonUsBackslash  = KeyCode::NonUsBackslash;
static constexpr KeyCode _Application     = KeyCode::Application;
static constexpr KeyCode _Power           = KeyCode::Power;
static constexpr KeyCode _KeypadEqual     = KeyCode::KeypadEqual;
static constexpr KeyCode _F13             = KeyCode::F13;
static constexpr KeyCode _F14             = KeyCode::F14;
static constexpr KeyCode _F15             = KeyCode::F15;
static constexpr KeyCode _F16             = KeyCode::F16;
static constexpr KeyCode _F17             = KeyCode::F17;
static constexpr KeyCode _F18             = KeyCode::F18;
static constexpr KeyCode _F19             = KeyCode::F19;
static constexpr KeyCode _F20             = KeyCode::F20;
static constexpr KeyCode _F21             = KeyCode::F21;
static constexpr KeyCode _F22             = KeyCode::F22;
static constexpr KeyCode _F23             = KeyCode::F23;
static constexpr KeyCode _F24             = KeyCode::F24;

static constexpr KeyCode _Int1            = KeyCode::Int1;
static constexpr KeyCode _Int2            = KeyCode::Int2;
static constexpr KeyCode _Int3            = KeyCode::Int3;
static constexpr KeyCode _Int4            = KeyCode::Int4;
static constexpr KeyCode _Int5            = KeyCode::Int5;
static constexpr KeyCode _Int6            = KeyCode::Int6;
static constexpr KeyCode _Int7            = KeyCode::Int7;
static constexpr KeyCode _Int8            = KeyCode::Int8;
static constexpr KeyCode _Int9            = KeyCode::Int9;
static constexpr KeyCode _Lang1           = KeyCode::Lang1;
static constexpr KeyCode _Lang2           = KeyCode::Lang2;
static constexpr KeyCode _Lang3           = KeyCode::Lang3;
static constexpr KeyCode _Lang4           = KeyCode::Lang4;
static constexpr KeyCode _Lang5           = KeyCode::Lang5;
static constexpr KeyCode _Lang6           = KeyCode::Lang6;
static constexpr KeyCode _Lang7           = KeyCode::Lang7;
static constexpr KeyCode _Lang8           = KeyCode::Lang8;
static constexpr KeyCode _Lang9           = KeyCode::Lang9;

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

inline Modifier operator|(const Modifier &a, const Modifier &b)
{
  return static_cast<Modifier>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

static constexpr Modifier _Ctrl       = Modifier::LeftCtrl;
static constexpr Modifier _Shift      = Modifier::LeftShift;
static constexpr Modifier _Alt        = Modifier::LeftAlt;
static constexpr Modifier _Gui        = Modifier::LeftGui;
static constexpr Modifier _LeftCtrl   = Modifier::LeftCtrl;
static constexpr Modifier _LeftShift  = Modifier::LeftShift;
static constexpr Modifier _LeftAlt    = Modifier::LeftAlt;
static constexpr Modifier _LeftGui    = Modifier::LeftGui;
static constexpr Modifier _RightCtrl  = Modifier::RightCtrl;
static constexpr Modifier _RightShift = Modifier::RightShift;
static constexpr Modifier _RightAlt   = Modifier::RightAlt;
static constexpr Modifier _RightGui   = Modifier::RightGui;

// Mouse
enum class MouseButton : uint8_t
{
  Left     = 1,
  Right    = 2,
  Middle   = 4,
  Backward = 8,
  Forward  = 16,
};

inline MouseButton operator|(const MouseButton &a, const MouseButton &b)
{
  return static_cast<MouseButton>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

static constexpr MouseButton _LeftButton     = MouseButton::Left;
static constexpr MouseButton _RightButton    = MouseButton::Right;
static constexpr MouseButton _MiddleButton   = MouseButton::Middle;
static constexpr MouseButton _BackwardButton = MouseButton::Backward;
static constexpr MouseButton _ForwardButton  = MouseButton::Forward;

// Consumer Controll
enum class UsageCode : uint16_t
{
  PlayPause   = 205,
  FastForward = 179,
  Rewind      = 180,
  Next        = 181,
  Prev        = 182,
  Mute        = 226,
  VolumeUp    = 233,
  VolumeDown  = 234,
};

static constexpr UsageCode _PlayPause   = UsageCode::PlayPause;
static constexpr UsageCode _FastForward = UsageCode::FastForward;
static constexpr UsageCode _Rewind      = UsageCode::Rewind;
static constexpr UsageCode _Next        = UsageCode::Next;
static constexpr UsageCode _Prev        = UsageCode::Prev;
static constexpr UsageCode _Mute        = UsageCode::Mute;
static constexpr UsageCode _VolumeUp    = UsageCode::VolumeUp;
static constexpr UsageCode _VolumeDown  = UsageCode::VolumeDown;

} // namespace hidpg
