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

using float_4 = rack::simd::float_4;
using namespace rack;

// external tests
// ADD EXTERN
extern void testReverb();
        
extern void testMARS_Basic_Comb_Filter();
extern void testPatchNotes();
extern void testThru();
extern void testLalaStereo();
extern void testAdsr();
extern void testFarini();
extern void testDuffy();
extern void testSchmittTrigger_4();
extern void testSampleAndHold();
extern void testBose();
extern void testMix();
extern void testSynthFilter();
extern void testSynthFilterII();
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
extern void testBascom();
extern void testAmburgh();
extern void testSaturator();
extern void testUtilityFilter();
extern void testLala();
extern void testEva();
extern void testEasing();
extern void testZazel();
extern void testIverson();
extern void testTriggerSequencer();
extern void testWaveShaper();
extern void testHula();

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
        // ADD NEWTEST
    testReverb();
        
    testMARS_Basic_Comb_Filter();

    testPatchNotes(); //valgrind ok
    testThru(); //valgrind ok
    testLalaStereo(); //valgrind ok
    testAdsr(); //valgrind ok
    testFarini(); //valgrind ok
    testDuffy(); //valgrind ok
    testSchmittTrigger_4(); //volgrind ok
    testSampleAndHold(); //valgrind ok
    testBose(); //valgrind ok
    testMix(); //valgrind ok
    testBascom(); //valgrind ok
    testWaveShaper(); //valgrind ok
    testHula(); //valgrind Fail
    testAmburgh(); //valgrind ok
    testSynthFilter(); //valgrind ok
    testSynthFilterII(); //valgrind ok
    testTriggerSequencer(); //valgrind ok
    testIverson(); //valgrid ok
    testLala(); //valgrind ok
    testEva(); //valgrind ok
    testZazel(); //valgrind ok
    //    //    testEasing();
    testSaturator(); //valgrind ok
    testEmpty(); //valgrind ok
    testTestSignal(); //valgring ok
    testAudioMath(); //valgrind ok
    testCircularBuffer(); //valgring ok
    testLookupTable(); //valgring ok
    testAnalyzer(); //valgring ok
    //    testPolyShiftRegister();
    //    testKSDelay();
    testCombFilter(); //Fails Vailgrind
    testMaccomo(); //valgrin
    testUtilityFilter();

    printf ("Tests passed.\n");
}
