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

#include <assert.h>
/**
 * @class SqTime
 * A simple high-res timer. Returns current seconds
 */

/**
  * Windows version is based on QueryPerformanceFrequency
  * Typically this is accurate to 1/3 microsecond. Can be made more
  * accurate by tinkering with your bios
  */

#if defined(_MSC_VER) || defined(ARCH_WIN)
//#if defined(_MSC_VER)
#define _USE_WINDOWS_PERFTIME
#include <Windows.h>
class SqTime
{
public:
    static double seconds()
    {
        LARGE_INTEGER t;
        if (frequency == 0)
        {
            QueryPerformanceFrequency (&t);
            frequency = double (t.QuadPart);
        }

        QueryPerformanceCounter (&t);
        int64_t n = t.QuadPart;
        return double (n) / frequency;
    }

private:
    static double frequency;
};

#else

#include <time.h>
class SqTime
{
public:
    static double seconds()
    {
        struct timespec ts;
        int x = clock_gettime (CLOCK_THREAD_CPUTIME_ID, &ts);
        assert (x == 0);
        (void) x;

        // seconds = sec + nsec / 10**9
        return double (ts.tv_sec) + double (ts.tv_nsec) / (1000.0 * 1000.0 * 1000.0);
    }
};
#endif
