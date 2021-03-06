/*
 * Copyright (c) 2020 Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

//UNIT TEST entry
#include <stdio.h>
#include <assert.h>

#include <string>

#include "simd/functions.hpp"
#include "simd/sse_mathfun.h"
#include "simd/sse_mathfun_extension.h"
#include "simd/vector.hpp"

using float_4 = rack::simd::float_4;
using namespace rack;

// external tests
extern void testSynthFilter();
extern void testEva();
extern void testEmpty();
extern void testAudioMath();
extern void testCircularBuffer();
extern void testLookupTable();
extern void testAnalyzer();
extern void testPolyShiftRegister();
extern void testTestSignal();
extern void testKSDelay();
extern void testCombFilter();
extern void testMaccomo();
extern void testSaturator();
extern void testUtilityFilter();
extern void testLala();
extern void testEva();
extern void testEasing();
extern void testZazel();
extern void testIverson();
extern void testTriggerSequencer();

//external performance tests
extern void initPerf();
extern void perfTest();

int main (int argc, char** argv)
{
    bool runPerf = false;

    if (argc > 1)
    {
        std::string arg = argv[1];
        if (arg == "--perf")
        {
            runPerf = true;
        }
        else
        {
            printf ("%s is not a valid command line argument\n", arg.c_str());
        }
    }
#ifdef _PERF
    runPerf = true;
#ifndef NDEBUG
#error asserts should be off for perf test
#endif
#endif

    //dont run 32bit
    assert (sizeof (size_t) == 8);

    if (runPerf)
    {
        initPerf();
        perfTest();
        return 0;
    }

    // run external tests defined above

    testSynthFilter();
    testTriggerSequencer();
    testIverson();
    testLala();
    testEva();
    testZazel();
    testEasing();
    testSaturator();
    testEmpty();
    testTestSignal();
    testAudioMath();
    testCircularBuffer();
    testLookupTable();
    testAnalyzer();
    testPolyShiftRegister();
    testKSDelay();
    testCombFilter();
    testMaccomo();
    testUtilityFilter();

    printf ("Tests passed.\n");
}
