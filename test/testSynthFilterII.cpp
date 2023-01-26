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

#include "SynthFilterII.h"
#include "Analyzer.h"
#include "FftAnalyzer.h"
#include "AudioMath.h"
#include "testSignal.h"
#include "asserts.h"
#include "simd/functions.hpp"
#include "simd/sse_mathfun.h"
#include "simd/sse_mathfun_extension.h"
#include <assert.h>
#include <stdio.h>
#include <algorithm>

using float_4 = ::rack::simd::float_4;
namespace ts = sspo::TestSignal;
using namespace sspo;

//static void testSlopeOnePoleLpFilter (const float cutoff,
//                                      const float sr,
//                                      const float expectedCorner,
//                                      const float expectedSlope)
//{
//    SynthFilterII::OnePoleLpFilter<float> filter;
//    filter.setSampleRate (sr);
//    filter.setFrequency (cutoff);
//
//    constexpr int fftSize = 1024 * 32;
//    ts::Signal signal;
//    auto driac = ts::makeDriac (fftSize);
//
//    for (auto x : driac)
//        signal.push_back (filter.process (x));
//
//    auto response = ts::getResponse (signal);
//    auto driacResonse = ts::getResponse (driac);
//
//    auto slope = FftAnalyzer::getSlopeLowpass (response, driacResonse, cutoff, sr);
//
//#if 0
//    printf ("SynthFilterII sr %f fc %f corner %f Slope %f flat %f\n",
//            sr,
//            cutoff,
//            slope.cornerGain,
//            slope.slope,
//            slope.flatGain);
//#else
//    assertClose (slope.cornerGain, expectedCorner, 0.15f);
//    assertClose (slope.slope, expectedSlope, 1.0f);
//    assertClose (slope.flatGain, 0.0f, 0.3f);
//#endif
//}
//
//static void testSlopeOnePoleLpFilter (const float_4 cutoff,
//                                      const float sr,
//                                      const float expectedCorner,
//                                      const float expectedSlope)
//{
//    SynthFilterII::OnePoleLpFilter<float_4> filter;
//    filter.setSampleRate (sr);
//    filter.setFrequency (cutoff);
//
//    constexpr int fftSize = 1024 * 32;
//    std::vector<ts::Signal> signals;
//    signals.resize (4); // a signal for each of the 4 floats in simd
//    auto driac = ts::makeDriac (fftSize);
//
//    for (auto x : driac)
//    {
//        auto s = filter.process (x);
//        for (auto i = 0; i < 4; ++i)
//            signals[i].push_back (s[i]);
//    }
//    auto driacResonse = ts::getResponse (driac);
//    for (auto i = 0; i < 4; ++i)
//    {
//        auto response = ts::getResponse (signals[i]);
//        auto slope = FftAnalyzer::getSlopeLowpass (response, driacResonse, cutoff[i], sr);
//#if 1
//    printf ("Butterworth High pass sr %f fc %f corner %f Slope %f flat %f\n",
//            sr,
//            cutoff[i],
//            slope.cornerGain,
//            slope.slope,
//            slope.flatGain);
//
//#else
//        assertClose (slope.cornerGain, expectedCorner, 0.2f);
//        assertClose (slope.slope, expectedSlope, 0.5f);
//#endif
//    }
//}

//static void testSlopeOnePoleLpFilterFloat()
//{
//    for (auto fc = 40.0f; fc < 2500.0f; fc += 100.0f)
//        testSlopeOnePoleLpFilter (fc, 44100, -3, -6.0f);
//}
//
//static void testSlopeOnePoleLpFilterSimd()
//{
//    for (auto fc = 40.0f; fc < 2100.0f; fc += 100.0f)
//    {
//        float_4 fc_4 = { fc, fc + 100.0f, fc + 250.0f, fc + 400.0f };
//        testSlopeOnePoleLpFilter (fc_4, 44100, -3, -6.0f);
//    }
//}

void testSynthFilterII()
{
    printf ("SynthFilterII\n");
//    testSlopeOnePoleLpFilterFloat();
//    testSlopeOnePoleLpFilterSimd();
}