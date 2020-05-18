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

// An empty test, can be used as a template

#include <assert.h>
#include <stdio.h>
#include "filter.hpp"
#include "digital.hpp"
#include "TestComposite.h"
#include "ExtremeTester.h"
#include "Analyzer.h"
#include "testSignal.h"

#include "CombFilter.h"

using CF = CombFilterComp<TestComposite>;
namespace ts = sspo::TestSignal;

static void testExtreme()
{
    CF cf;
    std::vector<std::pair<float, float>> paramLimits;
    cf.setSampleRate (44100);
    cf.init();

    paramLimits.resize (cf.NUM_PARAMS);
    using fp = std::pair<float, float>;

    auto iComp = CF::getDescription();
    for (int i = 0; i < iComp->getNumParams(); ++i)
    {
        auto desc = iComp->getParam (i);
        fp t (desc.min, desc.max);
        paramLimits[i] = t;
    }

    ExtremeTester<CF>::test (cf, paramLimits, true, "Comb Filter Massarti ");
}

// frequency in V/oct 0 = C4
void testPositiveCombPeaks (float voct, float sr)
{
    auto size = 65536 * 8.0f;
    auto freq = 261.63f * std::pow (2.0f, voct);
    freq = Analyzer::makeEvenPeriod (freq, sr, size);
    CF cf;
    cf.setSampleRate (sr);
    cf.init();
    auto noise = ts::noiseFromFile();

    cf.params[cf.COMB_PARAM].setValue (1.0f);
    cf.params[cf.FREQUENCY_PARAM].setValue (voct);
    cf.params[cf.FEEDBACK_PARAM].setValue (1.0f);
    FFTDataReal fftIn (size);
    for (auto i = 0; i < size; ++i)
    {
        cf.inputs[cf.MAIN_INPUT].setVoltage (noise[i] * 5.0f);
        cf.step();
        fftIn.set (i, (cf.outputs[cf.MAIN_OUTPUT].getVoltage() / 5.0f) * Analyzer::hamming (i, size));
    }
    FFTDataCpx fftOut (size);
    FFT::forward (&fftOut, fftIn);
    auto peaks = Analyzer::getPeaks (fftOut, sr, -48.0f);

    printf ("Comb peaks: freq %f  sr %f peak count %d\n", freq, sr, static_cast<int> (peaks.size()));

    for (auto i = 0; i < peaks.size(); ++i)
    {
        if (peaks[i].freq >= freq * 0.75f)
        {
            //expect to see many values clustered about harmonic of freq
            //printf ("%f\n", peaks[i].freq);
            auto error = std::fmod (peaks[i].freq, freq) > freq / 2.0f
                             ? freq - std::fmod (peaks[i].freq, freq)
                             : std::fmod (peaks[i].freq, freq);
            assertClose (error, 0.0f, peaks[i].freq * 0.10f);
        }
    }
}

void testCombFilter()
{
    printf ("CombFilter \n");
    testPositiveCombPeaks (0.0f, 44100.0f);
    testPositiveCombPeaks (0.0f, 5000.0f);
    testPositiveCombPeaks (-4.0f, 44100.0f);
    testPositiveCombPeaks (3.0f, 44100.0f);
    testPositiveCombPeaks (0.0f, 96000.0f);

    testExtreme();
}
