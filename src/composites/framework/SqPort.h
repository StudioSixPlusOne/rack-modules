/*
 MIT License

Copyright (c) 2018 squinkylabs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#pragma once

/**
 * This awful hack is so that both the real plugin and
 * the unit tests can pass this "Output" struct around
 * 
 * TODO: move to a common include file??
 */

#ifdef __PLUGIN
namespace rack
{
    namespace engine
    {
        struct Input;
        struct Param;
        struct Output;
    } // namespace engine
} // namespace rack
#else
#include "TestComposite.h"
#endif

#ifdef __PLUGIN
using SqInput = rack::engine::Input;
using SqOutput = rack::engine::Output;
using SqParam = rack::engine::Param;
#else
using SqOutput = ::Output;
using SqInput = ::Input;
using SqParam = ::Param;
#endif