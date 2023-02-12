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

#include "easing.h"
#include "AudioMath.h"
#include "testSignal.h"

namespace ts = sspo::TestSignal;

using namespace sspo;

//#define makeEasingFiles

#ifdef makeEasingFiles
static void makeEasingFiles (std::string filename,
                             Easings::Easing* easing,
                             float start,
                             float end,
                             float duration,
                             float interval)
{
    printf("Making easing file %s\n", filename.c_str());
    ts::Signal easedin;
    ts::Signal easedout;
    ts::Signal easedinout;
    for (auto i = 0.0f; i < duration; i += interval)
        easedin.push_back (easing->easeIn (i, start, end, duration));
    for (auto i = 0.0f; i < duration; i += interval)
        easedout.push_back (easing->easeOut (i, end, start, duration));
    for (auto i = 0.0f; i < duration; i += interval)
        easedinout.push_back (easing->easeInOut (i, start, end, duration));
    ts::toFile (easedin, filename + "IN");
    ts::toFile (easedout, filename + "OUT");
    ts::toFile (easedinout, filename + "INOUT");
}

static void makeEasingFiles()
{
    Easings::Back back;
    makeEasingFiles ("./test/signal/easing/back", &back, 0.0f, 1.0f, 1.0f, 0.01f);
    Easings::Bounce bounce;
    makeEasingFiles ("./test/signal/easing/bounce", &bounce, 0.0f, 1.0f, 1.0f, 0.01f);
    Easings::Circ circ;
    makeEasingFiles ("./test/signal/easing/circ", &circ, 0.0f, 1.0f, 1.0f, 0.01f);
    Easings::Cubic cubic;
    makeEasingFiles ("./test/signal/easing/cubic", &cubic, 0.0f, 1.0f, 1.0f, 0.01f);
    Easings::Elastic elastic;
    makeEasingFiles ("./test/signal/easing/elastic", &elastic, 0.0f, 1.0f, 1.0f, 0.01f);
    Easings::Expo expo;
    makeEasingFiles ("./test/signal/easing/expo", &expo, 0.0f, 1.0f, 1.0f, 0.01f);
    Easings::Linear linear;
    makeEasingFiles ("./test/signal/easing/linear", &linear, 0.0f, 1.0f, 1.0f, 0.01f);
    Easings::Quad quad;
    makeEasingFiles ("./test/signal/easing/quad", &quad, 0.0f, 1.0f, 1.0f, 0.01f);
    Easings::Quart quart;
    makeEasingFiles ("./test/signal/easing/quart", &quart, 0.0f, 1.0f, 1.0f, 0.01f);
    Easings::Quint quint;
    makeEasingFiles ("./test/signal/easing/quint", &quint, 0.0f, 1.0f, 1.0f, 0.01f);
    Easings::Sine sine;
    makeEasingFiles ("./test/signal/easing/sine", &sine, 0.0f, 1.0f, 1.0f, 0.01f);
}
#endif

static void testEasingFactory()
{
    Easings::EasingFactory ef;

    auto fexpoinout = ts::fromFile ("./test/signal/easing/expoINOUT");
    ts::Signal sig;
    for (auto i = 0.0f; i < 1.0; i += 0.01f)
    {
        sig.push_back (ef.getEasingVector()
                           [int (Easings::EasingFactory::EasingNames::expo)]
                               ->easeInOut (i, 0, 1, 1));
    }
    assert (ts::areSame (fexpoinout, sig));
}

void testEasing()
{
    printf ("testEasing\n");

#ifdef makeEasingFiles
    makeEasingFiles();
#endif
    testEasingFactory();
}
