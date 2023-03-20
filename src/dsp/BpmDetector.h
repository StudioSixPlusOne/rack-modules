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
    private:
        static constexpr float DEFAULT_SAMPLE_DURATION = 0.5;
        static constexpr int CLOCK_DURATION_HISTORY_SIZE = 8;

        float sampleDuration = DEFAULT_SAMPLE_DURATION;
        float lastSampleDuration = DEFAULT_SAMPLE_DURATION;
        float durationSinceTrigger = 0;
        int clocksSinceReset = 0;
        Sspo::CircularBuffer<float> clockDurationHistory = Sspo::CircularBuffer<float> (CLOCK_DURATION_HISTORY_SIZE);
        float clockHistorySum = 0.0f;
        sspo::SchmittTrigger<float> schmittTrigger;
        float averageCycleDuration = 0;

    public:
        BpmDetector()
        {
            reset();
        }

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

            durationSinceTrigger = 0;
            averageCycleDuration = 0.0f;
        }

        float process (float clock)
        {
            durationSinceTrigger += sampleDuration;

            auto triggered = schmittTrigger.process (clock);
            if (triggered)
            {
                clocksSinceReset++;
                //update average,
                //take the oldest value, subtract from running total
                auto oldHistory = clockDurationHistory.readBuffer (CLOCK_DURATION_HISTORY_SIZE - 1);
                clockHistorySum -= oldHistory;
                //add the new value to history
                clockDurationHistory.writeBuffer (durationSinceTrigger);
                //add new value to running total
                clockHistorySum += durationSinceTrigger;

                averageCycleDuration = clockHistorySum / float (std::min (CLOCK_DURATION_HISTORY_SIZE - 0, clocksSinceReset));
                lastSampleDuration = clocksSinceReset < 1 ? lastSampleDuration : averageCycleDuration;
                durationSinceTrigger = 0;
            }
            return lastSampleDuration;
        }
    };
} // namespace sspo