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
    assertClose (slope.cornerGain, expectedCorner, 0.2f);
    assertClose (slope.slope, expectedSlope, 0.8f);
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
        assertClose (slope.slope, expectedSlope, 1.0f);
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
    assertClose (minval, 0.0f, 0.006f);
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
        assertClose (maxval, 0.0f, 0.3f); //0.2dB variation at low freq
        assertClose (minval, 0.0f, 0.005f);
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

//Linkwitz-Riley 4 pole

static void testSlopeLrLp4 (const float cutoff,
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
#if 0
    printf ("LWR4 low pass sr %f fc %f corner %f Slope %f\n",
            sr,
            cutoff,
            slope.cornerGain,
            slope.slope);
#else
    assertClose (slope.cornerGain, expectedCorner, 0.2f); // max error at low fc
    assertLE (slope.slope, expectedSlope + 0.2f);
#endif
}

static void testSlopeLrLp4 (const float_4 cutoff,
                            const float_4 sr,
                            const float expectedCorner,
                            const float expectedSlope)
{
    LinkwitzRileyLP4<float_4> filter;
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
        printf ("LWR4 low pass sr %f fc %f corner %f Slope %f\n",
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

static void testSlopeLrHp4 (const float cutoff,
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
    printf ("LWR4 High pass sr %f fc %f corner %f Slope %f\n",
            sr,
            cutoff,
            slope.cornerGain,
            slope.slope);
#else
    assertClose (slope.cornerGain, expectedCorner, 0.15f);
    assertClose (slope.slope, expectedSlope, 0.5f);
#endif
}

static void testSlopeLrHp4 (const float_4 cutoff,
                            const float_4 sr,
                            const float expectedCorner,
                            const float expectedSlope)
{
    LinkwitzRileyHP4<float_4> filter;
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
    printf ("LWR4 High pass sr %f fc %f corner %f Slope %f\n",
            sr[i],
            cutoff[i],
            slope.cornerGain,
            slope.slope);
#else
        assertClose (slope.cornerGain, expectedCorner, 0.2f);
        assertClose (slope.slope, expectedSlope, 0.6f);
#endif
    }
}

static void testLrLp4()
{
    for (auto fc = 80.0f; fc < 5000.0f; fc += 100.0f)
        testSlopeLrLp4 (fc, 44100, -6, -24.0f);
}

static void testLrHp4()
{
    for (auto fc = 140.0f; fc < 5000.0f; fc += 100.0f)
        testSlopeLrHp4 (fc, 44100, -6, -24.5f);
}

static void testLrHpSmid4()
{
    float_4 sr{ 44100, 44100, 44100, 44100 };
    for (auto fc = 140.0f; fc < 5000.0f; fc += 100.0f)
    {
        float_4 fc_4 = { fc, fc + 100.0f, fc + 250.0f, fc + 400.0f };
        testSlopeLrHp4 (fc_4, sr, -6, -24.5f);
    }
}

static void testLrLpSmid4()
{
    float_4 sr{ 44100, 44100, 44100, 44100 };
    for (auto fc = 80.0f; fc < 4000.0f; fc += 100.0f)
    {
        float_4 fc_4 = { fc, fc + 100.0f, fc + 250.0f, fc + 400.0f };
        testSlopeLrLp4 (fc_4, sr, -6, -24.0f);
    }
}

static void testLWRCrossOver4 (float fc, float sr)
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
    assertClose (maxval, 0.0f, 0.37f); //0.35dB variation at low freq
    assertClose (minval, 0.0f, 0.006f);
#endif
}

static void testLWRCrossOver4 (float_4 fc, float_4 sr)
{
    LinkwitzRileyLP4<float_4> lp;
    LinkwitzRileyHP4<float_4> hp;
    lp.setParameters (sr, fc);
    hp.setParameters (sr, fc);

    constexpr int fftSize = 1024 * 32;
    std::vector<ts::Signal> signals;
    signals.resize (4);

    auto driac = ts::makeDriac (fftSize);

    for (auto x : driac)
    {
        auto s = lp.process (x) + hp.process (x);
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
        assertClose (maxval, 0.0f, 0.35f); //0.35dB variation at low freq
        assertClose (minval, 0.0f, 0.006f);
#endif
    }
}

static void testLWRCrossOver4()
{
    for (auto fc = 40.0f; fc < 18000.0f; fc += 100.0f)
        testLWRCrossOver4 (fc, 44100.0f);
}

static void testLWRCrossOverSmid4()
{
    float_4 sr{ 44100, 44100, 44100, 44100 };
    for (auto fc = 40.0f; fc < 18000.0f; fc += 100.0f)
    {
        float_4 fc_4 = { fc, fc + 100.0f, fc + 250.0f, fc + 400.0f };
        testLWRCrossOver4 (fc_4, sr);
    }
}

// ALLPASS *************************

static void testAllPass (float fc, float sr)
{
    BiQuad<float> allpass;
    allpass.setAllPass1stOrder (sr, fc);
    ts::Signal sig;
    int fftSize = 1024 * 16;

    auto driac = ts::makeDriac (fftSize);

    for (auto x : driac)
        sig.push_back (allpass.process (x));

    auto driacMagnitude = FftAnalyzer::getMagnitude (driac);
    auto sigMagnitude = FftAnalyzer::getMagnitude (sig);
    auto magMatch = ts::areSame (driacMagnitude, sigMagnitude, 0.2f);

#if 0
    for (auto i = 0; i < fftSize / 2; ++i)
        printf ("%f %f\n", sigMagnitude[i], driacMagnitude[i]);
#else

    assert (magMatch);
#endif

    auto driacPhase = FftAnalyzer::getPhase (driac);
    auto sigPhase = FftAnalyzer::getPhase (sig);
#if 0
    for (auto x : sigPhase)
        printf ("phase %f\n", x);
#else
    assertClose (sigPhase[0], 0.0f, 0.0001f);
    assertClose (sigPhase[sigPhase.size() - 1], -k_pi, 0.01f);

    auto freqBin = FFT::freqToBin (fc, sr, fftSize);
    assertClose (sigPhase[freqBin], -k_pi / 2.0f, 0.01f);
#endif
}

static void testAllPass()
{
    for (auto fc = 100.0f; fc < 5000.0f; fc += 100.0f)
        testAllPass (fc, 44100.0f);
}

//Mixed Biquad *********************

static void testMixedBiquadSimd()
{
    auto fclp = 300.0;
    auto fchp = 800.0f;
    auto fclp2 = 1200.0f;
    auto fchp2 = 1870.0f;

    auto sr = 96000.0f;

    BiQuad<float> lp;
    BiQuad<float> lp2;
    BiQuad<float> hp;
    BiQuad<float> hp2;

    lp.setLinkwitzRileyLp2 (sr, fclp);
    lp2.setLinkwitzRileyLp2 (sr, fclp2);
    hp.setLinkwitzRileyHp2 (sr, fchp);
    hp2.setLinkwitzRileyHp2 (sr, fchp2);

    MixedBiquadSimd filter;
    filter.mergeCoeffs (lp, lp2, hp, hp2);

    auto fftSize = 1024 * 16;

    auto driac = ts::makeDriac (fftSize);
    std::vector<ts::Signal> signals;
    signals.resize (4);
    for (auto x : driac)
    {
        auto s = filter.process (x);
        for (auto i = 0; i < 4; ++i)
        {
            signals[i].push_back (s[i]);
        }
    }

    auto driacResponse = ts::getResponse (driac);
    auto lpResponse = ts::getResponse (signals[0]);
    auto lp2Response = ts::getResponse (signals[1]);
    auto hpResponse = ts::getResponse (signals[2]);
    auto hp2Response = ts::getResponse (signals[3]);

    auto lpslope = FftAnalyzer::getSlopeLowpass (lpResponse, driacResponse, fclp, sr);
    auto lp2slope = FftAnalyzer::getSlopeLowpass (lp2Response, driacResponse, fclp2, sr);
    auto hpslope = FftAnalyzer::getSlopeHighpass (hpResponse, driacResponse, fchp, sr);
    auto hp2slope = FftAnalyzer::getSlopeHighpass (hp2Response, driacResponse, fchp2, sr);

    assertClose (lpslope.cornerGain, -6.0f, 0.5f);
    assertClose (lp2slope.cornerGain, -6.0f, 0.5f);
    assertClose (hpslope.cornerGain, -6.0f, 0.5f);
    assertClose (hp2slope.cornerGain, -6.0f, 0.5f);

    assertClose (lpslope.slope, -12.0f, 1.0f);
    assertClose (lp2slope.slope, -12.0f, 1.0f);
    assertClose (hpslope.slope, -12.0f, 1.0f);
    assertClose (hp2slope.slope, -12.0f, 1.0f);
}

// Butterworth Tests *****************************
static void testSlopeButterworthLp (const float cutoff,
                                    const float sr,
                                    const float expectedCorner,
                                    const float expectedSlope)
{
    BiQuad<float> filter;
    filter.setButterworthLp2 (sr, cutoff);

    constexpr int fftSize = 1024 * 32;
    ts::Signal signal;
    auto driac = ts::makeDriac (fftSize);

    for (auto x : driac)
        signal.push_back (filter.process (x));

    auto response = ts::getResponse (signal);
    auto driacResponse = ts::getResponse (driac);

    auto slope = FftAnalyzer::getSlopeLowpass (response, driacResponse, cutoff, sr);
#if 0
    printf ("Butterworth low pass sr %f fc %f corner %f Slope %f\n",
            sr,
            cutoff,
            slope.cornerGain,
            slope.slope);
#else
    assertClose (slope.cornerGain, expectedCorner, 0.3f); // max error at low fc
    assertLE (slope.slope, expectedSlope);
#endif
}

static void testSlopeButterworthLp (const float_4 cutoff,
                                    const float_4 sr,
                                    const float expectedCorner,
                                    const float expectedSlope)
{
    BiQuad<float_4> filter;
    filter.setButterworthLp2 (sr, cutoff);

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
        printf ("Butterworth low pass sr %f fc %f corner %f Slope %f\n",
                sr,
                cutoff,
                slope.cornerGain,
                slope.slope);
#else
        assertClose (slope.cornerGain, expectedCorner, 0.3f); // max error at low fc
        assertLE (slope.slope, expectedSlope + 0.5f);
#endif
    }
}

static void testSlopeButterworthHp (const float cutoff,
                                    const float sr,
                                    const float expectedCorner,
                                    const float expectedSlope)
{
    BiQuad<float> filter;
    filter.setButterworthHp2 (sr, cutoff);

    constexpr int fftSize = 1024 * 32;
    ts::Signal signal;
    auto driac = ts::makeDriac (fftSize);

    for (auto x : driac)
        signal.push_back (filter.process (x));

    auto response = ts::getResponse (signal);
    auto driacResonse = ts::getResponse (driac);

    auto slope = FftAnalyzer::getSlopeHighpass (response, driacResonse, cutoff, sr);
#if 0
    printf ("Butterworth High pass sr %f fc %f corner %f Slope %f\n",
            sr,
            cutoff,
            slope.cornerGain,
            slope.slope);
#else
    assertClose (slope.cornerGain, expectedCorner, 0.15f);
    assertClose (slope.slope, expectedSlope, 0.6f);
#endif
}

static void testUpsampleDecimator()
{
    constexpr int OVERSAMPLE = 4;
    Upsampler<OVERSAMPLE, 1, float> up;
    Decimator<OVERSAMPLE, 1, float> decimator;

    constexpr int fftSize = 1024 * 32;
    auto s = ts::makeSine (fftSize, 1000, 44100);
    ts::Signal result;

    float buffer[OVERSAMPLE];

    for (auto x : s)
    {
        up.process (x, buffer);
        result.push_back (decimator.process (buffer));
    }

    assertEQ (result.size(), fftSize);

    auto s_response = ts::getResponse (s);
    auto r_response = ts::getResponse (result);

    auto s_magnitude = FftAnalyzer::getMagnitude (s_response);
    auto r_magnitude = FftAnalyzer::getMagnitude (r_response);

    auto s_max_bin = std::distance (s_magnitude.begin(), std::max_element (s_magnitude.begin(), s_magnitude.end()));
    auto r_max_bin = std::distance (r_magnitude.begin(), std::max_element (r_magnitude.begin(), r_magnitude.end()));

    assertEQ (s_max_bin, r_max_bin);
}

static void testSlopeButterworthHp (const float_4 cutoff,
                                    const float_4 sr,
                                    const float expectedCorner,
                                    const float expectedSlope)
{
    BiQuad<float_4> filter;
    filter.setButterworthHp2 (sr, cutoff);

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
    printf ("Butterworth High pass sr %f fc %f corner %f Slope %f\n",
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

static void testButterworthLp()
{
    for (auto fc = 40.0f; fc < 5000.0f; fc += 100.0f)
        testSlopeButterworthLp (fc, 44100, -3, -11.8f);
}

static void testButterworthHp()
{
    for (auto fc = 140.0f; fc < 5000.0f; fc += 100.0f)
        testSlopeButterworthHp (fc, 44100, -3, -12.0f);
}

static void testButterworthHpSmid()
{
    float_4 sr{ 44100, 44100, 44100, 44100 };
    for (auto fc = 140.0f; fc < 5000.0f; fc += 100.0f)
    {
        float_4 fc_4 = { fc, fc + 100.0f, fc + 250.0f, fc + 400.0f };
        testSlopeButterworthHp (fc_4, sr, -3, -12.0f);
    }
    printf ("testButterworthHpSimd complete \n");
}

static void testDcBlocker()
{
    sspo::DcBlocker<float> dcBlocker;
}

static void testButterworthLpSmid()
{
    float_4 sr{ 44100, 44100, 44100, 44100 };
    for (auto fc = 40.0f; fc < 5000.0f; fc += 100.0f)
    {
        float_4 fc_4 = { fc, fc + 100.0f, fc + 250.0f, fc + 400.0f };
        testSlopeButterworthLp (fc_4, sr, -3, -11.8f);
    }
}

void testUtilityFilter()
{
    printf ("Utility Filter\n");
    testLrLp4();
    testLrHp4();
    testLWRCrossOver4();
    testLrHpSmid4();
    testLrLpSmid4();
    testLWRCrossOverSmid4();
    testLrLp();
    testLrHp();
    testLWRCrossOver();
    testLrHpSmid();
    testLrLpSmid();
    testLWRCrossOverSmid();
    testAllPass();
    testMixedBiquadSimd();
    testButterworthLp();
    testButterworthHp();
    testButterworthLpSmid();
    testButterworthHpSmid();
    testUpsampleDecimator();
}