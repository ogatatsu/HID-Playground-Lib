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

#include "MemStore.h"

namespace hidpg
{

MemStore::MemStore(const char *directory) : _directory(directory), _file(InternalFS)
{
}

void MemStore::init()
{
  InternalFS.begin();
  if (InternalFS.exists(_directory.c_str()) == false)
  {
    InternalFS.mkdir(_directory.c_str());
  }
}

bool MemStore::load(const char *name, void *buf, size_t size)
{
  bool result = false;

  String path = _directory + '/' + name;

  _file.open(path.c_str(), FILE_O_READ);

  if (_file)
  {
    if (size == _file.size())
    {
      _file.read(buf, size);
      result = true;
    }
    _file.close();
  }

  return result;
}

void MemStore::save(const char *name, const void *buf, size_t size)
{
  String path = _directory + '/' + name;

  if (InternalFS.exists(path.c_str()))
  {
    InternalFS.remove(path.c_str());
  }

  _file.open(path.c_str(), FILE_O_WRITE);
  _file.write(static_cast<const uint8_t *>(buf), size);
  _file.close();
}

bool MemStore::remove(const char *name)
{
  String path = _directory + '/' + name;

  if (InternalFS.exists(path.c_str()))
  {
    return InternalFS.remove(path.c_str());
  }
  return false;
}

void MemStore::clear()
{
  InternalFS.rmdir_r(_directory.c_str());

  InternalFS.mkdir(_directory.c_str());
}

} // namespace hidpg
