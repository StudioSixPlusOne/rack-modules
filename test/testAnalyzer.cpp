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

#include <asserts.h>

#include "Analyzer.h"
#include "FFT.h"
#include "FFTData.h"
#include "testSignal.h"

namespace ts = sspo::TestSignal;

static void testFft()
{
    auto size = 1000000;
    auto freq = 20.0f;
    auto sr = 44100.0f;
    auto sig = ts::makeSine (size, freq, sr);
    FFTDataReal indata (size);
    FFTDataCpx outdata (size);
    for (auto i = 0; i < size; ++i)
        indata.set (i, sig[i]);

    FFT::forward (&outdata, indata);

    //locate max bin
    auto max = 0.0;
    auto maxbin = -1;
    for (auto i = 0; i < size / 2.0; ++i)
    {
        if (outdata.getAbs (i) > max)
        {
            maxbin = i;
            max = outdata.getAbs (i);
        }
    }
    //printf ("freq  %d %f \n", maxbin, FFT::bin2Freq (maxbin, sr, size));
    assertClose (static_cast<float> (FFT::bin2Freq (maxbin, sr, size)), freq, 0.1f);
}

static void testAssertSingleFreq()
{
    auto size = 1000000;
    auto freq = 20.0f;
    auto sr = 44100.0f;
    auto sig = ts::makeSine (size, freq, sr);
    FFTDataReal indata (size);
    FFTDataCpx outdata (size);
    for (auto i = 0; i < size; ++i)
        indata.set (i, sig[i]);

    FFT::forward (&outdata, indata);

    Analyzer::assertSingleFreq (outdata, freq, sr);
}

static void testTwoPeaks()
{
    auto size = 1000000;
    auto freq1 = 20.0f;
    auto freq2 = 12765.0f;
    auto sr = 44100.0f;
    auto sig1 = ts::makeSine (size, freq1, sr);
    auto sig2 = ts::makeSine (size, freq2, sr);
    auto sig = ts::mix (sig1, sig2);

    FFTDataReal indata (size);
    FFTDataCpx outdata (size);
    for (auto i = 0; i < size; ++i)
        indata.set (i, sig[i]);

    FFT::forward (&outdata, indata);

    auto peaks = Analyzer::getPeaks (outdata, sr, -60.0f);

    assertEQ (peaks.size(), 2);
    assertClose (peaks[0].freq, freq1, 0.1f);
    assertClose (peaks[1].freq, freq2, 0.2f);
}

void testAnalyzer()
{
    printf ("Analyzer\n");
    testFft();
    testAssertSingleFreq();
    testTwoPeaks();
}