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

  static constexpr CharacterKey A               = CharacterKey::A;
  static constexpr CharacterKey B               = CharacterKey::B;
  static constexpr CharacterKey C               = CharacterKey::C;
  static constexpr CharacterKey D               = CharacterKey::D;
  static constexpr CharacterKey E               = CharacterKey::E;
  static constexpr CharacterKey F               = CharacterKey::F;
  static constexpr CharacterKey G               = CharacterKey::G;
  static constexpr CharacterKey H               = CharacterKey::H;
  static constexpr CharacterKey I               = CharacterKey::I;
  static constexpr CharacterKey J               = CharacterKey::J;
  static constexpr CharacterKey K               = CharacterKey::K;
  static constexpr CharacterKey L               = CharacterKey::L;
  static constexpr CharacterKey M               = CharacterKey::M;
  static constexpr CharacterKey N               = CharacterKey::N;
  static constexpr CharacterKey O               = CharacterKey::O;
  static constexpr CharacterKey P               = CharacterKey::P;
  static constexpr CharacterKey Q               = CharacterKey::Q;
  static constexpr CharacterKey R               = CharacterKey::R;
  static constexpr CharacterKey S               = CharacterKey::S;
  static constexpr CharacterKey T               = CharacterKey::T;
  static constexpr CharacterKey U               = CharacterKey::U;
  static constexpr CharacterKey V               = CharacterKey::V;
  static constexpr CharacterKey W               = CharacterKey::W;
  static constexpr CharacterKey X               = CharacterKey::X;
  static constexpr CharacterKey Y               = CharacterKey::Y;
  static constexpr CharacterKey Z               = CharacterKey::Z;

  static constexpr CharacterKey _1              = CharacterKey::_1;
  static constexpr CharacterKey _2              = CharacterKey::_2;
  static constexpr CharacterKey _3              = CharacterKey::_3;
  static constexpr CharacterKey _4              = CharacterKey::_4;
  static constexpr CharacterKey _5              = CharacterKey::_5;
  static constexpr CharacterKey _6              = CharacterKey::_6;
  static constexpr CharacterKey _7              = CharacterKey::_7;
  static constexpr CharacterKey _8              = CharacterKey::_8;
  static constexpr CharacterKey _9              = CharacterKey::_9;
  static constexpr CharacterKey _0              = CharacterKey::_0;

  static constexpr CharacterKey Enter           = CharacterKey::Enter;
  static constexpr CharacterKey Escape          = CharacterKey::Escape;
  static constexpr CharacterKey Backspace       = CharacterKey::Backspace;
  static constexpr CharacterKey Tab             = CharacterKey::Tab;
  static constexpr CharacterKey Space           = CharacterKey::Space;
  static constexpr CharacterKey Minus           = CharacterKey::Minus;
  static constexpr CharacterKey Equal           = CharacterKey::Equal;
  static constexpr CharacterKey LeftBracket     = CharacterKey::LeftBracket;
  static constexpr CharacterKey RightBracket    = CharacterKey::RightBracket;
  static constexpr CharacterKey Backslash       = CharacterKey::Backslash;
  static constexpr CharacterKey NonUsNumberSign = CharacterKey::NonUsNumberSign;
  static constexpr CharacterKey Semicolon       = CharacterKey::Semicolon;
  static constexpr CharacterKey Quote           = CharacterKey::Quote;
  static constexpr CharacterKey Grave           = CharacterKey::Grave;
  static constexpr CharacterKey Comma           = CharacterKey::Comma;
  static constexpr CharacterKey Period          = CharacterKey::Period;
  static constexpr CharacterKey Slash           = CharacterKey::Slash;
  static constexpr CharacterKey CapsLock        = CharacterKey::CapsLock;

  static constexpr CharacterKey F1              = CharacterKey::F1;
  static constexpr CharacterKey F2              = CharacterKey::F2;
  static constexpr CharacterKey F3              = CharacterKey::F3;
  static constexpr CharacterKey F4              = CharacterKey::F4;
  static constexpr CharacterKey F5              = CharacterKey::F5;
  static constexpr CharacterKey F6              = CharacterKey::F6;
  static constexpr CharacterKey F7              = CharacterKey::F7;
  static constexpr CharacterKey F8              = CharacterKey::F8;
  static constexpr CharacterKey F9              = CharacterKey::F9;
  static constexpr CharacterKey F10             = CharacterKey::F10;
  static constexpr CharacterKey F11             = CharacterKey::F11;
  static constexpr CharacterKey F12             = CharacterKey::F12;

  static constexpr CharacterKey PrintScreen     = CharacterKey::PrintScreen;
  static constexpr CharacterKey ScrollLock      = CharacterKey::ScrollLock;
  static constexpr CharacterKey Pause           = CharacterKey::Pause;
  static constexpr CharacterKey Insert          = CharacterKey::Insert;
  static constexpr CharacterKey Home            = CharacterKey::Home;
  static constexpr CharacterKey PageUp          = CharacterKey::PageUp;
  static constexpr CharacterKey Delete          = CharacterKey::Delete;
  static constexpr CharacterKey End             = CharacterKey::End;
  static constexpr CharacterKey PageDown        = CharacterKey::PageDown;
  static constexpr CharacterKey Right           = CharacterKey::RightArrow;
  static constexpr CharacterKey Left            = CharacterKey::LeftArrow;
  static constexpr CharacterKey Down            = CharacterKey::DownArrow;
  static constexpr CharacterKey Up              = CharacterKey::UpArrow;
  static constexpr CharacterKey NumLock         = CharacterKey::NumLock;
  static constexpr CharacterKey KeypadDivide    = CharacterKey::KeypadDivide;
  static constexpr CharacterKey KeypadMultiply  = CharacterKey::KeypadMultiply;
  static constexpr CharacterKey KeypadSubtract  = CharacterKey::KeypadSubtract;
  static constexpr CharacterKey KeypadAdd       = CharacterKey::KeypadAdd;
  static constexpr CharacterKey KeypadEnter     = CharacterKey::KeypadEnter;
  static constexpr CharacterKey Keypad1         = CharacterKey::Keypad1;
  static constexpr CharacterKey Keypad2         = CharacterKey::Keypad2;
  static constexpr CharacterKey Keypad3         = CharacterKey::Keypad3;
  static constexpr CharacterKey Keypad4         = CharacterKey::Keypad4;
  static constexpr CharacterKey Keypad5         = CharacterKey::Keypad5;
  static constexpr CharacterKey Keypad6         = CharacterKey::Keypad6;
  static constexpr CharacterKey Keypad7         = CharacterKey::Keypad7;
  static constexpr CharacterKey Keypad8         = CharacterKey::Keypad8;
  static constexpr CharacterKey Keypad9         = CharacterKey::Keypad9;
  static constexpr CharacterKey Keypad0         = CharacterKey::Keypad0;
  static constexpr CharacterKey KeypadPeriod    = CharacterKey::KeypadPeriod;
  static constexpr CharacterKey NonUsBackslash  = CharacterKey::NonUsBackslash;
  static constexpr CharacterKey Application     = CharacterKey::Application;
  static constexpr CharacterKey Power           = CharacterKey::Power;
  static constexpr CharacterKey KeypadEqual     = CharacterKey::KeypadEqual;
  static constexpr CharacterKey F13             = CharacterKey::F13;
  static constexpr CharacterKey F14             = CharacterKey::F14;
  static constexpr CharacterKey F15             = CharacterKey::F15;
  static constexpr CharacterKey F16             = CharacterKey::F16;
  static constexpr CharacterKey F17             = CharacterKey::F17;
  static constexpr CharacterKey F18             = CharacterKey::F18;
  static constexpr CharacterKey F19             = CharacterKey::F19;
  static constexpr CharacterKey F20             = CharacterKey::F20;
  static constexpr CharacterKey F21             = CharacterKey::F21;
  static constexpr CharacterKey F22             = CharacterKey::F22;
  static constexpr CharacterKey F23             = CharacterKey::F23;
  static constexpr CharacterKey F24             = CharacterKey::F24;

  static constexpr CharacterKey KeypadComma     = CharacterKey::KeypadComma;

  static constexpr CharacterKey Int1            = CharacterKey::Int1;
  static constexpr CharacterKey Int2            = CharacterKey::Int2;
  static constexpr CharacterKey Int3            = CharacterKey::Int3;
  static constexpr CharacterKey Int4            = CharacterKey::Int4;
  static constexpr CharacterKey Int5            = CharacterKey::Int5;
  static constexpr CharacterKey Int6            = CharacterKey::Int6;
  static constexpr CharacterKey Int7            = CharacterKey::Int7;
  static constexpr CharacterKey Int8            = CharacterKey::Int8;
  static constexpr CharacterKey Int9            = CharacterKey::Int9;
  static constexpr CharacterKey Lang1           = CharacterKey::Lang1;
  static constexpr CharacterKey Lang2           = CharacterKey::Lang2;
  static constexpr CharacterKey Lang3           = CharacterKey::Lang3;
  static constexpr CharacterKey Lang4           = CharacterKey::Lang4;
  static constexpr CharacterKey Lang5           = CharacterKey::Lang5;
  static constexpr CharacterKey Lang6           = CharacterKey::Lang6;
  static constexpr CharacterKey Lang7           = CharacterKey::Lang7;
  static constexpr CharacterKey Lang8           = CharacterKey::Lang8;
  static constexpr CharacterKey Lang9           = CharacterKey::Lang9;

  static constexpr Modifiers Ctrl               = Modifiers::LeftCtrl;
  static constexpr Modifiers Shift              = Modifiers::LeftShift;
  static constexpr Modifiers Alt                = Modifiers::LeftAlt;
  static constexpr Modifiers Gui                = Modifiers::LeftGui;
  static constexpr Modifiers LeftCtrl           = Modifiers::LeftCtrl;
  static constexpr Modifiers LeftShift          = Modifiers::LeftShift;
  static constexpr Modifiers LeftAlt            = Modifiers::LeftAlt;
  static constexpr Modifiers LeftGui            = Modifiers::LeftGui;
  static constexpr Modifiers RightCtrl          = Modifiers::RightCtrl;
  static constexpr Modifiers RightShift         = Modifiers::RightShift;
  static constexpr Modifiers RightAlt           = Modifiers::RightAlt;
  static constexpr Modifiers RightGui           = Modifiers::RightGui;
 
  static constexpr MouseButtons LeftButton      = MouseButtons::Left;
  static constexpr MouseButtons RightButton     = MouseButtons::Right;
  static constexpr MouseButtons MiddleButton    = MouseButtons::Middle;
  static constexpr MouseButtons BackwardButton  = MouseButtons::Backward;
  static constexpr MouseButtons ForwardButton   = MouseButtons::Forward;

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
