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


#include "math.hpp"
#include "WaveShaper.h"

#include <asserts.h>
#include <cmath>
#include <stdio.h>

using namespace sspo::AudioMath;

static void testCreate()
{
    auto table = WaveShaper::makeTable ([] (const float x) -> float
                                        { return cosf (k_pi * x); });

    auto index = 0;
    for (float i = WaveShaper::minValue; index < 4096; i += WaveShaper::interval)
    {
        //        printf ("%d %f %f  %f\n", index, i, (table[index], cosf (k_pi * i)));
        assertClose (table[index], cosf (k_pi * i), 0.0001f);
        index++;
    }
}

static void testConsume()
{
    WaveShaper::Nld nld;

    //tanh
    for (float i = WaveShaper::minValue; i < WaveShaper::maxValue - WaveShaper::interval; i += WaveShaper::interval)
    {
        //        printf ("tanh x %f %f  %f\n", i, nld.process (i, 1), tanhf (i));
        assert (areSame (nld.process (i, 1), tanhf (i), 0.001f));
        assert (areSame (nld.tanhShaper (i), tanhf (i), 0.001f));
    }
    //tanh2x
    for (float i = WaveShaper::minValue; i < WaveShaper::maxValue - WaveShaper::interval; i += WaveShaper::interval)
    {
     //    printf (" tanh 2x %f %f  %f\n", i, nld.process (i, 2), tanhf (i * 2.0f));
        assert (areSame (nld.process (i, 2), tanhf (2.0f * i) / tanhf(2.0f), 0.001f));
        assert (areSame (nld.tanh2Shaper (i), tanhf (i * 2.0f)/ tanhf(2.0f), 0.001f));
    }

    //cos x
    for (float i = WaveShaper::minValue; i < WaveShaper::maxValue - WaveShaper::interval; i += WaveShaper::interval)
    {
//         printf (" cos x %f %f  %f\n", i, nld.process (i, 3), cosf (i * k_pi));
        assert (areSame (nld.process (i, 3), 1.5f * i * (1.0f - (i * i * 0.33333333f)), 0.001f));
        assert (areSame (nld.cosShaper (i), 1.5f * i * (1.0f - (i * i * 0.33333333f)), 0.001f));
        assertGE (nld.cosShaper(i) , -1.0001f);
        assertLE (nld.cosShaper(i) , 1.0001f);
    }
}

void testWaveShaper()
{
    printf ("testWave shaper\n");
    testCreate();
    testConsume();
    //    testConsumeSimd();
}