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
#include <vector>
#include "TriggerSequencer.h"
#include "asserts.h"

static void testInit()
{
    sspo::TriggerSequencer<64> trig;
    assertEQ (trig.getMaxLength(), 64);
    assertEQ (trig.getSequence().size(), 64);
}

static void testReadZero()
{
    constexpr int l = 128;
    sspo::TriggerSequencer<l> trig;
    for (auto i = 0; i < l; ++i)
        assertEQ ((int) trig.getStep (i), (int) false);
}

static void testWrite()
{
    std::vector<bool> seq = { true,
                              false,
                              false,
                              true };

    sspo::TriggerSequencer<16> trig;
    for (auto i = 0; i < 4; ++i)
        trig.setStep (i, seq[i]);

    auto result = trig.getSequence();
    for (auto i = 0; i < (int) seq.size(); ++i)
        assertEQ (result[i], seq[i]);
}

static void testRead()
{
    std::vector<bool> seq = { true,
                              false,
                              false,
                              true };

    sspo::TriggerSequencer<16> trig;
    for (auto i = 0; i < (int) seq.size(); ++i)
        trig.setStep (i, seq[i]);

    for (auto i = 0; i < (int) seq.size(); ++i)
        assertEQ (trig.getStep (i), seq[i]);
}

static void testStepLoop()
{
    std::vector<int> seq = { true,
                             false,
                             false,
                             false };

    sspo::TriggerSequencer<16> trig;
    for (auto i = 0; i < (int) seq.size(); ++i)
        trig.setStep (i, seq[i]);
    trig.setLength (3);
    trig.setActive (true);

    for (auto j = 0; j < 2; ++j)
    {
        for (auto i = 0; i < 3; ++i)
        {
            assertEQ (trig.step (true), seq[i]);
            assertEQ (trig.getIndex(), i);
        }
    }
}

static void testStepNoTrig()
{
    sspo::TriggerSequencer<16> trig;
    auto index = trig.getIndex();
    for (auto i = 0; i < 200; ++i)
    {
        trig.step (false);
        assertEQ (trig.getIndex(), index);
    }
}

static void testReset()
{
    sspo::TriggerSequencer<13> trig;
    trig.step (true);
    trig.step (true);
    trig.step (true);

    assertEQ (trig.getIndex(), 2);
    trig.reset();

    assertEQ (trig.getIndex(), -1);
}

static void testActive()
{
    std::vector<int> seq = { true,
                             false,
                             false,
                             false };

    sspo::TriggerSequencer<16> trig;
    trig.setActive (false);
    for (auto i = 0; i < (int) seq.size(); ++i)
        trig.setStep (i, seq[i]);
    trig.setLength (3);

    for (auto j = 0; j < 2; ++j)
    {
        for (auto i = 0; i < 3; ++i)
        {
            assertEQ (trig.step (true), false);
            assertEQ (trig.getIndex(), i);
        }
    }
}

static void testIndex()
{
    sspo::TriggerSequencer<256> trig;
    trig.setLength (256);
    for (auto i = 0; i < 100; ++i)
    {
        trig.step (false);
        assertEQ (trig.getIndex(), -1);
    }

    for (auto i = 0; i < 200; ++i)
    {
        trig.step (true);
        assertEQ (trig.getIndex(), i);
    }
}

void testTriggerSequencer()
{
    printf ("test Trigger Sequencer\n");
    testInit();
    testReadZero();
    testWrite();
    testRead();
    testStepNoTrig();
    testStepLoop();
    testReset();
    testActive();
    testIndex();
}
