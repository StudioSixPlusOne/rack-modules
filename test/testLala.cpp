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

#include "ExtremeTester.h"
#include "LaLa.h"
#include "TestComposite.h"
#include "asserts.h"
#include "digital.hpp"
#include "math.hpp"
#include "testSignal.h"

namespace ts = sspo::TestSignal;

using Lala = LaLaComp<TestComposite>;

static void test01()
{
    Lala lala;
    lala.init();
    lala.step();
}

static void testExtreme (float sr)
{
    Lala lala;
    std::vector<std::pair<float, float>> paramLimits;
    lala.setSampleRate (sr);
    lala.init();

    paramLimits.resize (lala.NUM_PARAMS);
    using fp = std::pair<float, float>;

    auto iComp = Lala::getDescription();
    for (int i = 0; i < iComp->getNumParams(); ++i)
    {
        auto desc = iComp->getParam (i);
        fp t (desc.min, desc.max);
        paramLimits[i] = t;
    }

    ExtremeTester<Lala>::test (lala, paramLimits, true, "Lala");
}

void testLala()
{
    printf ("testLala\n");
    test01();
    testExtreme (96000.0f);
}
