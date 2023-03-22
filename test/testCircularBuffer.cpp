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

#include "CircularBuffer.h"
#include "asserts.h"
#include <assert.h>
#include <stdio.h>

using namespace sspo;

static void testDefaultSize()
{
    sspo::CircularBuffer<float> c;
    assert (c.size() == 4096);
}

static void testConstructorSize()
{
    sspo::CircularBuffer<float> c (512);
    assert (c.size() == 512);
}

static void testSizePow2()
{
    sspo::CircularBuffer<double> c (6);
    assert (c.size() == 8);

    sspo::CircularBuffer<int> d (500);
    assert (d.size() == 512);

    sspo::CircularBuffer<long long> e (4000);
    assert (e.size() == 4096);
}

static void testResetEmpty()
{
    sspo::CircularBuffer<float> c;
    for (int i = 0; i < c.size(); ++i)
        c.writeBuffer (std::rand() * 0.5 + 1); // all non zero
    for (int i = 0; i < c.size(); ++i)
        assert (c.readBuffer (i) != 0.0f);
    c.reset (512);
    for (int i = 0; i < c.size(); ++i)
        assert (c.readBuffer (i) == 0.0f);
}

static void testReadZeroDelay()
{
    sspo::CircularBuffer<float> c;
    auto x = 0.7f;
    c.writeBuffer (x);
    assert (c.readBuffer (0) == x);
}

static void testReadIntSampleDelay()
{
    sspo::CircularBuffer<int> c;
    for (auto i = 0; i < 10; ++i)
        c.writeBuffer (i);
    for (auto i = 0; i < 10; ++i)
        assertEQ (c.readBuffer (i), 9 - i);
}

static void testReadFloatSampleDelay()
{
    sspo::CircularBuffer<float> c;
    c.writeBuffer (0.5f);
    c.writeBuffer (1.0f);
    assert (AudioMath::areSame (c.readBuffer (0.5f), 0.75f));
}

static void testReadBufferWrap()
{
    sspo::CircularBuffer<int> c (8);
    for (auto i = 0; i < 10; ++i)
        c.writeBuffer (i);
    for (auto i = 0; i < 8; ++i)
        assertEQ (c.readBuffer (i), 9 - i);
}

static void testReadFloat4SampleDelay()
{
    sspo::CircularBuffer<float_4> c;
    c.writeBuffer (float_4 (0.5f, 1.5f, 2.5f, 3.5f));
    c.writeBuffer (float_4 (1.0f, 2.0f, 3.0f, 4.0f));

    auto result = c.readBuffer (float_4 (0.5f, 0.0f, 1.0f, 0.5f));
    assert (AudioMath::areSame (result[0], 0.75f));
    assert (AudioMath::areSame (result[1], 2.0f));
    assert (AudioMath::areSame (result[2], 2.5f));
    assert (AudioMath::areSame (result[3], 3.75f));
}

void testCircularBuffer()
{
    printf ("testCircularBuffer\n");
    testDefaultSize();
    testConstructorSize();
    testSizePow2();
    testResetEmpty();
    testReadZeroDelay();
    testReadIntSampleDelay();
    testReadFloatSampleDelay();
    testReadFloat4SampleDelay();
    testReadBufferWrap();
}
