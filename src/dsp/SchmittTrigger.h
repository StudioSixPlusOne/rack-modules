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

using namespace rack;

using float_4 = ::rack::simd::float_4;

namespace sspo
{
    template <typename T>
    class SchmittTrigger
    {
    public:
        void reset()
        {
            state = T (1.0f);
        }
        const T process (T triggers)
        {
            auto oldState = state;
            state = simd::ifelse ((state == onState),
                                  simd::ifelse (triggers <= offThreshold, 0.0f, state),
                                  simd::ifelse (triggers >= onThreshold, 1.0f, state));

            result = simd::ifelse (state - oldState == 1.0f, 1.0f, 0.0f);
#if 0
            auto sub = state - oldState;

        printf("state %f %f %f %f%\n", state.s[0],state.s[1],state.s[2],state.s[3]);
        printf("oldState %f %f %f %f%\n", oldState.s[0],oldState.s[1],oldState.s[2],oldState.s[3]);
        printf("sub %f %f %f %f%\n", sub.s[0],sub.s[1],sub.s[2],sub.s[3]);
        printf("result %f %f %f %f%\n", result.s[0],result.s[1],result.s[2],result.s[3]);
#endif
            return result;
        }

        void setOnThreshold (T thresholds)
        {
            onThreshold = thresholds;
        }

        void setOffThreshold (T thresholds)
        {
            offThreshold = thresholds;
        }

        const T& isHigh()
        {
            return state;
        }

    private:
        T state = T (1.0f);
        T result = T (1.0f);
        const T onState = T (1.0f);
        T offThreshold{ 0 };
        T onThreshold = T (1.0f);
    };

} // namespace sspo