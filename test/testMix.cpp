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
#include "digital.hpp"
#include "../src/composites/framework/TestComposite.h"
#include "ExtremeTester.h"
#include "Analyzer.h"
#include "testSignal.h"

#include "../src/composites/Mix.h"

using MA = MixComp<TestComposite>;
namespace ts = sspo::TestSignal;

static std::array<float, 14> sampleRates = { 11025.0f,
                                             12000.0f,
                                             22050.0f,
                                             24000.0f,
                                             44100.0f,
                                             48000.0f,
                                             88200.0f,
                                             96000.0f,
                                             176400.0f,
                                             192000.0f,
                                             352800.0f,
                                             384000.0f,
                                             705600.0f,
                                             768000.0f };

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

    ExtremeTester<MA>::test (ma, paramLimits, true, "Mix");
}

static void testExtreme()
{
    for (auto sr : sampleRates)
        testExtreme (sr);
}

static void testAllinputsMono()
{
    MA mixer;
    mixer.setSampleRate (sampleRates[3]);
    mixer.init();

    mixer.params[MA::NLD_PARAM].setValue (0.0);

    float gains[] = { 0.2f, 0.4f, 1.8f, 1.0f, 1.6f };

    for (auto i = 0; i < 5; ++i)
    {
        mixer.params[i].setValue (gains[i]);
    }

    float inputs[] = { -3.5f, 7.4f, 3.2f, 0.0f, -2.76f };
    for (auto i = 0; i < 5; ++i)
    {
        mixer.inputs[i].setVoltage (inputs[i]);
        mixer.inputs[i].setChannels (1);
    }

    auto mainGain = 0.6f;
    mixer.params[MA::MAIN_PARAM].setValue (mainGain);

    mixer.step();

    auto expected = 0.0f;

    for (auto i = 0; i < 5; ++i)
    {
        expected += inputs[i] * gains[i];
    }
    expected *= mainGain;

    assertEQ (mixer.outputs[MA::MAIN_OUTPUT].getVoltage(), expected);
}

static void testAllInputsPoly()
{
    MA mixer;
    mixer.setSampleRate (sampleRates[3]);
    mixer.init();

    mixer.params[MA::NLD_PARAM].setValue (0.0);

    float gains[] = { 0.2f, 0.4f, 1.8f, 1.0f, 1.6f };

    for (auto i = 0; i < 5; ++i)
    {
        mixer.params[i].setValue (gains[i]);
    }

    float_4 inputs[] = { float_4 (-3.5f, 7.4f, 3.2f, 0.0f),
                         float_4 (0.5f, -7.4f, -4.1f, 2.1f),
                         float_4 (-2.76f, 4.8564f, -2.3456f, 7.2345f),
                         float_4 (3.6456f, -6.234236f, 3.4567f, 3.56436f),
                         float_4 (4.45645f, -4.456745f, -6.23423f, 0.234f) };
    for (auto i = 0; i < 5; ++i)
    {
        mixer.inputs[i].setVoltageSimd (inputs[i], 0);
        mixer.inputs[i].setChannels (4);
    }

    auto mainGain = 0.6f;
    mixer.params[MA::MAIN_PARAM].setValue (mainGain);

    mixer.step();

    float_4 expected = float_4 (0);

    for (auto i = 0; i < 5; ++i)
    {
        expected += inputs[i] * gains[i];
    }
    expected *= mainGain;

    float_4 result = mixer.outputs[MA::MAIN_OUTPUT].getVoltageSimd<float_4> (0);

    auto same = result - expected;
    auto delta = 0.000001f;

    assert (same[0] < delta && same[1] < delta && same[2] < delta && same[3] < delta);
}

void testMix()
{
    printf ("test Mix\n");
    testExtreme();
    testAllinputsMono();
    testAllInputsPoly();
}