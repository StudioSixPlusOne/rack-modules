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

#include "UtilityFilters.h"
#include "Analyzer.h"
#include "FftAnalyzer.h"
#include "AudioMath.h"
#include "testSignal.h"
#include "asserts.h"
#include "simd/functions.hpp"
#include "simd/sse_mathfun.h"
#include "simd/sse_mathfun_extension.h"
#include "simd/vector.hpp"
#include <assert.h>
#include <stdio.h>
#include <algorithm>

using float_4 = ::rack::simd::float_4;
namespace ts = sspo::TestSignal;
using namespace sspo;

static void testSlopeLrLp (const float cutoff,
                           const float sr,
                           const float expectedCorner,
                           const float expectedSlope)
{
    LinkwitzRileyLP2<float> filter;
    filter.setParameters (sr, cutoff);

    constexpr int fftSize = 1024 * 32;
    ts::Signal signal;
    auto driac = ts::makeDriac (fftSize);

    for (auto x : driac)
        signal.push_back (filter.process (x));

    auto response = ts::getResponse (signal);
    auto driacResponse = ts::getResponse (driac);

    auto slope = FftAnalyzer::getSlopeLowpass (response, driacResponse, cutoff, sr);
#if 00
    printf ("LWR low pass sr %f fc %f corner %f Slope %f\n",
            sr,
            cutoff,
            slope.cornerGain,
            slope.slope);
#else
    assertClose (slope.cornerGain, expectedCorner, 0.2f); // max error at low fc
    assertLE (slope.slope, expectedSlope);
#endif
}

static void testSlopeLrLp (const float_4 cutoff,
                           const float_4 sr,
                           const float expectedCorner,
                           const float expectedSlope)
{
    LinkwitzRileyLP2<float_4> filter;
    filter.setParameters (sr, cutoff);

    constexpr int fftSize = 1024 * 32;
    auto driac = ts::makeDriac (fftSize);
    std::vector<ts::Signal> signals;
    signals.resize (4);

    for (auto x : driac)
    {
        auto s = filter.process (x);
        for (auto i = 0; i < 4; i++)
            signals[i].push_back (s[i]);
    }
    auto driacResponse = ts::getResponse (driac);

    //analyse per channel
    for (auto i = 0; i < 4; ++i)
    {
        auto response = ts::getResponse (signals[i]);

        auto slope = FftAnalyzer::getSlopeLowpass (response, driacResponse, cutoff[i], sr[i]);
#if 00
        printf ("LWR low pass sr %f fc %f corner %f Slope %f\n",
                sr,
                cutoff,
                slope.cornerGain,
                slope.slope);
#else
        assertClose (slope.cornerGain, expectedCorner, 0.2f); // max error at low fc
        assertLE (slope.slope, expectedSlope + 0.5f);
#endif
    }
}

static void testSlopeLrHp (const float cutoff,
                           const float sr,
                           const float expectedCorner,
                           const float expectedSlope)
{
    LinkwitzRileyHP2<float> filter;
    filter.setParameters (sr, cutoff);

    constexpr int fftSize = 1024 * 32;
    ts::Signal signal;
    auto driac = ts::makeDriac (fftSize);

    for (auto x : driac)
        signal.push_back (filter.process (x));

    auto response = ts::getResponse (signal);
    auto driacResonse = ts::getResponse (driac);

    auto slope = FftAnalyzer::getSlopeHighpass (response, driacResonse, cutoff, sr);
#if 0
    printf ("LWR High pass sr %f fc %f corner %f Slope %f\n",
            sr,
            cutoff,
            slope.cornerGain,
            slope.slope);
#else
    assertClose (slope.cornerGain, expectedCorner, 0.15f);
    assertClose (slope.slope, expectedSlope, 0.2f);
#endif
}

static void testSlopeLrHp (const float_4 cutoff,
                           const float_4 sr,
                           const float expectedCorner,
                           const float expectedSlope)
{
    LinkwitzRileyHP2<float_4> filter;
    filter.setParameters (sr, cutoff);

    constexpr int fftSize = 1024 * 32;
    std::vector<ts::Signal> signals;
    signals.resize (4); // a signal for each of the 4 floats in simd
    auto driac = ts::makeDriac (fftSize);

    for (auto x : driac)
    {
        auto s = filter.process (x);
        for (auto i = 0; i < 4; ++i)
            signals[i].push_back (s[i]);
    }
    auto driacResonse = ts::getResponse (driac);
    //analyse the 4 channels
    for (auto i = 0; i < 4; ++i)
    {
        auto response = ts::getResponse (signals[i]);
        auto slope = FftAnalyzer::getSlopeHighpass (response, driacResonse, cutoff[i], sr[i]);
#if 0
    printf ("LWR High pass sr %f fc %f corner %f Slope %f\n",
            sr[i],
            cutoff[i],
            slope.cornerGain,
            slope.slope);
#else
        assertClose (slope.cornerGain, expectedCorner, 0.2f);
        assertClose (slope.slope, expectedSlope, 0.5f);
#endif
    }
}

static void testLrLp()
{
    for (auto fc = 40.0f; fc < 5000.0f; fc += 100.0f)
        testSlopeLrLp (fc, 44100, -6, -12.0f);
}

static void testLrHp()
{
    for (auto fc = 40.0f; fc < 5000.0f; fc += 100.0f)
        testSlopeLrHp (fc, 44100, -6, -12.5f);
}

static void testLrHpSmid()
{
    float_4 sr{ 44100, 44100, 44100, 44100 };
    for (auto fc = 40.0f; fc < 5000.0f; fc += 100.0f)
    {
        float_4 fc_4 = { fc, fc + 100.0f, fc + 250.0f, fc + 400.0f };
        testSlopeLrHp (fc_4, sr, -6, -12.5f);
    }
}

static void testLrLpSmid()
{
    float_4 sr{ 44100, 44100, 44100, 44100 };
    for (auto fc = 40.0f; fc < 5000.0f; fc += 100.0f)
    {
        float_4 fc_4 = { fc, fc + 100.0f, fc + 250.0f, fc + 400.0f };
        testSlopeLrLp (fc_4, sr, -6, -12.5f);
    }
}

static void testLWRCrossOver (float fc, float sr)
{
    LinkwitzRileyLP2<float> lp;
    LinkwitzRileyHP2<float> hp;
    lp.setParameters (sr, fc);
    hp.setParameters (sr, fc);

    constexpr int fftSize = 1024 * 32;
    ts::Signal signal;
    signal.resize (0);

    auto driac = ts::makeDriac (fftSize);

    for (auto x : driac)
    {
        signal.push_back (lp.process (x) - hp.process (x));
    }
    auto driacResponse = ts::getResponse (driac);
    auto response = ts::getResponse (signal);

    ts::Signal levels;
    levels.resize (0);

    for (auto i = 0; i < response.size() / 2.0f; ++i)
    {
        levels.push_back (db (response.getAbs (i)) - db (driacResponse.getAbs (i)));
    }

    auto minval = *std::min_element (levels.begin(), levels.end());
    auto maxval = *std::max_element (levels.begin(), levels.end());
#if 0
    printf ("Min %f Max %f\n", minval, maxval);
#else
    assertClose (maxval, 0.0f, 0.27f); //0.2dB variation at low freq
    assertClose (minval, 0.0f, 0.0012f);
#endif
}

static void testLWRCrossOver (float_4 fc, float_4 sr)
{
    LinkwitzRileyLP2<float_4> lp;
    LinkwitzRileyHP2<float_4> hp;
    lp.setParameters (sr, fc);
    hp.setParameters (sr, fc);

    constexpr int fftSize = 1024 * 32;
    std::vector<ts::Signal> signals;
    signals.resize (4);

    auto driac = ts::makeDriac (fftSize);

    for (auto x : driac)
    {
        auto s = lp.process (x) - hp.process (x);
        for (auto i = 0; i < 4; ++i)
            signals[i].push_back (s[i]);
    }
    auto driacResponse = ts::getResponse (driac);

    //test per channel
    for (auto i = 0; i < 4; i++)
    {
        auto response = ts::getResponse (signals[i]);

        ts::Signal levels;
        levels.resize (0);

        for (auto i = 0; i < response.size() / 2.0f; ++i)
        {
            levels.push_back (db (response.getAbs (i)) - db (driacResponse.getAbs (i)));
        }

        auto minval = *std::min_element (levels.begin(), levels.end());
        auto maxval = *std::max_element (levels.begin(), levels.end());
#if 0
    printf ("Min %f Max %f\n", minval, maxval);
#else
        assertClose (maxval, 0.0f, 0.2f); //0.2dB variation at low freq
        assertClose (minval, 0.0f, 0.0012f);
#endif
    }
}

static void testLWRCrossOver()
{
    for (auto fc = 40.0f; fc < 18000.0f; fc += 100.0f)
        testLWRCrossOver (fc, 44100.0f);
}

static void testLWRCrossOverSmid()
{
    float_4 sr{ 44100, 44100, 44100, 44100 };
    for (auto fc = 40.0f; fc < 18000.0f; fc += 100.0f)
    {
        float_4 fc_4 = { fc, fc + 100.0f, fc + 250.0f, fc + 400.0f };
        testLWRCrossOver (fc_4, sr);
    }
}

void testUtilityFilter()
{
    printf ("Utility Filter\n");
    testLrLp();
    testLrHp();
    testLWRCrossOver();
    testLrHpSmid();
    testLrLpSmid();
    testLWRCrossOverSmid();
}