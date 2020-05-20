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

#include "SynthFilter.h"
#include "asserts.h"
#include "Analyzer.h"
#include "AudioMath.h"
#include "testSignal.h"
#include <assert.h>
#include <stdio.h>
#include <sstream>

namespace ts = sspo::TestSignal;

using namespace sspo;

static std::string makeFilename (std::string filename,
                                 SynthFilter::Type type,
                                 const float cutoff,
                                 const float sr,
                                 const int length)
{
    std::stringstream ss;
    ss << filename << "_" << int (type) << "_" << cutoff << "_" << sr << "_" << length;
    return ss.str();
}

static void makeOnePoleImpulse (std::string filename,
                                SynthFilter::Type type,
                                const float cutoff,
                                const float sr,
                                const int length)
{
    OnePoleFilter filter;
    filter.setType (type);
    filter.setUseNonLinearProcessing (false);
    filter.setParameters (cutoff, 0.0f, 0.0f, sr);

    ts::Signal impulse;
    auto driac = ts::makeDriac (length);

    for (auto x : driac)
        impulse.push_back (filter.process (x));

    if (! ts::toFile (impulse, makeFilename (filename, type, cutoff, sr, length)))
        assert (false);
}

static void testOnePoleImpulse (std::string filename,
                                SynthFilter::Type type,
                                const float cutoff,
                                const float sr,
                                const int length)
{
    OnePoleFilter filter;
    filter.setType (type);
    filter.setUseNonLinearProcessing (false);
    filter.setParameters (cutoff, 0.0f, 0.0f, sr);

    ts::Signal impulse;
    auto driac = ts::makeDriac (length);

    for (auto x : driac)
        impulse.push_back (filter.process (x));

    ts::Signal test = ts::fromFile (makeFilename (filename, type, cutoff, sr, length));
    assert (ts::areSame (impulse, test));
}

static void testMoogLadderSlope (SynthFilter::Type type,
                                 const float cutoff,
                                 const float sr,
                                 const float expected,
                                 const float tol = 1.5f)
{
    MoogLadderFilter filter;
    filter.setType (type);
    filter.setUseNonLinearProcessing (true);
    filter.setParameters (cutoff, 0.0f, 1.1f, 0.0f, sr);

    constexpr int fftSize = 1024 * 32;

    ts::Signal signal;
    // repeat get response and take average
    auto driac = ts::makeDriac (fftSize);

    for (auto x : driac)
        signal.push_back (filter.process (x));

    auto response = ts::getResponse (signal);

    auto slope = (type == SynthFilter::Type::LPF2 || type == SynthFilter::Type::LPF4)
                     ? Analyzer::getSlopeLowpass (response, cutoff, sr)
                     : Analyzer::getSlopeHighpass (response, cutoff, sr);

    //printf ("lop pass %d %f %f %f\n", int (type), cutoff, sr, slope);

    assertClose (slope, expected, tol);
}

static void testMoogLPHP()
{
    testMoogLadderSlope (SynthFilter::Type::LPF2, 10.0, 44100.0f, -9.0);
    testMoogLadderSlope (SynthFilter::Type::LPF2, 100.0, 44100.0f, -9.0);
    testMoogLadderSlope (SynthFilter::Type::LPF2, 1000.0, 44100.0f, -9.0);
    testMoogLadderSlope (SynthFilter::Type::LPF2, 2000.0, 44100.0f, -9.0);

    testMoogLadderSlope (SynthFilter::Type::LPF2, 10.0, 22050.0f, -9.0);
    testMoogLadderSlope (SynthFilter::Type::LPF2, 100.0, 22050.0f, -9.0);
    testMoogLadderSlope (SynthFilter::Type::LPF2, 1000.0, 22050.0f, -9.0);

    testMoogLadderSlope (SynthFilter::Type::LPF2, 10.0, 48000.0f, -9.0);
    testMoogLadderSlope (SynthFilter::Type::LPF2, 100.0, 48000.0f, -9.0);
    testMoogLadderSlope (SynthFilter::Type::LPF2, 1000.0, 48000.0f, -9.0);
    testMoogLadderSlope (SynthFilter::Type::LPF2, 2000.0, 48000.0f, -9.0);

    testMoogLadderSlope (SynthFilter::Type::LPF4, 10.0, 44100.0f, -19.0);
    testMoogLadderSlope (SynthFilter::Type::LPF4, 100.0, 44100, -19.0);
    testMoogLadderSlope (SynthFilter::Type::LPF4, 1000.0, 44100, -19.0);
    testMoogLadderSlope (SynthFilter::Type::LPF4, 2000.0, 44100, -19.0);

    testMoogLadderSlope (SynthFilter::Type::LPF4, 10.0, 22050, -19.0);
    testMoogLadderSlope (SynthFilter::Type::LPF4, 100.0, 22050, -19.0);
    testMoogLadderSlope (SynthFilter::Type::LPF4, 1000.0, 22050, -19.0);

    testMoogLadderSlope (SynthFilter::Type::LPF4, 10.0, 48000, -19.0);
    testMoogLadderSlope (SynthFilter::Type::LPF4, 100.0, 48000, -19.0);
    testMoogLadderSlope (SynthFilter::Type::LPF4, 1000.0, 48000, -19.0);
    testMoogLadderSlope (SynthFilter::Type::LPF4, 2000.0, 48000, -19.0);

    testMoogLadderSlope (SynthFilter::Type::HPF2, 100.0, 44100, -9.0);
    testMoogLadderSlope (SynthFilter::Type::HPF2, 1000.0, 44100, -9.0);
    testMoogLadderSlope (SynthFilter::Type::HPF2, 2000.0, 44100, -9.0);

    testMoogLadderSlope (SynthFilter::Type::HPF2, 100.0, 22050, -9.0);
    testMoogLadderSlope (SynthFilter::Type::HPF2, 1000.0, 22050, -9.0);

    testMoogLadderSlope (SynthFilter::Type::HPF2, 100.0, 48000, -9.0);
    testMoogLadderSlope (SynthFilter::Type::HPF2, 1000.0, 48000, -9.0);
    testMoogLadderSlope (SynthFilter::Type::HPF2, 2000.0, 48000, -9.0);

    testMoogLadderSlope (SynthFilter::Type::HPF4, 100.0, 44100, -17.0);
    testMoogLadderSlope (SynthFilter::Type::HPF4, 1000.0, 44100, -17.0);
    testMoogLadderSlope (SynthFilter::Type::HPF4, 2000.0, 44100, -17.0);

    testMoogLadderSlope (SynthFilter::Type::HPF4, 100.0, 22050, -17.0);
    testMoogLadderSlope (SynthFilter::Type::HPF4, 1000.0, 22050, -17.0);

    testMoogLadderSlope (SynthFilter::Type::HPF4, 100.0, 48000, -17.0);
    testMoogLadderSlope (SynthFilter::Type::HPF4, 1000.0, 48000, -17.0);
    testMoogLadderSlope (SynthFilter::Type::HPF4, 2000.0, 48000, -17.0);
}

static void testMoogBpPeak()
{
    printf ("peak\n");

    auto sr = 44100.0f;

    MoogLadderFilter filter;
    filter.setType (SynthFilter::Type::BPF2);
    filter.setUseNonLinearProcessing (false);

    constexpr int fftSize = 1024 * 8;
    const float binWidth = sr / fftSize;

    ts::Signal signal;
    auto driac = ts::makeDriac (fftSize);

    for (auto freq = 20; freq < 20000; freq += 10)
    {
        auto fc = Analyzer::makeEvenPeriod (freq, sr, fftSize);
        filter.reset();
        filter.setParameters (fc, 7.0f, 1.1f, 0.0f, sr);
        signal.resize (0);
        for (auto x : driac)
            signal.push_back (filter.process (x));

        auto response = ts::getResponse (signal);
        auto peakBin = Analyzer::getMax (response);
        auto freqBin = FFT::freqToBin (fc, sr, fftSize);
        auto tol = fc * 0.005 * binWidth; // 0.5% tolerance
        //printf ("%f ,%d %d %f\n", fc, freqBin, peakBin, tol);
        assertClose (peakBin, freqBin, int (tol));
    }
}

void testSynthFilter()
{
    printf ("testSynthFilter\n");
    testMoogLPHP();
    testMoogBpPeak();

//impulse response tests to check for changes
//set below to #if 1 to run impulse tests, #if 0 to generate impulses
#if 1
    testOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::LPF1,
                        20.0f,
                        5000,
                        3000);
    testOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::LPF1,
                        20.0f,
                        22050,
                        3000);
    testOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::LPF1,
                        20.0f,
                        44100,
                        3000);
    testOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::LPF1,
                        200.0f,
                        96000,
                        3000);
    testOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::LPF1,
                        2000.0f,
                        96000,
                        3000);
    testOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::HPF1,
                        20.0f,
                        5000,
                        3000);
    testOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::LPF1,
                        20.0f,
                        22050,
                        3000);
    testOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::HPF1,
                        20.0f,
                        44100,
                        3000);
    testOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::HPF1,
                        200.0f,
                        96000,
                        3000);
    testOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::HPF1,
                        2000.0f,
                        44100,
                        3000);

#else
    //generate Impulse responses
    //only use if the filter response has been intentionally changed
    makeOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::LPF1,
                        20.0f,
                        5000,
                        3000);
    makeOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::LPF1,
                        20.0f,
                        22050,
                        3000);
    makeOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::LPF1,
                        20.0f,
                        44100,
                        3000);
    makeOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::LPF1,
                        200.0f,
                        96000,
                        3000);
    makeOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::LPF1,
                        2000.0f,
                        96000,
                        3000);
    makeOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::HPF1,
                        20.0f,
                        5000,
                        3000);
    makeOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::LPF1,
                        20.0f,
                        22050,
                        3000);
    makeOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::HPF1,
                        20.0f,
                        44100,
                        3000);
    makeOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::HPF1,
                        200.0f,
                        96000,
                        3000);
    makeOnePoleImpulse ("./test/signal/onePoleImpulse",
                        SynthFilter::Type::HPF1,
                        2000.0f,
                        44100,
                        3000);
#endif
}