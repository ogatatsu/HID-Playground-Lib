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

// Normal Keycode
enum class Keycode : uint8_t
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

static constexpr Keycode _None = Keycode::None;

static constexpr Keycode _A               = Keycode::A;
static constexpr Keycode _B               = Keycode::B;
static constexpr Keycode _C               = Keycode::C;
static constexpr Keycode _D               = Keycode::D;
static constexpr Keycode _E               = Keycode::E;
static constexpr Keycode _F               = Keycode::F;
static constexpr Keycode _G               = Keycode::G;
static constexpr Keycode _H               = Keycode::H;
static constexpr Keycode _I               = Keycode::I;
static constexpr Keycode _J               = Keycode::J;
static constexpr Keycode _K               = Keycode::K;
static constexpr Keycode _L               = Keycode::L;
static constexpr Keycode _M               = Keycode::M;
static constexpr Keycode _N               = Keycode::N;
static constexpr Keycode _O               = Keycode::O;
static constexpr Keycode _P               = Keycode::P;
static constexpr Keycode _Q               = Keycode::Q;
static constexpr Keycode _R               = Keycode::R;
static constexpr Keycode _S               = Keycode::S;
static constexpr Keycode _T               = Keycode::T;
static constexpr Keycode _U               = Keycode::U;
static constexpr Keycode _V               = Keycode::V;
static constexpr Keycode _W               = Keycode::W;
static constexpr Keycode _X               = Keycode::X;
static constexpr Keycode _Y               = Keycode::Y;
static constexpr Keycode _Z               = Keycode::Z;

static constexpr Keycode _1               = Keycode::_1;
static constexpr Keycode _2               = Keycode::_2;
static constexpr Keycode _3               = Keycode::_3;
static constexpr Keycode _4               = Keycode::_4;
static constexpr Keycode _5               = Keycode::_5;
static constexpr Keycode _6               = Keycode::_6;
static constexpr Keycode _7               = Keycode::_7;
static constexpr Keycode _8               = Keycode::_8;
static constexpr Keycode _9               = Keycode::_9;
static constexpr Keycode _0               = Keycode::_0;

static constexpr Keycode _Enter           = Keycode::Enter;
static constexpr Keycode _Escape          = Keycode::Escape;
static constexpr Keycode _Backspace       = Keycode::Backspace;
static constexpr Keycode _Tab             = Keycode::Tab;
static constexpr Keycode _Space           = Keycode::Space;
static constexpr Keycode _Minus           = Keycode::Minus;
static constexpr Keycode _Equal           = Keycode::Equal;
static constexpr Keycode _BracketLeft     = Keycode::BracketLeft;
static constexpr Keycode _BracketRight    = Keycode::BracketRight;
static constexpr Keycode _Backslash       = Keycode::Backslash;
static constexpr Keycode _NonUsNumberSign = Keycode::NonUsNumberSign;
static constexpr Keycode _Semicolon       = Keycode::Semicolon;
static constexpr Keycode _Quote           = Keycode::Quote;
static constexpr Keycode _Grave           = Keycode::Grave;
static constexpr Keycode _Comma           = Keycode::Comma;
static constexpr Keycode _Period          = Keycode::Period;
static constexpr Keycode _Slash           = Keycode::Slash;
static constexpr Keycode _CapsLock        = Keycode::CapsLock;

static constexpr Keycode _F1              = Keycode::F1;
static constexpr Keycode _F2              = Keycode::F2;
static constexpr Keycode _F3              = Keycode::F3;
static constexpr Keycode _F4              = Keycode::F4;
static constexpr Keycode _F5              = Keycode::F5;
static constexpr Keycode _F6              = Keycode::F6;
static constexpr Keycode _F7              = Keycode::F7;
static constexpr Keycode _F8              = Keycode::F8;
static constexpr Keycode _F9              = Keycode::F9;
static constexpr Keycode _F10             = Keycode::F10;
static constexpr Keycode _F11             = Keycode::F11;
static constexpr Keycode _F12             = Keycode::F12;

static constexpr Keycode _PrintScreen     = Keycode::PrintScreen;
static constexpr Keycode _ScrollLock      = Keycode::ScrollLock;
static constexpr Keycode _Pause           = Keycode::Pause;
static constexpr Keycode _Insert          = Keycode::Insert;
static constexpr Keycode _Home            = Keycode::Home;
static constexpr Keycode _PageUp          = Keycode::PageUp;
static constexpr Keycode _Delete          = Keycode::Delete;
static constexpr Keycode _End             = Keycode::End;
static constexpr Keycode _PageDown        = Keycode::PageDown;
static constexpr Keycode _ArrowRight      = Keycode::ArrowRight;
static constexpr Keycode _ArrowLeft       = Keycode::ArrowLeft;
static constexpr Keycode _ArrowDown       = Keycode::ArrowDown;
static constexpr Keycode _ArrowUp         = Keycode::ArrowUp;
static constexpr Keycode _NumLock         = Keycode::NumLock;
static constexpr Keycode _KeypadDivide    = Keycode::KeypadDivide;
static constexpr Keycode _KeypadMultiply  = Keycode::KeypadMultiply;
static constexpr Keycode _KeypadSubtract  = Keycode::KeypadSubtract;
static constexpr Keycode _KeypadAdd       = Keycode::KeypadAdd;
static constexpr Keycode _KeypadEnter     = Keycode::KeypadEnter;
static constexpr Keycode _Keypad1         = Keycode::Keypad1;
static constexpr Keycode _Keypad2         = Keycode::Keypad2;
static constexpr Keycode _Keypad3         = Keycode::Keypad3;
static constexpr Keycode _Keypad4         = Keycode::Keypad4;
static constexpr Keycode _Keypad5         = Keycode::Keypad5;
static constexpr Keycode _Keypad6         = Keycode::Keypad6;
static constexpr Keycode _Keypad7         = Keycode::Keypad7;
static constexpr Keycode _Keypad8         = Keycode::Keypad8;
static constexpr Keycode _Keypad9         = Keycode::Keypad9;
static constexpr Keycode _Keypad0         = Keycode::Keypad0;
static constexpr Keycode _KeypadPeriod    = Keycode::KeypadPeriod;
static constexpr Keycode _NonUsBackslash  = Keycode::NonUsBackslash;
static constexpr Keycode _Application     = Keycode::Application;
static constexpr Keycode _Power           = Keycode::Power;
static constexpr Keycode _KeypadEqual     = Keycode::KeypadEqual;
static constexpr Keycode _F13             = Keycode::F13;
static constexpr Keycode _F14             = Keycode::F14;
static constexpr Keycode _F15             = Keycode::F15;
static constexpr Keycode _F16             = Keycode::F16;
static constexpr Keycode _F17             = Keycode::F17;
static constexpr Keycode _F18             = Keycode::F18;
static constexpr Keycode _F19             = Keycode::F19;
static constexpr Keycode _F20             = Keycode::F20;
static constexpr Keycode _F21             = Keycode::F21;
static constexpr Keycode _F22             = Keycode::F22;
static constexpr Keycode _F23             = Keycode::F23;
static constexpr Keycode _F24             = Keycode::F24;

static constexpr Keycode _Int1            = Keycode::Int1;
static constexpr Keycode _Int2            = Keycode::Int2;
static constexpr Keycode _Int3            = Keycode::Int3;
static constexpr Keycode _Int4            = Keycode::Int4;
static constexpr Keycode _Int5            = Keycode::Int5;
static constexpr Keycode _Int6            = Keycode::Int6;
static constexpr Keycode _Int7            = Keycode::Int7;
static constexpr Keycode _Int8            = Keycode::Int8;
static constexpr Keycode _Int9            = Keycode::Int9;
static constexpr Keycode _Lang1           = Keycode::Lang1;
static constexpr Keycode _Lang2           = Keycode::Lang2;
static constexpr Keycode _Lang3           = Keycode::Lang3;
static constexpr Keycode _Lang4           = Keycode::Lang4;
static constexpr Keycode _Lang5           = Keycode::Lang5;
static constexpr Keycode _Lang6           = Keycode::Lang6;
static constexpr Keycode _Lang7           = Keycode::Lang7;
static constexpr Keycode _Lang8           = Keycode::Lang8;
static constexpr Keycode _Lang9           = Keycode::Lang9;

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
