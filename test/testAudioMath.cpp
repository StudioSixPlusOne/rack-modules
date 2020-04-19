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
#include "../src/AudioMath.h"

using namespace sspo;

static void testAreSame()
{
    float a = 0.00001f;
    float b = 0.00001f;
    float c = 0.000001f;
    assert (AudioMath::areSame (a, b) && "Test aresame float");
    assert (!AudioMath::areSame (a, c) && "Test not areaSame float");

    double d = 7.998;
    double e = 7.998;
    double f = 7.999;

    assert (AudioMath::areSame (d, e) && "Test aresame double");
    assert (!AudioMath::areSame (d, f) && "Test not areaSame double");

    std::vector<float> g = {0.1 - 0.1, 0.3, 0.7, -0.6443};
    std::vector<float> h = {0.1 - 0.1, 0.3, 0.7, -0.6443};
    std::vector<float> i = {0.1 - 0.1, 0.3, 0.7, -0.6444};
    std::vector<float> j = {0.1 - 0.1, 0.3, 0.7};
    assert(AudioMath::areSame (g, h) && "Test aresame float vector");
    assert(!AudioMath::areSame (h, i) && "Test not areaSame float vector");
    assert(!AudioMath::areSame (j, i) && "Test not areaSame float vector length");
 
    std::vector<double> k = {0.1 - 0.1, 0.3, 0.7, -0.6443};
    std::vector<double> l = {0.1 - 0.1, 0.3, 0.7, -0.6443};
    std::vector<double> m = {0.1 - 0.1, 0.3, 0.7, -0.6444};
    std::vector<double> n = {0.1 - 0.1, 0.3, 0.7};
    assert (AudioMath::areSame (k, l) && "Test aresame double vector");
    assert (!AudioMath::areSame (l, m) && "Test not areaSame double vector");
    assert (!AudioMath::areSame (m, n) && "Test not areaSame double vector length");
}

static void testFastTanh()
{
    for (auto i = -5.0f; i < 5.0f; i += 0.0001)
    {
        assert (AudioMath::areSame (tanhf (i), AudioMath::fastTanh (i), 0.032f) && "fast tanh");
    }
}

void testAudioMath()
{
    printf ("AudioMath\n");
    testAreSame();
    testFastTanh();
}
