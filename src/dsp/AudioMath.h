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

#pragma once

#include <stdio.h>
#include <algorithm>
#include <cmath>
#include <float.h>
#include <vector>
#include <random>

//#include "LookupTable.h"
//#include "lookupTables/SineTable.h"

namespace sspo
{
    namespace AudioMath
    {
        constexpr auto LD_PI = 3.14159265358979323846264338327950288419716939937510L;
        constexpr auto k_pi = static_cast<float> (LD_PI);
        constexpr auto k_2pi = k_pi + k_pi;
        constexpr auto base_a4 = 440.0f;
        constexpr auto base_a4Midi = 69.0f;
        constexpr auto semitonesPerOctave = 12.0f;
        constexpr double Ln10 = 2.30258509299404568402;

        //* accurate to 0.032f when -5.0 < x < 5.0
        template <typename T>
        inline T fastTanh (T x)
        {
            return x * (27 + x * x) / (27 + 9 * x * x);
        }

        template <typename T>
        inline bool areSame (T a, T b, T delta = FLT_EPSILON) noexcept
        {
            return std::abs (a - b) <= delta;
        }

        template <typename T>
        inline bool areSame (const std::vector<T>& a, const std::vector<T>& b, const T delta = FLT_EPSILON)
        {
            return a.size() == b.size()
                       ? std::equal (a.begin(), a.end(), b.begin(), [=] (const T& l, const T& r) -> bool {
                             return areSame (l, r, delta);
                         })
                       : false;
        }

        template <typename T>
        inline T linearInterpolate (const T v0, const T v1, const T frac) noexcept
        {
            return frac * (v1 - v0) + v0;
        }

        template <typename T>
        class ZeroCrossing
        {
        public:
            ZeroCrossing()
            {
            }

            bool process (const T x)
            {
                auto positive = x > 0;
                auto ret = (positive != lastPositive);
                lastPositive = positive;
                return ret;
            }

        private:
            bool lastPositive = false;
        };

        static std::default_random_engine defaultGenerator{ 99 };
        static std::uniform_real_distribution<float> distribution{ 0.0, 1.0 - FLT_EPSILON };

        inline float rand01()
        {
            return distribution (defaultGenerator);
        }

        inline float db (float g)
        {
            return 20 * log (g) / Ln10;
        }

        inline float gainFromDb (float db)
        {
            return std::exp (Ln10 * db / 20.0);
        }

    } // namespace AudioMath
} // namespace sspo
