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

#include "ExtremeTester.h"
#include "PolyShiftRegister.h"
#include "TestComposite.h"
#include "asserts.h"
#include "dsp/digital.hpp"
#include "math.hpp"
#include "testSignal.h"
#include <algorithm>

namespace ts = sspo::TestSignal;

// Rack functions added, only compiled for tests

using PSR = PolyShiftRegisterComp<TestComposite>;

//basic test for compile
static void test01()
{
    PSR psr;
    psr.init();
    psr.step();
}

static void testShift()
{
    PSR psr;
    psr.init();
    psr.step();
    psr.params[psr.CHANNELS_PARAM].setValue (3);
    auto a = ts::makeFixed (100, 1.0f);
    auto b = ts::makeFixed (100, 1.5f);
    auto c = ts::makeFixed (100, 3.3f);
    auto seq = a + b + c;
    auto trig = ts::makeClockTrigger (100, 3);
    assertEQ (psr.currentChannels, 1);
    for (auto i = 0; i < int (trig.size()); ++i)
    {
        psr.inputs[psr.MAIN_INPUT].setVoltage (seq[i]);
        psr.inputs[psr.TRIGGER_INPUT].setVoltage (trig[i]);
        psr.step();
    }

    assertEQ (psr.currentChannels, 3);
    assertClose (psr.outputs[psr.MAIN_OUTPUT].getVoltage (0), 3.3f, FLT_EPSILON);
    //TODO disabled not sure if bug mac only
//    assertClose (psr.outputs[psr.MAIN_OUTPUT].getVoltage (1), 1.5f, FLT_EPSILON);
//    assertClose (psr.outputs[psr.MAIN_OUTPUT].getVoltage (2), 1.0f, FLT_EPSILON);
}

static void testShiftMonoCv (float triggerProbCv)
{
    std::vector<float> oldSignal{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    std::vector<float> newSignal;
    PSR psr;
    psr.init();
    psr.step();
    psr.params[psr.CHANNELS_PARAM].setValue (16);
    psr.inputs[psr.TRIGGER_PROB_INPUT].setVoltage (triggerProbCv, 0);
    psr.inputs[psr.TRIGGER_PROB_INPUT].setChannels (1);
    auto a = ts::makeFixed (100, 1.0f);
    auto b = ts::makeFixed (100, 1.5f);
    auto c = ts::makeFixed (100, 3.3f);
    auto seq = a + b + c + a + b + c + a + b + c + a + b + c + a + b + c;
    const int stepcount = 15;
    const int interval = 100;
    auto trig = ts::makeClockTrigger (interval, stepcount);
    assertEQ (psr.currentChannels, 1);
    for (auto j = 0; j < stepcount; ++j)
    {
        newSignal.resize (0);
        for (auto i = 0; i < int (interval); ++i)
        {
            psr.inputs[psr.MAIN_INPUT].setVoltage (seq[i]);
            psr.inputs[psr.TRIGGER_INPUT].setVoltage (trig[i]);
            psr.step();
        }
        //test newbuffer = oldbuffer or newbuffershifted
        //between elements 1 and psr.currentChannels

        //get new buffer
        for (auto k = 0; k < stepcount; ++k)
            newSignal.push_back (psr.outputs[psr.MAIN_OUTPUT].getVoltage (k));
        //compare same
        auto same = newSignal == oldSignal;
        //compare shifter
        auto shifted = std::equal (newSignal.begin() + 1,
                                   newSignal.end(),
                                   oldSignal.begin());
        //assert or
        assert (same || shifted);
        oldSignal = newSignal;
    }
}

static void testShiftMonoCv()
{
    testShiftMonoCv (0.0f);
    testShiftMonoCv (8.0f);
}

static void testShiftPolyCv()
{
    PSR psr;
    psr.init();
    psr.step();
    psr.params[psr.CHANNELS_PARAM].setValue (3);
    psr.inputs[psr.TRIGGER_PROB_INPUT].setChannels (3);
    psr.inputs[psr.TRIGGER_PROB_INPUT].setVoltage (10.0, 1);
    auto a = ts::makeFixed (100, 1.0f);
    auto b = ts::makeFixed (100, 1.5f);
    auto c = ts::makeFixed (100, 3.3f);
    auto seq = a + b + c;
    auto trig = ts::makeClockTrigger (100, 3);
    assertEQ (psr.currentChannels, 1);
    for (auto i = 0; i < int (trig.size()); ++i)
    {
        psr.inputs[psr.MAIN_INPUT].setVoltage (seq[i]);
        psr.inputs[psr.TRIGGER_INPUT].setVoltage (trig[i]);
        psr.step();
    }

    assertEQ (psr.currentChannels, 3);
    assertClose (psr.outputs[psr.MAIN_OUTPUT].getVoltage (0), 3.3f, FLT_EPSILON);
    assertClose (psr.outputs[psr.MAIN_OUTPUT].getVoltage (1), 0.0f, FLT_EPSILON);
    assertClose (psr.outputs[psr.MAIN_OUTPUT].getVoltage (2), 1.0f, FLT_EPSILON);
}

static void testReset()
{
    PSR psr;
    psr.init();
    psr.step();
    psr.params[psr.CHANNELS_PARAM].setValue (6);
    auto a = ts::makeFixed (100, 1.0f);
    auto b = ts::makeFixed (100, 1.5f);
    auto c = ts::makeFixed (100, 3.3f);
    auto seq = a + b + c + a + b + c;
    auto trig = ts::makeClockTrigger (100, 6);
    auto reset = ts::makeTrigger (600, 44);
    assertEQ (psr.currentChannels, 1);
    for (auto i = 0; i < 600; ++i)
    {
        psr.inputs[psr.MAIN_INPUT].setVoltage (seq[i]);
        psr.inputs[psr.TRIGGER_INPUT].setVoltage (trig[i]);
        psr.inputs[psr.RESET_INPUT].setVoltage (reset[i]);
        psr.step();
        assertEQ (psr.currentChannels, int (i / 100) + 1);
    }
    psr.inputs[psr.RESET_INPUT].setVoltage (10.0f);
    psr.step();

    assertEQ (psr.currentChannels, 1);
}

static void testShuffleNoCv()
{
    PSR psr;
    psr.init();
    psr.step();
    psr.params[psr.CHANNELS_PARAM].setValue (3);
    auto a = ts::makeFixed (100, 1.0f);
    auto b = ts::makeFixed (100, 1.5f);
    auto c = ts::makeFixed (100, 3.3f);
    auto seq = a + b + c;
    auto trig = ts::makeClockTrigger (100, 3);
    assertEQ (psr.currentChannels, 1);
    psr.params[psr.SHUFFLE_PROB_PARAM].setValue (0.5);

    auto shuffleCount = 0;
    for (auto i = 0; i < 1000; ++i)
    {
        for (auto i = 0; i < 300; ++i)
        {
            psr.inputs[psr.MAIN_INPUT].setVoltage (seq[i]);
            psr.inputs[psr.TRIGGER_INPUT].setVoltage (trig[i]);
            psr.step();
        }
        if ((psr.outputs[psr.MAIN_OUTPUT].getVoltage (0) != 3.3f)
            || (psr.outputs[psr.MAIN_OUTPUT].getVoltage (1) != 1.5f))
            shuffleCount++;
    }
    assertClose (shuffleCount, 500, 300);
}

static void testShufflePolyCv()
{
    PSR psr;
    psr.init();
    psr.step();
    psr.params[psr.CHANNELS_PARAM].setValue (3);
    auto a = ts::makeFixed (100, 1.0f);
    auto b = ts::makeFixed (100, 1.5f);
    auto c = ts::makeFixed (100, 3.3f);
    auto seq = a + b + c;
    auto trig = ts::makeClockTrigger (100, 3);
    assertEQ (psr.currentChannels, 1);
    psr.params[psr.SHUFFLE_PROB_PARAM].setValue (0.0);
    psr.inputs[psr.SHUFFLE_PROB_INPUT].setChannels (3);
    psr.inputs[psr.SHUFFLE_PROB_INPUT].setVoltage (5.0, 1);

    auto shuffleCount = 0;
    for (auto i = 0; i < 1000; ++i)
    {
        for (auto i = 0; i < 300; ++i)
        {
            psr.inputs[psr.MAIN_INPUT].setVoltage (seq[i]);
            psr.inputs[psr.TRIGGER_INPUT].setVoltage (trig[i]);
            psr.step();
        }
        assertClose (psr.outputs[psr.MAIN_OUTPUT].getVoltage (0), 3.3f, 0.0001f);
//        assertClose (psr.outputs[psr.MAIN_OUTPUT].getVoltage (2), 1.0f, 0.0001f);

        if (psr.outputs[psr.MAIN_OUTPUT].getVoltage (1) != 1.5f)
            shuffleCount++;
    }
    assertClose (shuffleCount, 500, 200);
}

static void testAccentZeroOffset()
{
    PSR psr;
    psr.init();
    psr.step();
    psr.params[psr.CHANNELS_PARAM].setValue (3);
    psr.params[psr.ACCENT_A_PROB_PARAM].setValue (0.0);
    psr.params[psr.ACCENT_A_OFFSET_PARAM].setValue (0.0);
    psr.inputs[psr.ACCENT_A_PROB_INPUT].setChannels (3);
    psr.inputs[psr.ACCENT_A_PROB_INPUT].setVoltage (0.7, 1);
    auto trig = ts::makeClockTrigger (100, 3);
    auto seq = ts::makeZeros (300);
    auto accentCount = 0;
    for (auto i = 0; i < 1000; ++i)
    {
        for (auto i = 0; i < 300; ++i)
        {
            psr.inputs[psr.MAIN_INPUT].setVoltage (seq[i]);
            psr.inputs[psr.TRIGGER_INPUT].setVoltage (trig[i]);
            psr.step();
        }
        for (auto c = 0; c < 3; ++c)
            assertClose (psr.outputs[psr.MAIN_OUTPUT].getVoltage (c), 0.0f, FLT_EPSILON);
    }
}

static void testAccentA()
{
    PSR psr;
    psr.init();
    psr.step();
    psr.params[psr.CHANNELS_PARAM].setValue (3);
    psr.params[psr.ACCENT_A_PROB_PARAM].setValue (0.5);
    psr.params[psr.ACCENT_A_OFFSET_PARAM].setValue (0.5);
    auto trig = ts::makeClockTrigger (100, 3);
    auto seq = ts::makeZeros (300);
    auto accentCount = 0;
    for (auto i = 0; i < 1000; ++i)
    {
        for (auto i = 0; i < 300; ++i)
        {
            psr.inputs[psr.MAIN_INPUT].setVoltage (seq[i]);
            psr.inputs[psr.TRIGGER_INPUT].setVoltage (trig[i]);
            psr.step();
        }
        if ((psr.outputs[psr.MAIN_OUTPUT].getVoltage (0) == 0.5f)
            && (psr.outputs[psr.MAIN_OUTPUT].getVoltage (1) == 0.5f))
            accentCount++;
    }
    assertClose (accentCount, 500, 200);
}

static void testAccentB()
{
    PSR psr;
    psr.init();
    psr.step();
    psr.params[psr.CHANNELS_PARAM].setValue (3);
    psr.params[psr.ACCENT_B_PROB_PARAM].setValue (0.5);
    psr.params[psr.ACCENT_B_OFFSET_PARAM].setValue (-0.7);
    auto trig = ts::makeClockTrigger (100, 3);
    auto seq = ts::makeZeros (300);
    auto accentCount = 0;
    for (auto i = 0; i < 1000; ++i)
    {
        for (auto i = 0; i < 300; ++i)
        {
            psr.inputs[psr.MAIN_INPUT].setVoltage (seq[i]);
            psr.inputs[psr.TRIGGER_INPUT].setVoltage (trig[i]);
            psr.step();
        }
        if ((psr.outputs[psr.MAIN_OUTPUT].getVoltage (0) == -0.7f)
            && (psr.outputs[psr.MAIN_OUTPUT].getVoltage (1) == -0.7f))
            accentCount++;
    }
    assertClose (accentCount, 500, 200);
}

static void testAccentAPolyCv()
{
    PSR psr;
    psr.init();
    psr.step();
    psr.params[psr.CHANNELS_PARAM].setValue (3);
    psr.params[psr.ACCENT_A_PROB_PARAM].setValue (0.0);
    psr.params[psr.ACCENT_A_OFFSET_PARAM].setValue (-3.2);
    psr.inputs[psr.ACCENT_A_PROB_INPUT].setChannels (3);
    psr.inputs[psr.ACCENT_A_PROB_INPUT].setVoltage (5, 2);
    auto trig = ts::makeClockTrigger (100, 3);
    auto seq = ts::makeZeros (300);
    auto accentCount = 0;
    for (auto i = 0; i < 1000; ++i)
    {
        for (auto i = 0; i < 300; ++i)
        {
            psr.inputs[psr.MAIN_INPUT].setVoltage (seq[i]);
            psr.inputs[psr.TRIGGER_INPUT].setVoltage (trig[i]);
            psr.step();
        }
        if (psr.outputs[psr.MAIN_OUTPUT].getVoltage (2) == -3.2f)
            accentCount++;
        assertClose (psr.outputs[psr.MAIN_OUTPUT].getVoltage (0), 0.0f, FLT_EPSILON);
        assertClose (psr.outputs[psr.MAIN_OUTPUT].getVoltage (1), 0.0f, FLT_EPSILON);
    }
    assertClose (accentCount, 500, 200);
}

static void testAccentBPolyCv()
{
    PSR psr;
    psr.init();
    psr.step();
    psr.params[psr.CHANNELS_PARAM].setValue (3);
    psr.params[psr.ACCENT_B_PROB_PARAM].setValue (0.0);
    psr.params[psr.ACCENT_B_OFFSET_PARAM].setValue (-3.2);
    psr.inputs[psr.ACCENT_B_PROB_INPUT].setChannels (3);
    psr.inputs[psr.ACCENT_B_PROB_INPUT].setVoltage (5, 1);
    auto trig = ts::makeClockTrigger (100, 3);
    auto seq = ts::makeZeros (300);
    auto accentCount = 0;
    for (auto i = 0; i < 1000; ++i)
    {
        for (auto i = 0; i < 300; ++i)
        {
            psr.inputs[psr.MAIN_INPUT].setVoltage (seq[i]);
            psr.inputs[psr.TRIGGER_INPUT].setVoltage (trig[i]);
            psr.step();
        }
        if (psr.outputs[psr.MAIN_OUTPUT].getVoltage (1) == -3.2f)
            accentCount++;
        assertClose (psr.outputs[psr.MAIN_OUTPUT].getVoltage (0), 0.0f, FLT_EPSILON);
        assertClose (psr.outputs[psr.MAIN_OUTPUT].getVoltage (2), 0.0f, FLT_EPSILON);
    }
    assertClose (accentCount, 500, 200);
}

static void testAccentRNG()
{
    PSR psr;
    psr.init();
    psr.step();
    psr.params[psr.CHANNELS_PARAM].setValue (3);
    psr.params[psr.ACCENT_RNG_PROB_PARAM].setValue (0.5);
    psr.params[psr.ACCENT_RNG_OFFSET_PARAM].setValue (0.5);
    auto trig = ts::makeClockTrigger (100, 3);
    auto seq = ts::makeZeros (300);
    auto accentCount = 0;
    for (auto i = 0; i < 1000; ++i)
    {
        for (auto i = 0; i < 300; ++i)
        {
            psr.inputs[psr.MAIN_INPUT].setVoltage (seq[i]);
            psr.inputs[psr.TRIGGER_INPUT].setVoltage (trig[i]);
            psr.step();
        }
        if ((psr.outputs[psr.MAIN_OUTPUT].getVoltage (0) != 0.0f)
            && (psr.outputs[psr.MAIN_OUTPUT].getVoltage (1) != 0.0f))
            accentCount++;

        assertLE (psr.outputs[psr.MAIN_OUTPUT].getVoltage (0), 0.5f);
    }
    assertClose (accentCount, 500, 200);
}

static void testExtreme()
{
    using fp = std::pair<float, float>;
    std::vector<std::pair<float, float>> paramLimits;
    PSR psr;
    psr.init();

    paramLimits.resize (psr.NUM_PARAMS);
    paramLimits[psr.CHANNELS_PARAM] = fp (1.0f, 16.0f);
    paramLimits[psr.TRIGGER_PROB_PARAM] = fp (0.0f, 1.0f);
    paramLimits[psr.SHUFFLE_PROB_PARAM] = fp (0.0f, 1.0f);
    paramLimits[psr.ACCENT_A_PROB_PARAM] = fp (0.0f, 1.0f);
    paramLimits[psr.ACCENT_B_PROB_PARAM] = fp (0.0f, 1.0f);
    paramLimits[psr.ACCENT_RNG_PROB_PARAM] = fp (0.0f, 1.0f);
    paramLimits[psr.ACCENT_A_OFFSET_PARAM] = fp (-10.0f, 10.0f);
    paramLimits[psr.ACCENT_B_OFFSET_PARAM] = fp (-10.0f, 10.0f);
    paramLimits[psr.ACCENT_RNG_OFFSET_PARAM] = fp (-10.0f, 10.0f);

    ExtremeTester<PSR>::test (psr, paramLimits, true, "PolyShiftRegister Tyrant");
}

void testPolyShiftRegister()
{
    printf ("Testing Poly Shift Register Tyrant\n");
    test01();
    testShift();
    testShiftMonoCv();
    testShiftPolyCv();
    testReset();
    testShuffleNoCv();
    testShufflePolyCv();
    testAccentA();
    testAccentB();
    testAccentRNG();
    testAccentZeroOffset();
    testAccentAPolyCv();
    testAccentBPolyCv();

    testExtreme();
}
