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

// The second revision of a pole mixing ladder filter, the original is
// SynthFilter, I have decided to leave the original unchanged as it is
// used in Maccamo and Amburg filters.#
// Pole mixing is via the Oberheim method
// This revision allows for independent gains of each stage, and non-linear
// distortion between stages. This NLD is implemented with lookup tables
// to allow to sampling of electronic circuits as used in physical modules.
// Oversampling shall be required and should be provided external to this class.

#pragma once

#include <cmath>
#include <vector>
#include <functional>
#include "AudioMath.h"
#include "UtilityFilters.h"
//#include "simd/vector.hpp"
#include "simd/functions.hpp"
#include "simd/sse_mathfun.h"
#include "simd/sse_mathfun_extension.h"

namespace sspo
{
    namespace SynthFilterII
    {
        /// One Pole lowpass filter used internally by SynthFilterII
        template <typename T>
        class OnePoleLpFilter
        {
        private:
            /// singular filter coefficient
            T feedForward{ 0 };

            /// last value storage
            T z1{ 0 };

            float inverseSr = 0.0;

        public:
            OnePoleLpFilter() = default;

            void setSampleRate(float sr)
            {
                inverseSr = 1.0f / sr;
            }

            void setFrequency (T fc)
            {
                auto wd = 2.0f * AudioMath::k_pi * fc;
                auto wa = (2.0f / inverseSr) * rack::simd::tan (wd * inverseSr / 2.0f);
                auto g = wa * inverseSr / 2.0f;
                feedForward = g / (1.0f + g);
            }

            T process (T in)
            {
                auto xn = in;
                auto vn = (xn - z1) * feedForward;
                auto lp = vn + z1;
                z1 = vn + lp;
                return lp;
            }
        };
    } // namespace SynthFilterII
} // namespace sspo
