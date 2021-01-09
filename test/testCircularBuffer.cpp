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
#include "../src/dsp/CircularBuffer.h"
#include <assert.h>
#include <stdio.h>
#include "testSignal.h"
#include "utils/FftAnalyzer.h"
#include <algorithm>

using namespace sspo;

static void testDefaultSize()
{
    CircularBuffer<float> c;
    assert (c.size() == 4096);
}

static void testConstructorSize()
{
    CircularBuffer<float> c (512);
    assert (c.size() == 512);
}

static void testSizePow2()
{
    CircularBuffer<double> c (6);
    assert (c.size() == 8);

    CircularBuffer<int> d (500);
    assert (d.size() == 512);

    CircularBuffer<long long> e (4000);
    assert (e.size() == 4096);
}

static void testResetEmpty()
{
    CircularBuffer<float> c;
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
    CircularBuffer<float> c;
    auto x = 0.7f;
    c.writeBuffer (x);
    assert (c.readBuffer (0) == x);
}

static void testReadIntSampleDelay()
{
    CircularBuffer<int> c;
    for (auto i = 0; i < 10; ++i)
        c.writeBuffer (i);
    for (auto i = 0; i < 10; ++i)
        assertEQ (c.readBuffer (i), 9 - i);
}

static void testReadFloatSampleDelay()
{
    CircularBuffer<float> c;
    c.writeBuffer (0.5f);
    c.writeBuffer (1.0f);
    assert (AudioMath::areSame (c.readBuffer (0.5f), 0.75f));
}

static void testReadBufferWrap()
{
    CircularBuffer<int> c (8);
    for (auto i = 0; i < 10; ++i)
        c.writeBuffer (i);
    for (auto i = 0; i < 8; ++i)
        assertEQ (c.readBuffer (i), 9 - i);
}

static void testAddIntIndex()
{
    CircularBuffer<int> c (8);
    auto x = 3;
    auto y = 4;
    c.addBuffer (4, x);
    assertEQ (c.readBuffer (4), x);
    c.addBuffer (4, y);
    assertEQ (c.readBuffer (4), x + y);
}

// ****************************
// WaveGuide
// ****************************

static void testWGDefaultConstructor()
{
    auto wg = WaveGuide<float>();
    assertEQ (wg.getBufferSize(), 4096);
}

static void testWGCustomConstructor()
{
    auto n0 = std::make_shared<WaveGuide<float>::Node>();
    auto n1 = std::make_shared<WaveGuide<float>::Node>();
    auto wg = WaveGuide<float> (n0, n1, 8000);
    assertEQ (wg.getBufferSize(), 8192);
    wg.addBuffer (0.0f, 1.0);
    assertEQ (wg.readBuffer (0.0f), 1.0);
    wg.addBuffer (0.3f, 1.0);
    assertEQ (wg.readBuffer (0.3f), 1.0);
}

static void testWGStepDefaultNode()
{
    {
        // place a single data point x in the buffer
        // step once and check the two adjacent nodes contain x/2
        auto x = 3.0f;
        WaveGuide<float> wg;
        wg.setBufferSize (16);
        wg.setLength (10);
        wg.addBuffer (0.5f, x);
        assertEQ (wg.readBuffer (0.5f), x);
        wg.step();
        assertEQ (AudioMath::areSame (wg.readBuffer (0.6f), x / 2.0f), true);
        assertEQ (AudioMath::areSame (wg.readBuffer (0.4f), x / 2.0f), true);
        wg.step();
        assertEQ (AudioMath::areSame (wg.readBuffer (0.7f), x / 2.0f), true);
        assertEQ (AudioMath::areSame (wg.readBuffer (0.3f), x / 2.0f), true);
        wg.step();
        assertEQ (AudioMath::areSame (wg.readBuffer (0.8f), x / 2.0f), true);
        assertEQ (AudioMath::areSame (wg.readBuffer (0.2f), x / 2.0f), true);
        wg.step();
        assertEQ (AudioMath::areSame (wg.readBuffer (0.9f), x / 2.0f), true);
        assertEQ (AudioMath::areSame (wg.readBuffer (0.1f), x / 2.0f), true);
        wg.step();
        assertEQ (wg.readBuffer (1.0f), x / 2.0f);
        assertEQ (wg.readBuffer (0.0f), x / 2.0f); // unsure this is corect
        //        printf ("%s\n", wg.toString().c_str());
        wg.step();
        assertEQ (wg.readBuffer (0.9f), x / 2.0f);
        //        assertEQ (wg.readBuffer (0.1f), x / 2.0f);
        //        wg.step();
        //        assertEQ (wg.readBuffer (0.8f), x / 2.0f);
        //        assertEQ (wg.readBuffer (0.2f), x / 2.0f);
        //        wg.step();
        //        assertEQ (wg.readBuffer (0.7f), x / 2.0f);
        //        assertEQ (wg.readBuffer (0.3f), x / 2.0f);
        //        wg.step();
        //        assertEQ (wg.readBuffer (0.6f), x / 2.0f);
        //        assertEQ (wg.readBuffer (0.5f), x / 2.0f);
        //        wg.step();
        //        assertEQ (wg.readBuffer (0.5f), x);
        //        assertEQ (wg.readBuffer (0.4f), 0.0f);
        //        assertEQ (wg.readBuffer (0.6f), 0.0f);
    }
    {
        WaveGuide<float> wg;
        wg.setBufferSize (16);
        auto bufferLength = 10;
        auto runlength = 50;
        auto inFrac = 0.1f;
        auto outFrac = 0.2f;
        wg.setLength (bufferLength);

        auto driac = sspo::TestSignal::makeDriac (runlength);
        auto result = sspo::TestSignal::Signal();
        for (auto s : driac)
        {
            wg.step();
            wg.addBuffer (inFrac, s);
            //        wg.addBuffer(0.0f, 1);
            auto r = wg.readBuffer (outFrac);
            result.push_back (r);
            //        printf(" %f ", r);
        }
        //    printf("\n");

        assertEQ (std::accumulate (result.begin(), result.end(), 0.0f), 5.0f);
    }
}

static void printWGrotation()
{
    WaveGuide<float> wg;
    wg.setBufferSize (16);
    wg.setLength (10);
    auto x = 3.0f;
    wg.addBuffer (0.5, x);
    for (auto i = 0; i < 50; i++)
    {
        //        printf ("step No %d \n", i);
        //        printf ("%s\n", wg.toString().c_str());
        wg.step();
    }
}

static void testWGNode()
{
    class Attenuator : public WaveGuide<float>::Node
    {
    public:
        float step (float x) override
        {
            return x * 0.7f;
        }
    };

    Attenuator n1;

    assertEQ (n1.step (1.0f), 0.7f);
}

static void testWGStepCustomNodes()
{
    class Attenuator : public WaveGuide<float>::Node
    {
    public:
        float step (float x) override
        {
            return x * sf;
        }

        float sf = 1.0f;
    };

    auto n0 = std::make_shared<Attenuator>();
    auto n1 = std::make_shared<Attenuator>();

    n0->sf = 0.9f;
    n1->sf = 0.1f;

    WaveGuide<float> wg (n0, n1);
    wg.setBufferSize (16);
    auto bufferLength = 10;
    auto runlength = 50;
    auto inFrac = 0.1f;
    auto outFrac = 0.2f;
    wg.setLength (bufferLength);

    auto driac = sspo::TestSignal::makeDriac (runlength);
    auto result = sspo::TestSignal::Signal();
    for (auto s : driac)
    {
        wg.step();
        wg.addBuffer (inFrac, s);
        //        wg.addBuffer(0.0f, 1);
        auto r = wg.readBuffer (outFrac);
        result.push_back (r);
        //        printf (" %f ", r);
    }
    //    printf ("\n");

    assertEQ (result[1], 0.5f);
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
    testReadBufferWrap();
    testAddIntIndex();

    // ****************************
    // WaveGuide
    // ****************************

    printWGrotation();
    testWGDefaultConstructor();
    testWGCustomConstructor();
    testWGStepDefaultNode();
    testWGNode();
    testWGStepCustomNodes();
}