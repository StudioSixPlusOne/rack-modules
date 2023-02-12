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

#include <assert.h>
#include <stdio.h>
#include "digital.hpp"
#include "TestComposite.h"
#include "ExtremeTester.h"
#include "Analyzer.h"
#include "testSignal.h"

#include "Bascom.h"

using MA = BascomComp<TestComposite>;
namespace ts = sspo::TestSignal;

static void testExtreme (float sr)
{
    MA ma;
    std::vector<std::pair<float, float>> paramLimits;
    ma.setSampleRate (sr);
    ma.init();

    paramLimits.resize (ma.NUM_PARAMS);
    using fp = std::pair<float, float>;

    auto iComp = MA::getDescription();
    for (int i = 0; i < iComp->getNumParams(); ++i)
    {
        auto desc = iComp->getParam (i);
        fp t (desc.min, desc.max);
        paramLimits[i] = t;
    }

    ExtremeTester<MA>::test (ma, paramLimits, true, "Bascom");
}

static void testExtreme()
{
    testExtreme (7000.0f);
    //   testExtreme (5000.0f);
    //   testExtreme (22050.0f);
    //   testExtreme (44100.0f);
    //   testExtreme (48000.0f);
    //   testExtreme (96000.0f);
    //   testExtreme (500000.0f);
}

static void testSelfOscillate (const float voct, const float sr)
{
    auto freq = std::pow (2, voct) * dsp::FREQ_C4;
    freq = freq <= sr / 2.0f ? freq : sr / 2.0f;
    MA ma;
    ma.setSampleRate (sr);
    ma.init();
    ma.outputs[MA::MAIN_OUTPUT].setChannels (1);
    ma.inputs[MA::MAIN_INPUT].setChannels (1);
    ma.inputs[MA::VOCT_INPUT].setChannels (1);
    ma.inputs[MA::VOCT_INPUT].setVoltage (voct, 0);
    ma.params[MA::RESONANCE_PARAM].setValue (10.0f);
    ma.params[MA::DRIVE_PARAM].setValue (5.0f);
    ma.params[MA::COEFF_A_PARAM].setValue (0.0f);
    ma.params[MA::COEFF_B_PARAM].setValue (0.0f);
    ma.params[MA::COEFF_C_PARAM].setValue (0.0f);
    ma.params[MA::COEFF_D_PARAM].setValue (0.0f);
    ma.params[MA::COEFF_E_PARAM].setValue (1.0f);
    ma.params[MA::FREQUENCY_PARAM].setValue (0.5f);
    ma.params[MA::VCA_PARAM].setValue (1.0f);

    assertEQ (ma.sampleRate, sr);

    ts::Signal signal;
    //warmup as oscillations are started by a -120dB signal
    for (auto i = 0; i < 10000000; ++i)
    {
        ma.step();
        signal.push_back (ma.outputs[ma.MAIN_OUTPUT].getVoltage());
    }

    signal.resize (0);

    auto size = 16 * 1024;
    freq = Analyzer::makeEvenPeriod (freq, sr, size);

    for (auto i = 0; i < size; ++i)
    {
        ma.inputs[MA::VOCT_INPUT].setVoltage (voct, 0);
        ma.step();
        signal.push_back (ma.outputs[ma.MAIN_OUTPUT].getVoltage (0));
    }

    //printf ("%f %f %f %d\n",voct, freq, sr, int (signal.size()));

    auto response = ts::getResponse (signal);
    auto maxBin = Analyzer::getMax (response);
    assertEQ (maxBin, FFT::freqToBin (freq, sr, size));
}

static void testSelfOscillate()
{
    testSelfOscillate (0.0f, 44100);
    testSelfOscillate (-1.1f, 44100);
    testSelfOscillate (1.05f, 5000);
    testSelfOscillate (4.0f, 44100);
}

void testBascom()
{
    printf ("testBascom\n");

    //extreme tests take too long
    //     testExtreme();
    //   printf("test Extreme complete");
    testSelfOscillate();
}