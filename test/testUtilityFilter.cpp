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
#include <assert.h>
#include <stdio.h>
#include <algorithm>

namespace ts = sspo::TestSignal;
using namespace sspo;

static void testSlopeLrLp (const float cutoff,
                           const float sr,
                           const float expectedCorner,
                           const float expectedSlope)
{
    LinkwitzRileyLP4<float> filter;
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

static void testSlopeLrHp (const float cutoff,
                           const float sr,
                           const float expectedCorner,
                           const float expectedSlope)
{
    LinkwitzRileyHP4<float> filter;
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

static void testLWRCrossOver (float fc, float sr)
{
    LinkwitzRileyLP4<float> lp;
    LinkwitzRileyHP4<float> hp;
    lp.setParameters (sr, fc);
    hp.setParameters (sr, fc);

    constexpr int fftSize = 1024 * 32;
    ts::Signal signal;
    signal.resize (0);

    auto driac = ts::makeDriac (fftSize);

    for (auto x : driac)
    {
        signal.push_back (lp.process (x) + hp.process (x));
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
    assertClose (maxval, 0.0f, 0.2f); //0.2dB variation at low freq
    assertClose (minval, 0.0f, 0.0001f);
#endif
}

static void testLWRCrossOver()
{
    for (auto fc = 40.0f; fc < 18000.0f; fc += 100.0f)
        testLWRCrossOver (fc, 44100.0f);
}

void testUtilityFilter()
{
    printf ("Utility Filter\n");
    testLrLp();
    testLrHp();
    testLWRCrossOver();
}