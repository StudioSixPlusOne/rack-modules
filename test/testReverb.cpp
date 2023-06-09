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

#include "Reverb.h"

namespace ts = sspo::TestSignal;

static void testCombFilterConstructor()
{
    sspo::Reverb::CombFilter<float> testclass;
    assertEQ (1, 1);
}

static void testCombFilterNoFeedback()
{
    auto length = 1000;
    auto delayTime = 0.01f;
    auto feedback = 0.0f;
    auto sampleRate = 1000.0f;

    sspo::Reverb::CombFilter<float> cf;
    cf.setSampleRate (sampleRate);
    cf.setParameters (feedback, delayTime);

    auto driac = sspo::TestSignal::makeDriac (length);

    ts::Signal result;

    for (auto x : driac)
    {
        result.push_back (cf.step (x));
    }

    auto expectedInpuslseReturnIndex = delayTime * sampleRate;

    //check all zero except 1st delay

    for (auto i = 0; i < length; ++i)
    {
        if (i != expectedInpuslseReturnIndex + 1)
        {
            assertEQ (result[i], 0.0f);
        }
        else
        {
            assertEQ (result[i], 1.0f);
        }
    }
}

static void testCombFilterFeedback()
{
    auto length = 1000;
    auto delayTime = 0.01f;
    auto feedback = 0.9f;
    auto sampleRate = 1000.0f;

    sspo::Reverb::CombFilter<float> cf;
    cf.setSampleRate (sampleRate);
    cf.setParameters (feedback, delayTime);

    auto driac = sspo::TestSignal::makeDriac (length);

    ts::Signal result;

    for (auto x : driac)
    {
        result.push_back (cf.step (x));
    }

    int expectedImpulseReturnIndex = delayTime * sampleRate + 1;

    //check all zero except  delay and multiples

    for (auto i = 0; i < length; ++i)
    {
        if ((i % expectedImpulseReturnIndex != 0) || (i == 0))
        {
            assertEQ (result[i], 0.0f);
        }
        else
        {
            assertGT (result[i], 0.0f);
        }
    }
}

void testReverb()
{
    printf ("test Reverb\n");
    printf ("test Reverb Com filter");
    testCombFilterConstructor();
    testCombFilterNoFeedback();
    testCombFilterFeedback();
}