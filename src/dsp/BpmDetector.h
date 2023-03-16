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

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>
#include "simd/functions.hpp"
#include "simd/sse_mathfun.h"
#include "simd/sse_mathfun_extension.h"
#include "LookupTable.h"
#include "AudioMath.h"
#include "CircularBuffer.h"
#include "SchmittTrigger.h"

using namespace rack;

using float_4 = ::rack::simd::float_4;

namespace sspo
{
    /// Used to detect a BPM, process(clock) to be called every frame,
    /// with the clock input. The duration between leading edges is measured
    /// with a schmitt trigger. The results are buffered to give a smoothed output.

    class BpmDetector
    {
    public:
        void setSampleRate (float sr)
        {
            assert (sr > 0.0f);
            sampleDuration = 1.0f / sr;
        }

        void reset()
        {
            //clear duration history
            clockDurationHistory.clear();

            //clear duration history sum
            clockHistorySum = 0.0f;

            //clear clocks since reset
            clocksSinceReset = 0;
        }

        void step (float clock)
        {
            //            auto trigger
        }

    private:
        static constexpr float DEFAULT_SAMPLE_DURATION = 1.0f / 44100.0f;
        static constexpr int CLOCK_DURATION_HISTORY = 8;

        float sampleDuration = DEFAULT_SAMPLE_DURATION;
        int clocksSinceReset = 0;
        Sspo::CircularBuffer<float> clockDurationHistory = Sspo::CircularBuffer<float> (CLOCK_DURATION_HISTORY);
        float clockHistorySum = 0.0f;
    };
} // namespace sspo