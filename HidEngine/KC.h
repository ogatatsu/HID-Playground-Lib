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

#include "KeyCode.h"

namespace hidpg
{
  // clang-format off
  static constexpr KeyCode None            = KeyCode::None;

  static constexpr KeyCode A               = KeyCode::A;
  static constexpr KeyCode B               = KeyCode::B;
  static constexpr KeyCode C               = KeyCode::C;
  static constexpr KeyCode D               = KeyCode::D;
  static constexpr KeyCode E               = KeyCode::E;
  static constexpr KeyCode F               = KeyCode::F;
  static constexpr KeyCode G               = KeyCode::G;
  static constexpr KeyCode H               = KeyCode::H;
  static constexpr KeyCode I               = KeyCode::I;
  static constexpr KeyCode J               = KeyCode::J;
  static constexpr KeyCode K               = KeyCode::K;
  static constexpr KeyCode L               = KeyCode::L;
  static constexpr KeyCode M               = KeyCode::M;
  static constexpr KeyCode N               = KeyCode::N;
  static constexpr KeyCode O               = KeyCode::O;
  static constexpr KeyCode P               = KeyCode::P;
  static constexpr KeyCode Q               = KeyCode::Q;
  static constexpr KeyCode R               = KeyCode::R;
  static constexpr KeyCode S               = KeyCode::S;
  static constexpr KeyCode T               = KeyCode::T;
  static constexpr KeyCode U               = KeyCode::U;
  static constexpr KeyCode V               = KeyCode::V;
  static constexpr KeyCode W               = KeyCode::W;
  static constexpr KeyCode X               = KeyCode::X;
  static constexpr KeyCode Y               = KeyCode::Y;
  static constexpr KeyCode Z               = KeyCode::Z;

  static constexpr KeyCode _1              = KeyCode::_1;
  static constexpr KeyCode _2              = KeyCode::_2;
  static constexpr KeyCode _3              = KeyCode::_3;
  static constexpr KeyCode _4              = KeyCode::_4;
  static constexpr KeyCode _5              = KeyCode::_5;
  static constexpr KeyCode _6              = KeyCode::_6;
  static constexpr KeyCode _7              = KeyCode::_7;
  static constexpr KeyCode _8              = KeyCode::_8;
  static constexpr KeyCode _9              = KeyCode::_9;
  static constexpr KeyCode _0              = KeyCode::_0;

  static constexpr KeyCode Enter           = KeyCode::Enter;
  static constexpr KeyCode Escape          = KeyCode::Escape;
  static constexpr KeyCode Backspace       = KeyCode::Backspace;
  static constexpr KeyCode Tab             = KeyCode::Tab;
  static constexpr KeyCode Space           = KeyCode::Space;
  static constexpr KeyCode Minus           = KeyCode::Minus;
  static constexpr KeyCode Equal           = KeyCode::Equal;
  static constexpr KeyCode BracketLeft     = KeyCode::BracketLeft;
  static constexpr KeyCode BracketRight    = KeyCode::BracketRight;
  static constexpr KeyCode Backslash       = KeyCode::Backslash;
  static constexpr KeyCode NonUsNumberSign = KeyCode::NonUsNumberSign;
  static constexpr KeyCode Semicolon       = KeyCode::Semicolon;
  static constexpr KeyCode Quote           = KeyCode::Quote;
  static constexpr KeyCode Grave           = KeyCode::Grave;
  static constexpr KeyCode Comma           = KeyCode::Comma;
  static constexpr KeyCode Period          = KeyCode::Period;
  static constexpr KeyCode Slash           = KeyCode::Slash;
  static constexpr KeyCode CapsLock        = KeyCode::CapsLock;

  static constexpr KeyCode F1              = KeyCode::F1;
  static constexpr KeyCode F2              = KeyCode::F2;
  static constexpr KeyCode F3              = KeyCode::F3;
  static constexpr KeyCode F4              = KeyCode::F4;
  static constexpr KeyCode F5              = KeyCode::F5;
  static constexpr KeyCode F6              = KeyCode::F6;
  static constexpr KeyCode F7              = KeyCode::F7;
  static constexpr KeyCode F8              = KeyCode::F8;
  static constexpr KeyCode F9              = KeyCode::F9;
  static constexpr KeyCode F10             = KeyCode::F10;
  static constexpr KeyCode F11             = KeyCode::F11;
  static constexpr KeyCode F12             = KeyCode::F12;

  static constexpr KeyCode PrintScreen     = KeyCode::PrintScreen;
  static constexpr KeyCode ScrollLock      = KeyCode::ScrollLock;
  static constexpr KeyCode Pause           = KeyCode::Pause;
  static constexpr KeyCode Insert          = KeyCode::Insert;
  static constexpr KeyCode Home            = KeyCode::Home;
  static constexpr KeyCode PageUp          = KeyCode::PageUp;
  static constexpr KeyCode Delete          = KeyCode::Delete;
  static constexpr KeyCode End             = KeyCode::End;
  static constexpr KeyCode PageDown        = KeyCode::PageDown;
  static constexpr KeyCode ArrowRight      = KeyCode::ArrowRight;
  static constexpr KeyCode ArrowLeft       = KeyCode::ArrowLeft;
  static constexpr KeyCode ArrowDown       = KeyCode::ArrowDown;
  static constexpr KeyCode ArrowUp         = KeyCode::ArrowUp;
  static constexpr KeyCode NumLock         = KeyCode::NumLock;
  static constexpr KeyCode KeypadDivide    = KeyCode::KeypadDivide;
  static constexpr KeyCode KeypadMultiply  = KeyCode::KeypadMultiply;
  static constexpr KeyCode KeypadSubtract  = KeyCode::KeypadSubtract;
  static constexpr KeyCode KeypadAdd       = KeyCode::KeypadAdd;
  static constexpr KeyCode KeypadEnter     = KeyCode::KeypadEnter;
  static constexpr KeyCode Keypad1         = KeyCode::Keypad1;
  static constexpr KeyCode Keypad2         = KeyCode::Keypad2;
  static constexpr KeyCode Keypad3         = KeyCode::Keypad3;
  static constexpr KeyCode Keypad4         = KeyCode::Keypad4;
  static constexpr KeyCode Keypad5         = KeyCode::Keypad5;
  static constexpr KeyCode Keypad6         = KeyCode::Keypad6;
  static constexpr KeyCode Keypad7         = KeyCode::Keypad7;
  static constexpr KeyCode Keypad8         = KeyCode::Keypad8;
  static constexpr KeyCode Keypad9         = KeyCode::Keypad9;
  static constexpr KeyCode Keypad0         = KeyCode::Keypad0;
  static constexpr KeyCode KeypadPeriod    = KeyCode::KeypadPeriod;
  static constexpr KeyCode NonUsBackslash  = KeyCode::NonUsBackslash;
  static constexpr KeyCode Application     = KeyCode::Application;
  static constexpr KeyCode Power           = KeyCode::Power;
  static constexpr KeyCode KeypadEqual     = KeyCode::KeypadEqual;
  static constexpr KeyCode F13             = KeyCode::F13;
  static constexpr KeyCode F14             = KeyCode::F14;
  static constexpr KeyCode F15             = KeyCode::F15;
  static constexpr KeyCode F16             = KeyCode::F16;
  static constexpr KeyCode F17             = KeyCode::F17;
  static constexpr KeyCode F18             = KeyCode::F18;
  static constexpr KeyCode F19             = KeyCode::F19;
  static constexpr KeyCode F20             = KeyCode::F20;
  static constexpr KeyCode F21             = KeyCode::F21;
  static constexpr KeyCode F22             = KeyCode::F22;
  static constexpr KeyCode F23             = KeyCode::F23;
  static constexpr KeyCode F24             = KeyCode::F24;

  static constexpr KeyCode KeypadComma     = KeyCode::KeypadComma;

  static constexpr KeyCode Int1            = KeyCode::Int1;
  static constexpr KeyCode Int2            = KeyCode::Int2;
  static constexpr KeyCode Int3            = KeyCode::Int3;
  static constexpr KeyCode Int4            = KeyCode::Int4;
  static constexpr KeyCode Int5            = KeyCode::Int5;
  static constexpr KeyCode Int6            = KeyCode::Int6;
  static constexpr KeyCode Int7            = KeyCode::Int7;
  static constexpr KeyCode Int8            = KeyCode::Int8;
  static constexpr KeyCode Int9            = KeyCode::Int9;
  static constexpr KeyCode Lang1           = KeyCode::Lang1;
  static constexpr KeyCode Lang2           = KeyCode::Lang2;
  static constexpr KeyCode Lang3           = KeyCode::Lang3;
  static constexpr KeyCode Lang4           = KeyCode::Lang4;
  static constexpr KeyCode Lang5           = KeyCode::Lang5;
  static constexpr KeyCode Lang6           = KeyCode::Lang6;
  static constexpr KeyCode Lang7           = KeyCode::Lang7;
  static constexpr KeyCode Lang8           = KeyCode::Lang8;
  static constexpr KeyCode Lang9           = KeyCode::Lang9;

  static constexpr Modifiers Ctrl              = Modifiers::LeftCtrl;
  static constexpr Modifiers Shift             = Modifiers::LeftShift;
  static constexpr Modifiers Alt               = Modifiers::LeftAlt;
  static constexpr Modifiers Gui               = Modifiers::LeftGui;
  static constexpr Modifiers LeftCtrl          = Modifiers::LeftCtrl;
  static constexpr Modifiers LeftShift         = Modifiers::LeftShift;
  static constexpr Modifiers LeftAlt           = Modifiers::LeftAlt;
  static constexpr Modifiers LeftGui           = Modifiers::LeftGui;
  static constexpr Modifiers RightCtrl         = Modifiers::RightCtrl;
  static constexpr Modifiers RightShift        = Modifiers::RightShift;
  static constexpr Modifiers RightAlt          = Modifiers::RightAlt;
  static constexpr Modifiers RightGui          = Modifiers::RightGui;

  static constexpr MouseButtons LeftButton     = MouseButtons::Left;
  static constexpr MouseButtons RightButton    = MouseButtons::Right;
  static constexpr MouseButtons MiddleButton   = MouseButtons::Middle;
  static constexpr MouseButtons BackwardButton = MouseButtons::Backward;
  static constexpr MouseButtons ForwardButton  = MouseButtons::Forward;

  static constexpr ConsumerControlCode LightUp         = ConsumerControlCode::LightUp;
  static constexpr ConsumerControlCode LightDown       = ConsumerControlCode::LightDown;
  static constexpr ConsumerControlCode PlayPause       = ConsumerControlCode::PlayPause;
  static constexpr ConsumerControlCode FastForward     = ConsumerControlCode::FastForward;
  static constexpr ConsumerControlCode Rewind          = ConsumerControlCode::Rewind;
  static constexpr ConsumerControlCode NextTrack       = ConsumerControlCode::NextTrack;
  static constexpr ConsumerControlCode PrevTrack       = ConsumerControlCode::PrevTrack;
  static constexpr ConsumerControlCode Mute            = ConsumerControlCode::Mute;
  static constexpr ConsumerControlCode VolumeUp        = ConsumerControlCode::VolumeUp;
  static constexpr ConsumerControlCode VolumeDown      = ConsumerControlCode::VolumeDown;
  static constexpr ConsumerControlCode LaunchMedia     = ConsumerControlCode::LaunchMedia;
  static constexpr ConsumerControlCode LaunchMail      = ConsumerControlCode::LaunchMail;
  static constexpr ConsumerControlCode LaunchApp1      = ConsumerControlCode::LaunchApp1;
  static constexpr ConsumerControlCode LaunchApp2      = ConsumerControlCode::LaunchApp2;

  static constexpr SystemControlCode SystemPowerDown   = SystemControlCode::SystemPowerDown;
  static constexpr SystemControlCode SystemSleep       = SystemControlCode::SystemSleep;
  static constexpr SystemControlCode SystemWakeUp      = SystemControlCode::SystemWakeUp;

} // namespace hidpg
