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
#include <array>
#include <vector>
#include "dsp/digital.hpp"
#include "../src/composites/framework/TestComposite.h"
#include "ExtremeTester.h"
#include "Analyzer.h"
#include "testSignal.h"
#include "FftAnalyzer.h"
#include <cmath>

#include "../src/composites/Hula.h"

using float_4 = ::rack::simd::float_4;

using HC = HulaComp<TestComposite>;
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
    HC hc;
    std::vector<std::pair<float, float>> paramLimits;
    hc.setSampleRate (sr);
    hc.init();

    paramLimits.resize (hc.NUM_PARAMS);
    using fp = std::pair<float, float>;

    auto iComp = HC::getDescription();
    for (int i = 0; i < iComp->getNumParams(); ++i)
    {
        auto desc = iComp->getParam (i);
        fp t (desc.min, desc.max);
        paramLimits[i] = t;
    }

    ExtremeTester<HC>::test (hc, paramLimits, true, "Hula");
}

static void testExtreme()
{
    for (auto sr : sampleRates)
    {
        testExtreme (sr);
    }
}

static void testVoct (float_4 vocts, float sr)
{
    auto freqs = rack::simd::pow (2, vocts) * dsp::FREQ_C4;
    // allow +- 3 cent random tuning
    auto minFreqs = rack::simd::pow (2, vocts - ((1.0f / 12.0f) * 0.03)) * dsp::FREQ_C4;
    auto maxFreqs = rack::simd::pow (2, vocts + ((1.0f / 12.0f) * 0.03)) * dsp::FREQ_C4;

    HC hc;
    hc.setSampleRate (sr);
    hc.init();
    hc.outputs[HC::MAIN_OUTPUT].setChannels (4);
    hc.inputs[HC::VOCT_INPUT].setChannels (4);
    hc.inputs[HC::FEEDBACK_CV_INPUT].setChannels (1);
    hc.inputs[HC::FM_INPUT].setChannels (1);
    hc.inputs[HC::DEPTH_CV_INPUT].setChannels (1);

    hc.params[HC::RATIO_PARAM].setValue (0.0f);
    hc.params[HC::SEMITONE_PARAM].setValue (0.0f);
    hc.params[HC::OCTAVE_PARAM].setValue (0.0f);
    hc.params[HC::DEPTH_PARAM].setValue (0.0f);
    hc.params[HC::FEEDBACK_PARAM].setValue (0.0f);
    hc.params[HC::DEFAULT_TUNING_PARAM].setValue (261.62f);
    hc.params[HC::UNISON_PARAM].setValue (1.0f);
    hc.params[HC::DC_OFFSET_PARAM].setValue (0.0f);
    hc.params[HC::SCALE_PARAM].setValue (1.0f);

    hc.inputs[HC::FEEDBACK_CV_INPUT]
        .setVoltage (0.0f, 0);
    hc.inputs[HC::FM_INPUT].setVoltage (0.0f, 0);
    hc.inputs[HC::DEPTH_CV_INPUT].setVoltage (0.0f, 0);
    hc.inputs[HC::VOCT_INPUT].setVoltageSimd (vocts, 0);

    assertEQ (hc.sampleRate, sr);

    auto size = 1024 * 16;
    std::array<ts::Signal, 4> signals;

    //run process to fill signal buffers
    for (auto i = 0; i < size; ++i)
    {
        hc.step();
        for (int j = 0; j < 4; ++j)
        {
            signals[j].push_back (hc.outputs[hc.MAIN_OUTPUT].getVoltage (j));
        }
    }

    // FFT and test frequency
    for (auto i = 0; i < 4; ++i)
    {
        auto response = ts::getResponse (signals[i]);
        auto maxResponseBin = Analyzer::getMax (response);
        auto minRequiredBin = FFT::freqToBin (minFreqs[i], sr, size);
        auto maxRequiredBin = FFT::freqToBin (maxFreqs[i], sr, size) + 2;
#if 0
        printf ("SR: %f   Voct: %f   Freq: %f  Min Expected bin: %d  Mxn Expected bin: %d   Actual bin: %d  Pass %d\n",
                sr,
                vocts[i],
                freqs[i],
                minRequiredBin,
                maxRequiredBin,
                maxResponseBin,
                ((maxResponseBin >= minRequiredBin) && (maxResponseBin <= maxRequiredBin)));
#else
        assert (((maxResponseBin >= minRequiredBin) && (maxResponseBin <= maxRequiredBin)));
#endif
    }
}

static void testVoct()
{
    for (auto sr : sampleRates)
    {
        for (float voct = -4.0f; voct < 3.5f; voct += (1.0f / 12.0f))
        {
            auto vocts = float_4 (voct, voct + 0.5f, voct - 0.5f, voct / 2.0f);
            testVoct (vocts, sr);
        }
    }
}

static void testOctave (float octave, float sr)
{
    auto freq = rack::simd::pow (2, octave) * dsp::FREQ_C4;
    // allow +- 3 cent random tuning
    auto minFreq = rack::simd::pow (2, octave - ((1.0f / 12.0f) * 0.03)) * dsp::FREQ_C4;
    auto maxFreq = rack::simd::pow (2, octave + ((1.0f / 12.0f) * 0.03)) * dsp::FREQ_C4;

    HC hc;
    hc.setSampleRate (sr);
    hc.init();
    hc.outputs[HC::MAIN_OUTPUT].setChannels (1);
    hc.inputs[HC::VOCT_INPUT].setChannels (1);
    hc.inputs[HC::FEEDBACK_CV_INPUT].setChannels (1);
    hc.inputs[HC::FM_INPUT].setChannels (1);
    hc.inputs[HC::DEPTH_CV_INPUT].setChannels (1);

    hc.params[HC::RATIO_PARAM].setValue (0.0f);
    hc.params[HC::SEMITONE_PARAM].setValue (0.0f);
    hc.params[HC::DEPTH_PARAM].setValue (0.0f);
    hc.params[HC::FEEDBACK_PARAM].setValue (0.0f);
    hc.params[HC::DEFAULT_TUNING_PARAM].setValue (261.63f);
    hc.params[HC::UNISON_PARAM].setValue (1.0f);
    hc.params[HC::DC_OFFSET_PARAM].setValue (0.0f);
    hc.params[HC::SCALE_PARAM].setValue (1.0f);

    hc.params[HC::OCTAVE_PARAM].setValue (octave);

    hc.inputs[HC::FEEDBACK_CV_INPUT].setVoltage (0.0f, 0);
    hc.inputs[HC::FM_INPUT].setVoltage (0.0f, 0);
    hc.inputs[HC::DEPTH_CV_INPUT].setVoltage (0.0f, 0);
    hc.inputs[HC::VOCT_INPUT].setVoltage (0.0f, 0);

    assertEQ (hc.sampleRate, sr);

    auto size = 1024 * 16;
    ts::Signal signal;

    //run process to fill signal buffers
    for (auto i = 0; i < size; ++i)
    {
        hc.step();
        signal.push_back (hc.outputs[hc.MAIN_OUTPUT].getVoltage (0));
    }

    // FFT and test frequency

    auto response = ts::getResponse (signal);
    auto maxResponseBin = Analyzer::getMax (response);
    auto minRequiredBin = FFT::freqToBin (minFreq, sr, size);
    auto maxRequiredBin = FFT::freqToBin (maxFreq, sr, size) + 1;
#if 0
        printf ("SR: %f   Octave: %f   Freq: %f  Min Expected bin: %d  Mxn Expected bin: %d   Actual bin: %d  Pass %d\n",
                sr,
                octave,
                freq,
                minRequiredBin,
                maxRequiredBin,
                maxResponseBin,
                ((maxResponseBin >= minRequiredBin) && (maxResponseBin <= maxRequiredBin)));
#else
    assert (((maxResponseBin >= minRequiredBin) && (maxResponseBin <= maxRequiredBin)));
#endif
}

static void testOctave()
{
    for (auto sr : sampleRates)
    {
        for (int octave = -4; octave < 4; octave++)
        {
            testOctave (octave, sr);
        }
    }
}

static void testRatio (float ratio, float sr)
{
    auto freq = dsp::FREQ_C4 * ratio;
    // allow +- 3 cent random tuning
    auto minFreq = rack::simd::pow (2, 0.0f - ((1.0f / 12.0f) * 0.03)) * dsp::FREQ_C4 * ratio;
    auto maxFreq = rack::simd::pow (2, 0.0f + ((1.0f / 12.0f) * 0.03)) * dsp::FREQ_C4 * ratio;

    HC hc;
    hc.setSampleRate (sr);
    hc.init();
    hc.outputs[HC::MAIN_OUTPUT].setChannels (1);
    hc.inputs[HC::VOCT_INPUT].setChannels (1);
    hc.inputs[HC::FEEDBACK_CV_INPUT].setChannels (1);
    hc.inputs[HC::FM_INPUT].setChannels (1);
    hc.inputs[HC::DEPTH_CV_INPUT].setChannels (1);

    hc.params[HC::RATIO_PARAM].setValue (std::log2f (ratio));
    hc.params[HC::SEMITONE_PARAM].setValue (0.0f);
    hc.params[HC::DEPTH_PARAM].setValue (0.0f);
    hc.params[HC::FEEDBACK_PARAM].setValue (0.0f);
    hc.params[HC::DEFAULT_TUNING_PARAM].setValue (261.63f);
    hc.params[HC::UNISON_PARAM].setValue (1.0f);
    hc.params[HC::DC_OFFSET_PARAM].setValue (0.0f);
    hc.params[HC::SCALE_PARAM].setValue (1.0f);

    hc.params[HC::OCTAVE_PARAM].setValue (-0.0f);

    hc.inputs[HC::FEEDBACK_CV_INPUT].setVoltage (0.0f, 0);
    hc.inputs[HC::FM_INPUT].setVoltage (0.0f, 0);
    hc.inputs[HC::DEPTH_CV_INPUT].setVoltage (0.0f, 0);
    hc.inputs[HC::VOCT_INPUT].setVoltage (0.0f, 0);

    assertEQ (hc.sampleRate, sr);

    auto size = 1024 * 16;
    ts::Signal signal;

    //run process to fill signal buffers
    for (auto i = 0; i < size; ++i)
    {
        hc.step();
        signal.push_back (hc.outputs[hc.MAIN_OUTPUT].getVoltage (0));
    }

    // FFT and test frequency

    auto response = ts::getResponse (signal);
    auto maxResponseBin = Analyzer::getMax (response);
    auto minRequiredBin = FFT::freqToBin (minFreq, sr, size);
    auto maxRequiredBin = FFT::freqToBin (maxFreq, sr, size) + 3;
#if 0
        printf ("SR: %f   Ratio: %f   Freq: %f  Min Expected bin: %d  Mxn Expected bin: %d   Actual bin: %d  Pass %d\n",
                sr,
                ratio,
                freq,
                minRequiredBin,
                maxRequiredBin,
                maxResponseBin,
                ((maxResponseBin >= minRequiredBin) && (maxResponseBin <= maxRequiredBin)));
#else
    assert (((maxResponseBin >= minRequiredBin) && (maxResponseBin <= maxRequiredBin)));
#endif
}

static void testRatio()
{
    for (auto sr : sampleRates)
    {
        for (float ratio = 0.5; ratio < 21.0f; ratio += 0.25f)
        {
            testRatio (ratio, sr);
        }
    }
}

static void testDcOffset (float_4 vocts, float sr)
{
    auto maxDc = -24.0f;

    auto freqs = rack::simd::pow (2, vocts) * dsp::FREQ_C4;
    // allow +- 3 cent random tuning
    auto minFreqs = rack::simd::pow (2, vocts - ((1.0f / 12.0f) * 0.03)) * dsp::FREQ_C4;
    auto maxFreqs = rack::simd::pow (2, vocts + ((1.0f / 12.0f) * 0.03)) * dsp::FREQ_C4;

    HC hc;
    hc.setSampleRate (sr);
    hc.init();
    hc.outputs[HC::MAIN_OUTPUT].setChannels (4);
    hc.inputs[HC::VOCT_INPUT].setChannels (4);
    hc.inputs[HC::FEEDBACK_CV_INPUT].setChannels (1);
    hc.inputs[HC::FM_INPUT].setChannels (1);
    hc.inputs[HC::DEPTH_CV_INPUT].setChannels (1);

    hc.params[HC::RATIO_PARAM].setValue (1.0f);
    hc.params[HC::SEMITONE_PARAM].setValue (0.0f);
    hc.params[HC::OCTAVE_PARAM].setValue (0.0f);
    hc.params[HC::DEPTH_PARAM].setValue (0.0f);
    hc.params[HC::FEEDBACK_PARAM].setValue (0.0f);
    hc.params[HC::DEFAULT_TUNING_PARAM].setValue (261.63f);
    hc.params[HC::UNISON_PARAM].setValue (1.0f);
    hc.params[HC::DC_OFFSET_PARAM].setValue (0.0f);
    hc.params[HC::SCALE_PARAM].setValue (1.0f);

    hc.inputs[HC::FEEDBACK_CV_INPUT].setVoltage (0.0f, 0);
    hc.inputs[HC::FM_INPUT].setVoltage (0.0f, 0);
    hc.inputs[HC::DEPTH_CV_INPUT].setVoltage (0.0f, 0);
    hc.inputs[HC::VOCT_INPUT].setVoltageSimd (vocts, 0);

    assertEQ (hc.sampleRate, sr);

    auto size = 1024 * 16;
    std::array<ts::Signal, 4> signals;

    //run process to fill signal buffers
    for (auto i = 0; i < size; ++i)
    {
        hc.step();
        for (int j = 0; j < 4; ++j)
        {
            signals[j].push_back (hc.outputs[hc.MAIN_OUTPUT].getVoltage (j));
        }
    }

    // FFT and test DC offset
    for (auto i = 0; i < 4; ++i)
    {
        auto response = ts::getResponse (signals[i]);
        auto magnitudes = sspo::FftAnalyzer::getMagnitude (signals[i]);
        auto dc = magnitudes[0];
#if 0
        printf ("SR: %f   Voct: %f   Freq: %f  Dc offset: %fdB Pass %d\n",
                sr,
                vocts[i],
                freqs[i],
                dc,
                (dc < maxDc));
#else
        assert (dc < maxDc);
#endif
    }
}

static void testDcOffset()
{
    for (float voct = -4.0f; voct < 3.5f; voct += (1.0f / 12.0f))
    {
        auto vocts = float_4 (voct, voct + 0.5f, voct - 0.5f, voct / 2.0f);
        testDcOffset (vocts, 44100.0f);
    }
}

void testHula()
{
    printf ("testHula\n");
    testExtreme();
//    testVoct();
//    testOctave();
//    testRatio();
//    testDcOffset();
}