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

// An Zazel test

#include "TestComposite.h"
#include "ExtremeTester.h"
#include "Analyzer.h"
#include "testSignal.h"
#include "dsp/digital.hpp"
#include "math.hpp"
#include "Zazel.h"
#include <assert.h>
#include <stdio.h>

namespace ts = sspo::TestSignal;

// Rack functions added, only compiled for tests

using Zazel = ZazelComp<TestComposite>;

static void test01()
{
    Zazel zazel;
    zazel.setSampleRate (44100.0f);
    zazel.init();
    zazel.step();
}

static void testCycleFromFresh()
{
    Zazel zazel;
    auto sr = 44100.0f;
    zazel.setSampleRate (sr);
    auto freq = 100.0f;
    zazel.init();
    zazel.params[zazel.ONESHOT_PARAM].setValue (-0); //cycle mode
    zazel.inputs[zazel.CLOCK_INPUT].setChannels (0);
    zazel.params[zazel.DURATION_PARAM].setValue (1.0f / freq); //100hz
    zazel.params[zazel.START_PARAM].setValue (-1.0f);
    zazel.params[zazel.END_PARAM].setValue (1.0f);
    auto fftSize = 1024 * 16;
    ts::Signal sig;
    for (auto i = 0; i < fftSize; ++i)
    {
        zazel.step();
        sig.push_back (zazel.outputs[zazel.MAIN_OUTPUT].getVoltage());
        assertClose (zazel.lastClockDuration, 44100, 1);
    }

    auto response = ts::getResponse (sig);
    auto maxBin = Analyzer::getMax (response);
    auto targetBin = FFT::freqToBin (freq, sr, fftSize);
    assertClose (maxBin, targetBin, 1);
}

static void testOneShotAtInit()
{
    Zazel zazel;
    auto sr = 44100.0f;
    zazel.setSampleRate (sr);
    zazel.init();
    zazel.params[zazel.ONESHOT_PARAM].setValue (-1); //oneshot mode
    zazel.inputs[zazel.CLOCK_INPUT].setChannels (0);
    zazel.params[zazel.DURATION_PARAM].setValue (0.001); //sharp curve
    zazel.params[zazel.START_PARAM].setValue (-1.0f);
    zazel.params[zazel.END_PARAM].setValue (1.0f);
    auto t = ts::makeTrigger (1024, 512);
    auto trigger = t + t + t + t + t + t + t + t + t + t + t + t + t + t + t + t;

    auto fftSize = 1024 * 16;
    ts::Signal sig;
    for (auto i = 0; i < fftSize; ++i)
    {
        zazel.inputs[zazel.START_CONT_INPUT].setVoltage (trigger[i]);
        zazel.step();
        sig.push_back (zazel.outputs[zazel.MAIN_OUTPUT].getVoltage());
        assertClose (zazel.lastClockDuration, 44100, 1);
    }
    auto freq = sr / 2048;
    auto response = ts::getResponse (sig);
    auto maxBin = Analyzer::getMax (response);
    auto targetBin = FFT::freqToBin (freq, sr, fftSize);
    assertClose (maxBin, targetBin, 1);
}
static void testOneShotFlag()
{
    Zazel zazel;
    zazel.setSampleRate (20050);
    zazel.params[zazel.ONESHOT_PARAM].setValue (-1);
    zazel.step();
    assert (zazel.oneShot);
    zazel.params[zazel.ONESHOT_PARAM].setValue (0);
    zazel.step();
    assert (! zazel.oneShot);
}

static void testClockSpeed()
{
    Zazel zazel;
    auto sr = 44100.0f;
    zazel.setSampleRate (sr);
    zazel.init();
    zazel.inputs[zazel.CLOCK_INPUT].setChannels (1);
    auto t = ts::makeTrigger (1024, 512);
    auto trigger = t + t + t + t + t + t + t + t + t + t + t + t + t + t + t + t;

    for (auto x : trigger)
    {
        zazel.inputs[zazel.CLOCK_INPUT].setVoltage (x);
        zazel.step();
    }
    assertEQ (zazel.lastClockDuration, 1024);
}

static void testOneshotRiseTime()
{
    Zazel zazel;
    auto sr = 44100.0f;
    zazel.setSampleRate (sr);
    zazel.init();
    zazel.params[zazel.ONESHOT_PARAM].setValue (-1); //oneshot mode
    zazel.inputs[zazel.CLOCK_INPUT].setChannels (0);
    zazel.params[zazel.START_PARAM].setValue (-1.0f);
    zazel.params[zazel.END_PARAM].setValue (1.0f);
    auto t = ts::makeTrigger (1024, 512);
    auto trigger = t + t + t + t + t + t + t + t + t + t + t + t + t + t + t + t;
    zazel.params[zazel.DURATION_PARAM].setValue (1.0f / 1000);
    auto counter = 0;
    zazel.mode = Zazel::Mode::ONESHOT_LOW;
    //dont count first pass
    while (zazel.mode == Zazel::Mode::ONESHOT_ATTACK || zazel.mode == Zazel::Mode::PAUSED)
    {
        zazel.inputs[zazel.START_CONT_INPUT].setVoltage (trigger[counter]);
        zazel.step();
        counter++;
    }
    //run until attack
    while (zazel.mode != Zazel::Mode::ONESHOT_ATTACK)
    {
        zazel.inputs[zazel.START_CONT_INPUT].setVoltage (trigger[counter]);
        zazel.step();
        counter++;
    }

    counter = 0;
    while (zazel.mode == Zazel::Mode::ONESHOT_ATTACK || zazel.mode == Zazel::Mode::PAUSED)
    {
        zazel.inputs[zazel.START_CONT_INPUT].setVoltage (trigger[counter]);
        zazel.step();
        counter++;
    }

    assertEQ (counter, int (sr / 1000));
}
static void testOneshotFallTime()
{
    Zazel zazel;
    auto sr = 44100.0f;
    zazel.setSampleRate (sr);
    zazel.init();
    zazel.params[zazel.ONESHOT_PARAM].setValue (-1); //oneshot mode
    zazel.inputs[zazel.CLOCK_INPUT].setChannels (0);
    zazel.params[zazel.START_PARAM].setValue (-1.0f);
    zazel.params[zazel.END_PARAM].setValue (1.0f);
    auto t = ts::makeTrigger (1024, 512);
    auto trigger = t + t + t + t + t + t + t + t + t + t + t + t + t + t + t + t;
    zazel.params[zazel.DURATION_PARAM].setValue (1.0f / 1000);
    auto counter = 0;
    zazel.mode = Zazel::Mode::ONESHOT_LOW;
    //dont count until decay
    while (zazel.mode != Zazel::Mode::ONESHOT_DECAY)
    {
        zazel.inputs[zazel.START_CONT_INPUT].setVoltage (trigger[counter]);
        zazel.step();
        counter++;
    }
    //run until decay complete
    counter = 0;
    while (zazel.mode == Zazel::Mode::ONESHOT_DECAY)
    {
        zazel.inputs[zazel.START_CONT_INPUT].setVoltage (0);
        zazel.step();
        counter++;
    }
    assertEQ (counter, int (sr / 1000));
}
static void testPauseOneshotRising()
{
    Zazel zazel;
    auto sr = 44100.0f;
    zazel.setSampleRate (sr);
    zazel.init();
    zazel.params[zazel.ONESHOT_PARAM].setValue (-1); //oneshot mode
    zazel.inputs[zazel.CLOCK_INPUT].setChannels (0);
    zazel.params[zazel.START_PARAM].setValue (-1.0f);
    zazel.params[zazel.END_PARAM].setValue (1.0f);
    auto t = ts::makeTrigger (1024, 512);
    auto trigger = t + t + t + t + t + t + t + t + t + t + t + t + t + t + t + t;
    auto pauseLen = 100;
    auto pauseTrig = ts::makeTrigger (pauseLen, 44) + ts::makeTrigger (pauseLen, 44);
    zazel.params[zazel.DURATION_PARAM].setValue (1.0f / 1000);
    auto counter = 0;
    zazel.mode = Zazel::Mode::ONESHOT_LOW;
    //dont count first pass
    while (zazel.mode == Zazel::Mode::ONESHOT_ATTACK || zazel.mode == Zazel::Mode::PAUSED)
    {
        zazel.inputs[zazel.START_CONT_INPUT].setVoltage (trigger[counter]);
        zazel.step();
        counter++;
    }
    //run until attack
    while (zazel.mode != Zazel::Mode::ONESHOT_ATTACK)
    {
        zazel.inputs[zazel.START_CONT_INPUT].setVoltage (trigger[counter]);
        zazel.step();
        counter++;
    }

    auto attCounter = 0;
    //run until middle of attack
    for (auto i = 0; i < int (sr / 1000) / 2; ++i)

    {
        zazel.inputs[zazel.START_CONT_INPUT].setVoltage (trigger[counter]);
        zazel.step();
        counter++;
        attCounter++;
    }
    //test during pause value remains steady
    auto lastOut = zazel.outputs[zazel.MAIN_OUTPUT].getVoltage();
    for (auto i = 0; i <= pauseLen; ++i)
    {
        zazel.inputs[zazel.START_CONT_INPUT].setVoltage (trigger[counter]);
        zazel.inputs[zazel.STOP_CONT_INPUT].setVoltage (pauseTrig[i]);
        zazel.step();
        counter++;
        auto out = zazel.outputs[zazel.MAIN_OUTPUT].getVoltage();
        assertClose (out, lastOut, 0.000001f);
    }
    attCounter++; // one frame out of pause in last pause loop
    //remainder of attack phase;
    while (zazel.mode == Zazel::Mode::ONESHOT_ATTACK)
    {
        zazel.inputs[zazel.START_CONT_INPUT].setVoltage (trigger[counter]);
        zazel.step();
        counter++;
        attCounter++;
    }
    assertEQ (attCounter, int (sr / 1000));
}

static void testPauseOneshotFalling()
{
    Zazel zazel;
    auto sr = 44100.0f;
    zazel.setSampleRate (sr);
    zazel.init();
    zazel.params[zazel.ONESHOT_PARAM].setValue (-1); //oneshot mode
    zazel.inputs[zazel.CLOCK_INPUT].setChannels (0);
    zazel.params[zazel.START_PARAM].setValue (-1.0f);
    zazel.params[zazel.END_PARAM].setValue (1.0f);
    auto t = ts::makeTrigger (1024, 512);
    auto trigger = t + t + t + t + t + t + t + t + t + t + t + t + t + t + t + t;
    auto pauseLen = 100;
    auto pauseTrig = ts::makeTrigger (pauseLen, 44) + ts::makeTrigger (pauseLen, 44);
    zazel.params[zazel.DURATION_PARAM].setValue (1.0f / 1000);
    auto counter = 0;
    zazel.mode = Zazel::Mode::ONESHOT_LOW;
    //run until decay
    while (zazel.mode != Zazel::Mode::ONESHOT_DECAY)
    {
        zazel.inputs[zazel.START_CONT_INPUT].setVoltage (trigger[counter]);
        zazel.step();
        counter++;
    }
    auto attCounter = 0;
    //run until middle of decay
    for (auto i = 0; i < int (sr / 1000) / 2; ++i)

    {
        zazel.inputs[zazel.START_CONT_INPUT].setVoltage (trigger[counter]);
        zazel.step();
        counter++;
        attCounter++;
    }
    //test during pause value remains steady
    auto lastOut = zazel.outputs[zazel.MAIN_OUTPUT].getVoltage();
    for (auto i = 0; i <= pauseLen; ++i)
    {
        zazel.inputs[zazel.START_CONT_INPUT].setVoltage (trigger[counter]);
        zazel.inputs[zazel.STOP_CONT_INPUT].setVoltage (pauseTrig[i]);
        zazel.step();
        counter++;
        auto out = zazel.outputs[zazel.MAIN_OUTPUT].getVoltage();
        assertClose (out, lastOut, 0.000001f);
    }
}

static void testChangePhase()
{
    Zazel zazel;
    zazel.framesSincePhaseChange = 100;
    zazel.changePhase (Zazel::Mode::ONESHOT_HIGH);
    assert (zazel.mode == Zazel::Mode::ONESHOT_HIGH);
    assertEQ (zazel.framesSincePhaseChange, 0);
}

static void testSetSampleRate()
{
    Zazel zazel;
    zazel.setSampleRate (22050);
    assertEQ (zazel.sampleRate, 22050);
}

static void testSyncClock()
{
    //no clock connected;
    Zazel zazel;
    zazel.setSampleRate (5432);
    for (auto i = 0; i < 100000; ++i)
        zazel.step();
    assertEQ (zazel.lastClockDuration, zazel.sampleRate);

    //with external clock
    auto clock = ts::makeClockTrigger (1000, 3);
    zazel.inputs[zazel.CLOCK_INPUT].setChannels (1);
    for (auto x : clock)
    {
        zazel.inputs[zazel.CLOCK_INPUT].setVoltage (x);
        zazel.step();
    }
    assertEQ (zazel.lastClockDuration, 1000);
}

static void testExtreme()
{
    Zazel zazel;
    std::vector<std::pair<float, float>> paramLimits;
    zazel.setSampleRate (44100);
    zazel.init();

    paramLimits.resize (zazel.NUM_PARAMS);
    using fp = std::pair<float, float>;

    auto iComp = Zazel::getDescription();
    for (int i = 0; i < iComp->getNumParams(); ++i)
    {
        auto desc = iComp->getParam (i);
        fp t (desc.min, desc.max);
        paramLimits[i] = t;
    }

    ExtremeTester<Zazel>::test (zazel, paramLimits, true, "Zazel ");
}

void testZazel()
{
    printf ("test Zazel\n");
    test01();
    testCycleFromFresh();
    testOneShotAtInit();
    testOneShotFlag();

    testClockSpeed();
    testOneshotRiseTime();
    testOneshotFallTime();
    testPauseOneshotRising();
    testPauseOneshotFalling();

    testChangePhase();
    testSetSampleRate();
    testSyncClock();

    testExtreme();
}
