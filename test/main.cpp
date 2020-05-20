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

#include <assert.h>
#include <stdio.h>
#include <string>

// external tests
extern void testEmpty();
extern void testAudioMath();
extern void testCircularBuffer();
extern void testLookupTable();
extern void testAnalyzer();
extern void testPolyShiftRegister();
extern void testTestSignal();
extern void testKSDelay();
extern void testCombFilter();
extern void testSynthFilter();
extern void testMaccomo();

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

    testEmpty();
    testTestSignal();
    testAudioMath();
    testCircularBuffer();
    testLookupTable();
    testAnalyzer();
    testPolyShiftRegister();
    testKSDelay();
    testCombFilter();
    testSynthFilter();
    testMaccomo();

    printf ("Tests passed.\n");
}
