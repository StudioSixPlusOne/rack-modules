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


#include "MeasureTime.h"

#include "asserts.h"

#include "simd/functions.hpp"
#include "simd/sse_mathfun.h"
#include "simd/sse_mathfun_extension.h"



double overheadInOut = 0;
double overheadInOut4 = 0;
double overheadOutOnly = 0;

static void setup()
{
#ifdef _DEBUG
//    assert(false);  // don't run this in debug
#endif
    double d = .1;
    const double scale = 1.0 / RAND_MAX;
    overheadInOut = MeasureTime<float>::run(0.0, "test1 (do nothing i/o)", [&d, scale]() {
        return TestBuffers<float>::get();
        }, 1);


    overheadInOut4 = MeasureTime<float>::run(0.0, "test1 (do nothing i/o float 4)", [&d, scale]() {
            rack::simd::float_4 f4;
            f4[0] = TestBuffers<float>::get();
            return f4[0];
        }, 1);

    overheadOutOnly = MeasureTime<float>::run(0.0, "test1 (do nothing oo)", [&d, scale]() {
        return 0.0f;
        }, 1);

}

void initPerf()
{
    printf("initializing perf...\n");
    assert(overheadInOut == 0);
    assert(overheadOutOnly == 0);
    setup();
    assert(overheadInOut > 0);
    assert(overheadOutOnly > 0);
}