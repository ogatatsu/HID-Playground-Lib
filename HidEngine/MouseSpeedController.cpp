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

#include "MouseSpeedController.h"
#include <stdlib.h>

namespace hidpg
{

/*------------------------------------------------------------------*/
/* Rational
 *------------------------------------------------------------------*/
Rational::Rational(int number, int denom) : _number(number), _denom(denom)
{
  normalize();
}

int Rational::number() const
{
  return _number;
}

int Rational::denom() const
{
  return _denom;
}

Rational &Rational::operator*=(const Rational &rhs)
{
  _denom *= rhs.denom();
  _number *= rhs.number();
  normalize();
  return *this;
}

Rational &Rational::operator/=(const Rational &rhs)
{
  _denom *= rhs.number();
  _number *= rhs.denom();
  normalize();
  return *this;
}

static int gcd(int a, int b)
{
  if (b == 0)
  {
    return a;
  }
  else
  {
    return gcd(b, a % b);
  }
}

void Rational::normalize()
{
  int g = gcd(abs(_number), abs(_denom));
  _number = _number / g;
  _denom = _denom / g;

  if (_denom < 0)
  {
    _denom = -_denom;
    _number = -_number;
  }
}

/*------------------------------------------------------------------*/
/* MouseSpeedController
 *------------------------------------------------------------------*/
Rational MouseSpeedController::_rational(1, 1);
uint8_t MouseSpeedController::_zeroCounter = 0;

void MouseSpeedController::accel(int16_t percent)
{
  _rational *= Rational(percent, 100);
}

void MouseSpeedController::decel(int16_t percent)
{
  _rational /= Rational(percent, 100);
}

void MouseSpeedController::setZero()
{
  _zeroCounter++;
}

void MouseSpeedController::unsetZero()
{
  _zeroCounter--;
}

double MouseSpeedController::getfactor()
{
  if (_zeroCounter == 0)
  {
    return static_cast<double>(_rational.number() / _rational.denom());
  }
  else
  {
    return 0;
  }
}

} // namespace hidpg
