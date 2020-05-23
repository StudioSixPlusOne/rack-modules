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

#include "common.hpp"
#include "random.hpp"
#include "filter.hpp"
#include "digital.hpp"
#include "math.hpp"

#include "asserts.h"
#include <stdio.h>
#include <limits>
#include "HardLimiter.h"

constexpr float inc = 0.00001f;
constexpr float epslion = 0.00001f;

static void testBelowLimit (float limit, float knee)
{
    sspo::Saturator sat (limit, knee);

    for (auto i = -limit + knee + inc; i < limit - knee; i += inc)
    {
        //printf ("%f\n", i);
        assertClose (sat.process (i), i, epslion);
    }
}

static void testWithinKnee (float limit, float knee)
{
    sspo::Saturator sat (limit, knee);
    auto crossDistortionband = knee * 0.02f;
    auto cdb = crossDistortionband;

    auto allowableDistortion = 0.05f;
    auto ad = allowableDistortion;

    for (auto i = -limit; i < -limit + knee - cdb; i += inc)
    {
        printf ("%f\n", i);

        assertGE (sat.process (i), -limit);
        assertLE (sat.process (i), -(limit - knee) + ad);
    }

    for (auto i = limit; i > limit - knee + cdb; i -= inc)
    {
        printf ("%f\n", i);
        assertLE (sat.process (i), limit);
        assertGE (sat.process (i), limit - knee - ad);
    }
}

static void testAboveLimit (float limit, float knee)
{
    sspo::Saturator sat (limit, knee);

    for (auto i = limit; i < limit + 20; i += inc)
        assertClose (sat.process (i), limit, epslion)

            for (auto i = -limit; i > -limit - 20; i -= inc)
                assertClose (sat.process (i), -limit, epslion)
}

static void testInfinate (float limit, float knee)
{
    sspo::Saturator sat (limit, knee);
    assertClose (sat.process (std::numeric_limits<float>::infinity()), limit, epslion);
    assertClose (sat.process (-std::numeric_limits<float>::infinity()), -limit, epslion);
}

static void testDefaultConstructor()
{
    sspo::Saturator sat;

    auto limit = sat.max;
    auto knee = sat.kneeWidth;

    assertClose (limit, 1.0f, epslion);
    assertClose (knee, 0.05f, epslion);
}

static void testVoltageSaturator()
{
    sspo::Saturator sat (11.7, 0.5f);

    for (auto i = -150.0f; i < 150.0f; i += inc)
        assertClose (sspo::voltageSaturate (i), sat.process (i), epslion);
}

void testSaturator()
{
    printf ("testSaturator\n");
    testBelowLimit (1.0f, 0.05f);
    testBelowLimit (11.7f, 0.5f);
    testWithinKnee (1.0f, 0.05f);
    testWithinKnee (11.7f, 0.5f);
    testAboveLimit (1.0f, 0.05f);
    testAboveLimit (11.7f, 0.5f);
    testInfinate (1.0f, 0.05f);
    testInfinate (11.7f, 0.5f);
    testDefaultConstructor();
    testVoltageSaturator();
}
