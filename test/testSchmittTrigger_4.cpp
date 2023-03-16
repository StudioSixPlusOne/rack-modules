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

#include "SchmittTrigger.h"

namespace ts = sspo::TestSignal;

static void testConstructor()
{
    sspo::SchmittTrigger<float_4> st;
    assertClose (st.isHigh().s[0], 1.0f, 0.001f);
    assertClose (st.isHigh().s[1], 1.0f, 0.001f);
    assertClose (st.isHigh().s[2], 1.0f, 0.001f);
    assertClose (st.isHigh().s[3], 1.0f, 0.001f);
}

static void testReset4()
{
    sspo::SchmittTrigger<float_4> st;
    st.reset();
    assertClose (st.isHigh().s[0], 1.0f, 0.001f);
    assertClose (st.isHigh().s[1], 1.0f, 0.001f);
    assertClose (st.isHigh().s[2], 1.0f, 0.001f);
    assertClose (st.isHigh().s[3], 1.0f, 0.001f);
}

static void testReset()
{
    sspo::SchmittTrigger<float> st;
    st.reset();
    assertClose (st.isHigh(), 1.0f, 0.001f);
}

static void testFalseOnZero4()
{
    sspo::SchmittTrigger<float_4> st;
    float_4 testTriggers{ 0.0f, 1.0f, 0.0f, 1.0f };

    float_4 result = st.process (testTriggers);

    assertClose (st.isHigh().s[0], 0.0f, 0.001f);
    assertClose (st.isHigh().s[1], 1.0f, 0.001f);
    assertClose (st.isHigh().s[2], 0.0f, 0.001f);
    assertClose (st.isHigh().s[3], 1.0f, 0.001f);

    assertClose (result.s[0], 0.0f, 0.001f);
    assertClose (result.s[1], 0.0f, 0.001f);
    assertClose (result.s[2], 0.0f, 0.001f);
    assertClose (result.s[3], 0.0f, 0.001f);
}

static void testFalseOnZero()
{
    sspo::SchmittTrigger<float> st;

    auto result = st.process (1.0f);
    assertClose (st.isHigh(), 1.0f, 0.001f);

    result = st.process (0.0f);
    assertClose (st.isHigh(), 0.0f, 0.001f);
    assertClose (result, 0.0f, 0.001f);
}

static void testTrueFirstOne4()
{
    printf ("testTrueFirstOne4\n");
    sspo::SchmittTrigger<float_4> st;
    //init all trig positions to zero
    float_4 testTriggers0{ 0.0f, 0.0f, 0.0f, 0.0f };
    st.process (testTriggers0);

    assertClose (st.isHigh().s[0], 0.0f, 0.001f);
    assertClose (st.isHigh().s[1], 0.0f, 0.001f);
    assertClose (st.isHigh().s[2], 0.0f, 0.001f);
    assertClose (st.isHigh().s[3], 0.0f, 0.001f);

    //first triggers on 1 + 3
    float_4 testTriggers1{ 0.0f, 1.0f, 0.0f, 1.0f };
    float_4 result = st.process (testTriggers1);

    assertClose (st.isHigh().s[0], 0.0f, 0.001f);
    assertClose (st.isHigh().s[1], 1.0f, 0.001f);
    assertClose (st.isHigh().s[2], 0.0f, 0.001f);
    assertClose (st.isHigh().s[3], 1.0f, 0.001f);

    assertClose (result.s[0], 0.0f, 0.001f);
    assertClose (result.s[1], 1.0f, 0.001f);
    assertClose (result.s[2], 0.0f, 0.001f);
    assertClose (result.s[3], 1.0f, 0.001f);
}

static void testTrueFirstOne()
{
    printf ("testTrueFirstOne4\n");
    sspo::SchmittTrigger<float> st;
    //init all trig positions to zero
    float testTriggers0 = 0;
    st.process (testTriggers0);

    assertClose (st.isHigh(), 0.0f, 0.001f);

    //first triggers on 1 + 3
    float testTriggers1 = 1.0f;
    auto result = st.process (testTriggers1);

    assertClose (st.isHigh(), 1.0f, 0.001f);
    assertClose (result, 1.0f, 0.001f);
}

static void testFalseSecondOne4()
{
    sspo::SchmittTrigger<float_4> st;
    //init all trig positions to zero
    float_4 testTriggers0{ 0.0f, 0.0f, 0.0f, 0.0f };
    st.process (testTriggers0);

    //first triggers on 1 + 3
    float_4 testTriggers1{ 0.0f, 1.0f, 0.0f, 1.0f };
    st.process (testTriggers1);
    float_4 result = st.process (testTriggers1);

    assertClose (result.s[0], 0.0f, 0.001f);
    assertClose (result.s[1], 0.0f, 0.001f);
    assertClose (result.s[2], 0.0f, 0.001f);
    assertClose (result.s[3], 0.0f, 0.001f);

    assertClose (st.isHigh().s[0], 0.0f, 0.001f);
    assertClose (st.isHigh().s[1], 1.0f, 0.001f);
    assertClose (st.isHigh().s[2], 0.0f, 0.001f);
    assertClose (st.isHigh().s[3], 1.0f, 0.001f);
}

static void testFalseSecondOne()
{
    sspo::SchmittTrigger<float> st;
    //init all trig positions to zero
    float testTriggers0 = 0;;
    st.process (testTriggers0);

    //first triggers on 1 + 3
    float testTriggers1 = 1.0;
    st.process (testTriggers1);
    auto result = st.process (testTriggers1);

    assertClose (result, 0.0f, 0.001f);
    assertClose (st.isHigh(), 1.0f, 0.001f);

}

void testSchmittTrigger()
{
    printf ("test SchmittTrigger\n");
    testConstructor();
    testReset4();
    testFalseOnZero4();
    testTrueFirstOne4();
    testFalseSecondOne4();

    testReset();
    testFalseOnZero();
    testTrueFirstOne();
    testFalseSecondOne();


}