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
#include "AudioMath.h"

using Eva = EvaComp<TestComposite>;
using namespace sspo::AudioMath;

constexpr float maxLimit = 11.7f;
constexpr float satStart = 11.2f;

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

static bool checkOutput (float out, float expected)
{
    if ((expected > -satStart) && (expected < satStart))
    {
        return sspo::AudioMath::areSame (out, expected, 0.00001f);
    }
    else if ((expected < maxLimit) || (expected > maxLimit))
    {
        return sspo::AudioMath::areSame (std::abs (out), maxLimit, FLT_EPSILON * 12);
    }
    else
    {
        return (std::abs (out) <= maxLimit) && (std::abs (out) >= satStart - 0.1);
    }
}

static void testMonoSumming (float i1, float i2, float i3, float i4, float att, float attCv)
{
    Eva eva;
    eva.params[eva.GAIN_SHAPE_PARAM].setValue (0);
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
    assertEQ (checkOutput (out, expected), true);
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
    eva.params[eva.GAIN_SHAPE_PARAM].setValue (0);

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
        assertEQ (checkOutput (out, expected), true);
    }
}

static void testPolySumming()
{
    testPolySumming (VF{ 0.3243224f, -0.34345455f, -0.67876856 },
                     VF{ 0.3243224f, 0.546546f, -0.456546546, -0.34324f, -0.5656f, 0.2343224f },
                     VF{ 0.5, -0.65 },
                     VF{ 0.324324f, -0.23433f, -0.234343f, -0.456546f, 0.3454354f, -0.98967f, 0.3434324 },
                     0.2f,
                     VF{ 1.0f, 0.3f, -0.4f });
}

static void testAttenuationFromShape()
{
    Eva eva;
    eva.params[eva.GAIN_SHAPE_PARAM].setValue (0.63);
    float_4 attenuation;
    attenuation[0] = -1.0f;
    attenuation[1] = 1.0f;
    attenuation[2] = -0.37;
    attenuation[3] = +0.49;

    auto shape = eva.params[eva.GAIN_SHAPE_PARAM].getValue();

    auto result = eva.attenuationFromShape (attenuation, shape);

    assertClose (result[0], -1.0f, 0.001f);
    assertClose (result[1], 1.0f, 0.001f);
    assertClose (result[2], -0.198f, 0.001f);
    assertClose (result[3], +0.313f, 0.001f);
}

static void testAttenuationFromShapeIsNormalised()
{
    auto shape = 1.34f;
    Eva eva;
    float_4 attenuation;
    for (attenuation[0] = -1.0f; attenuation[0] < 1.0; attenuation[0] += 0.01)
    {
        auto result = eva.attenuationFromShape (attenuation, shape);
        assertGE (result[0], -1.0f);
        assertLE (result[0], 1.0f);
    }
}

static void testAttenuationFromShapeWhenAttenuationZero()
{
    Eva eva;

    auto inc = 0.1f;
    auto epsilon = 0.001f;
    std::vector<float_4> permutations;

    auto testsRun = 0;
    auto zeroChecked = 0;
    permutations.push_back ({ 0, 1, 1, 1 });
    permutations.push_back ({ 1, 0, 1, 1 });
    permutations.push_back ({ 1, 1, 0, 1 });
    permutations.push_back ({ 1, 1, 1, 0 });
    permutations.push_back ({ 0, 0, 1, 1 });
    permutations.push_back ({ 1, 0, 0, 1 });
    permutations.push_back ({ 1, 1, 0, 0 });
    permutations.push_back ({ 0, 1, 1, 0 });
    permutations.push_back ({ 0, 0, 0, 1 });
    permutations.push_back ({ 1, 0, 0, 0 });
    permutations.push_back ({ 0, 1, 0, 0 });
    permutations.push_back ({ 0, 0, 1, 0 });
    permutations.push_back ({ 0, 0, 0, 0 });
    for (auto shape = -3.0f; shape <= 3.0f; shape += inc)
    {
        //printf ("shape %f\n", shape);
        for (auto a = -1.0f; a < 1.0f; a += inc)
        {
            //printf ("a %f\n", a);
            for (auto b = -1.0f; b < 1.0f; b += inc)
            {
                //printf ("b %f\n", b);
                for (auto c = -1.0f; c < 1.0f; c += inc)
                {
                    //printf ("c %f\n", c);
                    for (auto d = -1.0f; d < 1.0f; d += inc)
                    {
                        //printf ("d %f\n", d);
                        for (auto perm : permutations)
                        {
                            float_4 x{ a, b, c, d };
                            auto y = x * perm;
                            //printf ("y %f %f %f %f\n", y[0], y[1], y[2], y[3]);
                            auto result = eva.attenuationFromShape (y, shape);
                            for (auto i = 0; i < 4; ++i)
                            {
                                testsRun++;
                                if (areSame (perm[i], 0.0f))
                                {
                                    zeroChecked++;
                                    assertClose (result[i], 0.0f, epsilon);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    printf ("run %d zeros %d\n", testsRun, zeroChecked);
}

static void testSimdClamp2()
{
    float_4 attMin{ -1.0f, -1.0f, -1.0f, -1.0f };
    float_4 attMax{ 1.0f, 1.0f, 1.0f, 1.0f };
    float_4 x{ -2.0f, -1.0f, 1.0f, 2.0f };

    auto res = simd::clamp (x, attMin, attMax);

    assertEQ (res[0], -1.0f);
    assertEQ (res[1], -1.0f);
    assertEQ (res[2], 1.0f);
    assertEQ (res[3], 1.0f);
}

//test to compare std::pow and simd::pow
// as expected with fast approx functions a level of tolerance is found
// simd(0, order), does not return 0, instead 240613957091486838146644248300244434944.000000
static void testSimdPow()
{
    auto inc = 0.1f;
    auto epsilon = 0.001f;

    float_4 x{};
    float_4 order_4{};
    for (auto order = 0.0f; order <= 3.0f; order += inc)
    {
        order_4 = { order, order, order, order };

        for (x[0] = 0.0f; x[0] <= 20.0f; x[0] += inc)
        {
            auto resultStd = std::pow (x[0], order);
            auto resultSimd = simd::pow (x, order_4);
            //assertClose (resultStd, resultSimd[0], epsilon);
            if (! ((resultSimd[0] >= resultStd - epsilon)
                   && (resultSimd[0] <= resultStd + epsilon)))
            {
                printf ("x : %f, order %f, std %f simd %f\n",
                        x[0],
                        order,
                        resultStd,
                        resultSimd[0]);
            }
        }
    }
}

void testEva()
{
    printf ("testEva\n");
    testAttenuationFromShape();
    testAttenuationFromShapeIsNormalised();
    testAttenuationFromShapeWhenAttenuationZero();
    testSimdClamp2();
    testSimdPow();
    testExtreme();
    testMaxInputChannels();
    testMonoSumming();
    testPolySumming();
}
