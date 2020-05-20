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
#include <map>

#include "testSignal.h"

namespace ts = sspo::TestSignal;

static void testMakeZeros()
{
    auto x = ts::makeZeros (101);
    assertEQ (x.size(), 101);
    for (auto s : x)
    {
        assertEQ (s, 0.0f);
    }
}

static void testPlusEqual()
{
    auto x = ts::makeZeros (101);
    auto y = ts::makeZeros (49);
    x += y;
    assertEQ (x.size(), 150);
    for (auto s : x)
    {
        assertEQ (s, 0.0f);
    }
}

static void testPlus()
{
    auto x = ts::makeZeros (101);
    auto y = ts::makeZeros (49);
    auto z = x + y;
    assertEQ (z.size(), 150);
    for (auto s : z)
    {
        assertEQ (s, 0.0f);
    }
}

static void testMakeFixed()
{
    auto x = ts::makeFixed (101, 0.25f);
    assertEQ (x.size(), 101);
    for (auto s : x)
    {
        assertClose (s, 0.25f, FLT_EPSILON);
    }
}

static void testMultiply()
{
    auto x = ts::makeFixed (101, 0.25f);
    auto y = x * 4.0f;
    assertEQ (y.size(), 101);
    for (auto s : y)
    {
        assertClose (s, 1.00f, FLT_EPSILON);
    }
}

static void testMakeNoise()
{
    //map used to check spread of results
    std::map<int, bool> bands;

    auto x = ts::makeNoise (1000000);
    assertEQ (x.size(), 1000000);
    for (auto s : x)
    {
        assertLE (s, 1.0f);
        assertGE (s, -1.0f);
        bands[static_cast<int> (s * 100)] = true;
    }
    assertGT (bands.size(), 198);
}

static void testNoiseFromFile()
{
    //map used to check spread of results
    std::map<int, bool> bands;

    auto x = ts::noiseFromFile();
    assertEQ (x.size(), 500000);
    for (auto s : x)
    {
        assertLE (s, 1.0f);
        assertGE (s, -1.0f);
        bands[static_cast<int> (s * 100)] = true;
    }
    assertGT (bands.size(), 198);
}

static void testMakeDriac()
{
    auto x = ts::makeDriac (1000);
    assertEQ (x.size(), 1000);
    assertClose (x[0], 1.0f, FLT_EPSILON);
    for (auto i = 1; i < int (x.size()); ++i)
    {
        assertClose (x[i], 0.0f, FLT_EPSILON);
    }
}

static void testMakeTrigger()
{
    auto x = ts::makeTrigger (300, 20);
    assertEQ (x.size(), 300);
    for (auto i = 0; i < 20; i++)
    {
        assertClose (x[i], 1.0f, FLT_EPSILON);
    }
    for (auto i = 20; i < 300; i++)
    {
        assertClose (x[i], 0.0f, FLT_EPSILON);
    }
}

static void testMakeClockTrigger()
{
    auto x = ts::makeClockTrigger (100, 1000);
    assertEQ (x.size(), 100000);
    auto count = 0;
    for (auto s : x)
        count = s == 1.0 ? count + 1 : count;

    assertEQ (count, 44000);
}

void testTestSignal()
{
    printf ("testTestSignal\n");
    testMakeZeros();
    testPlusEqual();
    testPlus();
    testMakeFixed();
    testMultiply();
    testMakeNoise();
    testNoiseFromFile();
    testMakeDriac();
    testMakeTrigger();
    testMakeClockTrigger();
}
