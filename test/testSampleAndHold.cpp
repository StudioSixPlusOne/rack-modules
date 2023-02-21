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

#include <array>
#include <assert.h>
#include <stdio.h>
#include "dsp/digital.hpp"
#include "TestComposite.h"
#include "ExtremeTester.h"
#include "Analyzer.h"
#include "testSignal.h"

#include "SampleAndHold.h"

namespace ts = sspo::TestSignal;

static void testConstructor()
{
    sspo::SampleAndHold<float> sh;
    assertEQ (1, 1)
}

static void testMonoNoTrig()
{
    sspo::SampleAndHold<float> sh;
    auto data = -7.2f;
    auto result = sh.step (data, 0.0f);

    assertClose (result, 0.0f, 0.0001f)
}

static void testMonoZeroOneTrig()
{
    sspo::SampleAndHold<float> sh;
    auto data = -7.2f;
    sh.step (data, 0.0f);
    auto result = sh.step (data, 1.0f);

    assertClose (result, data, 0.0001f)
}

static void testMonoZeroOneZeroTrig()
{
    sspo::SampleAndHold<float> sh;
    auto data = -7.2f;
    sh.step (data, 0.0f);
    sh.step (data, 1.0f);
    auto result = sh.step (0.0f, 0.0f);
    assertClose (result, data, 0.0001f)
}

static void test4NoTrig()
{
    sspo::SampleAndHold<float_4> sh;
    auto data = float_4 (-7.2f, 4.1f, 0.0f, 9.2f);
    auto trigger = float_4::zero();
    auto result = sh.step (data, trigger);

    assertClose (result[0], 0.0f, 0.0001f);
    assertClose (result[1], 0.0f, 0.0001f);
    assertClose (result[2], 0.0f, 0.0001f);
    assertClose (result[3], 0.0f, 0.0001f);
}

static void test4ZeroOneTrig()
{
    sspo::SampleAndHold<float_4> sh;
    auto data = float_4 (-7.2f, 4.1f, 0.0f, 9.2f);
    auto trigger = float_4::zero();
    auto result = sh.step (data, trigger);
    trigger = float_4 (1.0f);
    result = sh.step (data, trigger);

    assertClose (result[0], -7.2f, 0.0001f);
    assertClose (result[1], 4.1f, 0.0001f);
    assertClose (result[2], 0.0f, 0.0001f);
    assertClose (result[3], 9.2f, 0.0001f);
}

static void test4ZeroOneZeroTrig()
{
    sspo::SampleAndHold<float_4> sh;
    auto data = float_4 (-7.2f, 4.1f, 0.0f, 9.2f);

    //zero
    auto trigger = float_4::zero();
    auto result = sh.step (data, trigger);

    //One
    trigger = float_4 (1.0f);
    result = sh.step (data, trigger);

    //Zero
    trigger = float_4::zero();
    result = sh.step (data, trigger);

    assertClose (result[0], -7.2f, 0.0001f);
    assertClose (result[1], 4.1f, 0.0001f);
    assertClose (result[2], 0.0f, 0.0001f);
    assertClose (result[3], 9.2f, 0.0001f);
}

static void testDroop()
{
    sspo::SampleAndHold<float_4> sh;
    sh.setUseDroop(float_4(1.0f));
    auto data = float_4 (-7.2f, 4.1f, 0.0f, 9.2f);

    //zero
    auto trigger = float_4::zero();
    auto result = sh.step (data, trigger);

    //One
    trigger = float_4 (1.0f);
    result = sh.step (data, trigger);

    for (auto i = 0U; i < 10000; ++i)
    {
        //Zero
        trigger = float_4::zero();
        result = sh.step (data, trigger);
    }

    assertGT (result[0], -7.2f);
    assertLT (result[1], 4.1f);
    assertEQ (result[2], 0.0f);
    assertLT (result[3], 9.2f);

    assertClose (result[0], -7.2f, 0.01f);
    assertClose (result[1], 4.1f, 0.01f);
    assertClose (result[2], 0.0f, 0.0001f);
    assertClose (result[3], 9.2f, 0.01f);
}

static void testNoDroop()
{
    sspo::SampleAndHold<float_4> sh;
    sh.setUseDroop(float_4::zero());
    auto data = float_4 (-7.2f, 4.1f, 0.0f, 9.2f);

    //zero
    auto trigger = float_4::zero();
    auto result = sh.step (data, trigger);

    //One
    trigger = float_4 (1.0f);
    result = sh.step (data, trigger);

    for (auto i = 0U; i < 10000; ++i)
    {
        //Zero
        trigger = float_4::zero();
        result = sh.step (data, trigger);
    }

    assertClose (result[0], -7.2f, 0.0001f);
    assertClose (result[1], 4.1f, 0.0001f);
    assertClose (result[2], 0.0f, 0.0001f);
    assertClose (result[3], 9.2f, 0.0001f);
}

void testSampleAndHold()
{
    printf ("test SampleAndHold\n");
    testConstructor();
    testMonoNoTrig();
    testMonoZeroOneTrig();
    testMonoZeroOneZeroTrig();
    test4NoTrig();
    test4ZeroOneTrig();
    test4ZeroOneZeroTrig();
    testNoDroop();
    testDroop();
}