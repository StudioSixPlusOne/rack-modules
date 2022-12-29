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
#include "math.hpp"
#include "LookupTable.h"

#include <asserts.h>
#include <cmath>
#include <stdio.h>

using namespace sspo::AudioMath;

static void testCreate()
{
    sspo::AudioMath::LookupTable::Table<float> sineTable = LookupTable::makeTable<float> (0.0f, 5.0f, 0.001f, [] (const float x) -> float { return std::sin (x); });
    assert (sineTable.minX == 0.0f);
    assert (sineTable.maxX == 5.0f);
    assert (sineTable.interval == 0.001f);

    auto index = 0;
    for (float i = 0.000f; i < 5.0f; i += 0.001f)
    {
        assertEQ (sineTable.table[index], std::sin (i));
        index++;
    }
}

static void testConsume()
{
    sspo::AudioMath::LookupTable::Table<float> sineTable = LookupTable::makeTable<float> (-k_2pi - 0.1f, k_2pi + 0.1f, 0.001f, [] (const float x) -> float { return std::sin (x); });
    for (float i = 3.0f; i < 5.0f; i += 0.0001f)
    {
        assertClose (LookupTable::process<float> (sineTable, i), std::sin (i), 0.001f);
        assertClose (lookup.sin (i), std::sin (i), 0.01f);
    }

    sspo::AudioMath::LookupTable::Table<float> pow2Table = LookupTable::makeTable<float> (-10.1f, 10.1f, 0.001f, [] (const float x) -> float { return std::pow (2.0f, x); });
    for (float i = -10.0f; i < 10.0f; i += 0.001f)
    {
        assertClose (LookupTable::process<float> (pow2Table, i), std::pow (2.0f, i), 1.0f);
        assertClose (lookup.pow2 (i), std::pow (2.0f, i), 1.0f);
    }

    sspo::AudioMath::LookupTable::Table<float> pow10Table = LookupTable::makeTable<float> (-10.1f, 10.1f, 0.001f, [] (const float x) -> float { return std::pow (10.0f, x); });
    for (float i = -10.0f; i < 10.0f; i += 0.001f)
    {
        assertClose (LookupTable::process<float> (pow10Table, i), std::pow (10.0f, i), std::pow (10.0f, i) * 0.01f);
        assertClose (lookup.pow10 (i), std::pow (10.0f, i), std::pow (10.0f, i) * 0.01f);
    }

    sspo::AudioMath::LookupTable::Table<float> log10Table = LookupTable::makeTable<float> (0.00001f, 10.1f, 0.00001f, [] (const float x) -> float { return std::log10 (x); });
    for (float i = 0.001f; i < 10.0f; i += 0.001f)
    {
        assertClose (LookupTable::process<float> (log10Table, i), std::log10 (i), 0.05f);
        assertClose (lookup.log10 (i), std::log10 (i), 0.05f);
    }

    //unison scalar
    sspo::AudioMath::LookupTable::Table<float> usTable = LookupTable::makeTable<float> (0.00001f, 10.1f, 0.00001f, [] (const float x) -> float { return sspo::AudioMath::LookupTable::unisonSpreadScalar (x); });
    for (float i = 0.001f; i < 1.0f; i += 0.001f)
    {
        assertClose (LookupTable::process<float> (usTable, i), LookupTable::unisonSpreadScalar (i), 0.05f);
        assertClose (lookup.unisonSpread (i), LookupTable::unisonSpreadScalar (i), 0.05f);
    }

    //std::cout << sspo::AudioMath::LookupTable::makeHeader(sineTable, "SineTable") << "\n\n";
}

static void testConsumeSimd()
{
    ///  pow2 and hulaSin have simd lookups for simultanious reading
    float_4 a{ -3.0, -0, 1.4, 2.0 };
    float_4 r = lookup.hulaSin4 (a);

    for (auto i = 0; i < 4; ++i)
        assertClose (lookup.hulaSin (a[i]), r[i], 0.001f);

    printf ("testConsumeSimd Test Lookup ok");
}

void testLookupTable()
{
    printf ("testLookupTable\n");
    testCreate();
    testConsume();
    testConsumeSimd();
}
