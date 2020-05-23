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
#include "simd/vector.hpp"
#include "simd/functions.hpp"
#include "simd/sse_mathfun.h"
#include "simd/sse_mathfun_extension.h"

#include "asserts.h"
#include "TestComposite.h"
#include "ExtremeTester.h"

#include <assert.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include "Eva.h"

using Eva = EvaComp<TestComposite>;

static void testExtreme()
{
    Eva eva;
    std::vector<std::pair<float, float>> paramLimits;

    paramLimits.resize (eva.NUM_PARAMS);
    using fp = std::pair<float, float>;

    auto iComp = Eva::getDescription();
    for (int i = 0; i < iComp->getNumParams(); ++i)
    {
        auto desc = iComp->getParam (i);
        fp t (desc.min, desc.max);
        paramLimits[i] = t;
    }

    ExtremeTester<Eva>::test (eva, paramLimits, true, "Eva ");
}

static void testMaxInputChannels()
{
    Eva eva;
    eva.inputs[eva.ONE_INPUT].setChannels (2);
    eva.inputs[eva.TWO_INPUT].setChannels (4);
    eva.inputs[eva.THREE_INPUT].setChannels (6);
    eva.inputs[eva.FOUR_INPUT].setChannels (8);

    auto maxChannels = eva.maxInputChannels();

    assertEQ (maxChannels, 8);
}

static void testMonoSumming (float i1, float i2, float i3, float i4, float att, float attCv)
{
    Eva eva;
    for (auto i = 0; i < 4; ++i)
        eva.inputs[i].setChannels (1);

    eva.inputs[eva.ONE_INPUT].setVoltage (i1);
    eva.inputs[eva.TWO_INPUT].setVoltage (i2);
    eva.inputs[eva.THREE_INPUT].setVoltage (i3);
    eva.inputs[eva.FOUR_INPUT].setVoltage (i4);
    eva.params[eva.ATTENUVERTER_PARAM].setValue (att);
    eva.inputs[eva.ATTENUATION_CV].setVoltage (attCv);

    eva.step();
    auto attenuation = clamp (att + (attCv / 5.0f), -1.0f, 1.0f);
    auto out = eva.outputs[eva.MAIN_OUTPUT].getVoltage();
    auto expected = (i1 + i2 + i3 + i4) * attenuation;
    assertClose (out, expected, 0.00001f);
}

static void testMonoSumming()
{
    testMonoSumming (-27.4f, 17.5f, 0.0f, -0.45, 1.0f, 0.5f);
    testMonoSumming (-27.4f, 17.5f, 0.0f, -0.45, -1.0f, 1.0f);
    testMonoSumming (-27.4f, 17.5f, 0.0f, -0.45, 0.0f, -1.0f);
    testMonoSumming (-0.4f, 0.5f, 0.2345f, -0.45, 0.34f, -12.0f);
    testMonoSumming (0.4f, 0.523f, 0.3245f, 0.45, -0.0123f, 3.0f);
    testMonoSumming (0.3245f, -0.4545f, -0.34324f, -0.45343, -0.3243f, 0.1f);
}
using VF = std::vector<float>;

static void vectToInput (Eva& e, VF& vect, Eva::InputIds input)
{
    for (auto i = 0; i < int (vect.size()); ++i)
        e.inputs[input].setVoltage (vect[i]);
    e.inputs[input].setChannels (vect.size());
}

static void testPolySumming (VF i1, VF i2, VF i3, VF i4, float att, VF attCv)
{
    Eva eva;
    vectToInput (eva, i1, eva.ONE_INPUT);
    vectToInput (eva, i2, eva.TWO_INPUT);
    vectToInput (eva, i3, eva.THREE_INPUT);
    vectToInput (eva, i4, eva.FOUR_INPUT);
    eva.params[eva.ATTENUVERTER_PARAM].setValue (att);

    for (auto i = 0; i < int (attCv.size()); ++i)
        eva.inputs[eva.ATTENUATION_CV].setVoltage (attCv[i], i);
    eva.inputs[eva.ATTENUATION_CV].setChannels (attCv.size());

    eva.step();

    //test output channel count
    auto maxsize = std::max (
        { int (i1.size()), int (i2.size()), int (i3.size()), int (i4.size()) });
    auto channels = eva.outputs[eva.MAIN_OUTPUT].getChannels();
    assertEQ (channels, maxsize);

    //check out poly channels
    for (auto i = 0; i < channels; ++i)
    {
        auto attenuation = clamp (att + (eva.inputs[eva.ATTENUATION_CV].getPolyVoltage (i) / 5.0f), -1.0f, 1.0f);
        auto expected = (eva.inputs[eva.ONE_INPUT].getPolyVoltage (i)
                         + eva.inputs[eva.TWO_INPUT].getPolyVoltage (i)
                         + eva.inputs[eva.THREE_INPUT].getPolyVoltage (i)
                         + eva.inputs[eva.FOUR_INPUT].getPolyVoltage (i))
                        * attenuation;
        auto out = eva.outputs[eva.MAIN_OUTPUT].getPolyVoltage (i);
        assertClose (out, expected, 0.00001f);
    }
}

static void testPolySumming()
{
    testPolySumming (VF{ 0.3243224f, -0.34345455f, -0.67876856 },
                     VF{ 0.3243224f, 0.546546f, -0.456546546, -0.34324f, -0.5656f, 0.2343224f },
                     VF{ 0.5, -0.65 },
                     VF{ 0.324324f, -0.23433f, -0.234343f, -0.456546f, 0.3454354f, -0.98967f, 0.3434324 },
                     0.2f,
                     VF {1.0f, 0.3f, -0.4f});
}

void testEva()
{
    printf ("testEva\n");
    testExtreme();
    testMaxInputChannels();
    testMonoSumming();
    testPolySumming();
}
