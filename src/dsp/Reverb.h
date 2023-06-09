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
#include <cmath>
#include <utility>
#include "simd/functions.hpp"
#include "simd/sse_mathfun.h"
#include "simd/sse_mathfun_extension.h"
#include "LookupTable.h"
#include "AudioMath.h"
#include "CircularBuffer.h"

using namespace rack;

using float_4 = ::rack::simd::float_4;

namespace sspo
{
    namespace Reverb
    {
        template <typename T>
        class CombFilter
        {
        public:
            //default constructor
            //setup for 44100 SR
            CombFilter()
            {
                setSampleRate (44100.0f);
            }

            T step (T in)
            {
                auto y = delayLine.readBuffer (sampleRate * D);
                delayLine.writeBuffer (in + y * g);
                return y;
            }

            void setSampleRate (float sr)
            {
                sampleRate = sr;
                delayLine.reset (sampleRate * maxDelayTime);
            }

            void setParameters (float feedback, float delayTimeSeconds)
            {
                g = math::clamp (feedback, 0.0f, 1.0f);
                D = math::clamp (delayTimeSeconds, 0.0f, maxDelayTime);
            }

            static constexpr float maxDelayTime = 0.050f; // 50 mSec

        private:
            sspo::CircularBuffer<T> delayLine;
            float sampleRate = 44100.0f;
            float g = 0.8f;
            float D = maxDelayTime;
        };
    } // namespace Reverb
} // namespace sspo