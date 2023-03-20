/*
* Copyright (c) 2023 Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>
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

#include <array>
#include <assert.h>
#include <stdio.h>
#include "dsp/digital.hpp"
#include "TestComposite.h"
#include "ExtremeTester.h"
#include "Analyzer.h"
#include "testSignal.h"
#include "vcvSampleRates.h"

#include "BpmDetector.h"

namespace ts = sspo::TestSignal;

auto bpms = { 66.4f, 90.4f, 100.0f, 118.0f, 123.0f, 128.4f, 137.6f, 145.9f, 180.5f, 200.7f, 300.1f };

static void testConstructor()
{
    sspo::BpmDetector testclass;
    assertEQ (1, 1);
}

void testSteadyClock (float BPM, float SAMPLE_RATE, int count)
{
    sspo::BpmDetector bmpDetector;

    bmpDetector.setSampleRate (SAMPLE_RATE);

    //make clock
    auto clockSignal = sspo::TestSignal::makeClockTrigger (60.0f / BPM * SAMPLE_RATE, count);

    //process
    float result = 0;

    for (auto cs : clockSignal)
    {
        result = bmpDetector.process (cs);
    }

    assertClose (result, 60.0f / BPM, 0.03f);
}

void testSteadyClock()
{
    for (auto sr : Sspo::sampleRates)
    {
        for (auto bpm : bpms)
        {
            testSteadyClock (bpm, sr, 8);
        }
    }
}

void testSineClock (float BPM, float SAMPLE_RATE, int count)
{
    sspo::BpmDetector bmpDetector;

    bmpDetector.setSampleRate (SAMPLE_RATE);

    //make clock
    auto clockSignal = sspo::TestSignal::makeSine ((BPM / 60.0f) * SAMPLE_RATE * count, (BPM / 60.0f), SAMPLE_RATE, 5.0f);

    //process
    float result = 0;

    for (auto cs : clockSignal)
    {
        result = bmpDetector.process (cs);
    }

#if 0

    printf ("Bmp %f, SR %f, Actual %f, Expected %f\n", BPM, SAMPLE_RATE, result, 60.0f / BPM);

#endif
    assertClose (result, 60.0f / BPM, 0.03f);
}

void testSineClock()
{
    for (auto sr : Sspo::sampleRates)
    {
        for (auto bpm : bpms)
        {
            testSineClock (bpm, sr, 8);
        }
    }
}

void testNewClockFirstTwoLeadingEdge()
{
    printf ("testNewClockFirstTwoLeadingEdge`n");
    for (auto sr : Sspo::sampleRates)
    {
        for (auto bpm : bpms)
        {
            testSteadyClock (bpm, sr, 2);
        }
    }
}

void testSaneOutputWithNoHistory()
{
    sspo::BpmDetector bpmDetector;

    auto result = bpmDetector.process (0.0f);

    assertClose (result, 0.5f, 0.25f);
}

static void testSampleRateChange()
{
    float SAMPLE_RATE = 44100.0f;
    float BPM = 127.34;
    int count = 8;

    sspo::BpmDetector bmpDetector;

    bmpDetector.setSampleRate (SAMPLE_RATE);

    //make clock
    auto clockSignal = sspo::TestSignal::makeClockTrigger (60.0f / BPM * SAMPLE_RATE, count / 2);

    //process
    float result = 0;

    for (auto cs : clockSignal)
    {
        result = bmpDetector.process (cs);
    }

    SAMPLE_RATE = 48000.0f;
    clockSignal = sspo::TestSignal::makeClockTrigger (60.0f / BPM * SAMPLE_RATE, count / 2);
    bmpDetector.setSampleRate (SAMPLE_RATE);

    //process

    for (auto cs : clockSignal)
    {
        result = bmpDetector.process (cs);
    }

    assertClose (result, 60.0f / BPM, 0.03f);
}

void testReset()
{
    float SAMPLE_RATE = 44100.0f;
    float BPM = 99.63f;
    int count = 8;

    sspo::BpmDetector bmpDetector;

    bmpDetector.setSampleRate (SAMPLE_RATE);

    //make clock
    auto clockSignal = sspo::TestSignal::makeClockTrigger (60.0f / BPM * SAMPLE_RATE, count / 2);

    //process
    float result = 0;

    for (auto cs : clockSignal)
    {
        result = bmpDetector.process (cs);
    }

    bmpDetector.reset();
    result = bmpDetector.process (0.0f);

    assertClose (result, 60.0f / BPM, 0.03f);
}

void testWobbleClock()
{
    assert (false);
}

void testBpmDetector()
{
    printf ("test BpmDetector\n");
    testSteadyClock();
    testSineClock();
    testNewClockFirstTwoLeadingEdge();
    testSaneOutputWithNoHistory();
    testSampleRateChange();
    testReset();
    testConstructor();
    //    testWobbleClock();
}