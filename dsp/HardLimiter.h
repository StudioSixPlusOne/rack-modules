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

#include <cmath>
#include <utility>
#include <algorithm>

#include "LookupTable.h"

namespace sspo
{
    ///
    /// !A fixed parameter limiter 
    /// Based on description given in Prikle, Designing Audio Effects 2nd
    ///
    struct Limiter 
    {
        Limiter() = default;

        void calcCoeffs()
        {
            attackCoeff = std::exp (TC / (sampleRate * attackTime));
            releaseCoeff = std::exp (TC / (sampleRate * releaseTimes));
        }

        void setSampleRate(const float sr)
        {
            sampleRate = sr;
            calcCoeffs();
        }

        void setTimes(const float attack, const float release)
        {
            attackTime = attack;
            releaseTimes = release;
            calcCoeffs();
        }

        float process(const float in)
        {
            //envelope follower
            auto rectIn = std::abs (in);
            currentEnv = rectIn > lastEnv 
                ? attackCoeff * (lastEnv - rectIn) + rectIn
                : releaseCoeff * (lastEnv - rectIn) + rectIn;
            currentEnv = std::max (currentEnv, 0.00000000001f);
            lastEnv = currentEnv;

            auto dn = 20.0f * lookup.log10 (currentEnv);

            //Hard knee compression
            auto yndB = dn <= threshold ? dn : threshold + ((dn - threshold) / ratio);
            auto gndB = yndB - dn;
            auto G = lookup.pow10 ( gndB / 20.0f);

            return in * G;
        }
        
        float attackTime{ 0.0001f };
        float releaseTimes{ 0.0025f };
        float ratio{ 10.5f };
        float threshold{ -0.0f }; //dB

        private:

        float attackCoeff{ 0.0f };
        float releaseCoeff{ 0.0f };
        float lastEnv{ 0.0f };
        float currentEnv{ 0.0f };
        float sampleRate{ 1.0f };

        static constexpr float TC{ -0.9996723408f }; // { std::log (0.368f); } //capacitor discharge to 36.8%
    };

    struct Saturator
    {
        float max = 1.0f;
        float kneeWidth = 0.05f;

        float process(float in) const
        {
            auto ret = 0.0f;
            if (std::abs(in) < (max - kneeWidth))
            {
                ret = in;
            }
            else 
            {
                if (std::abs(in) < max)
                {
                    ret = in - ((std::pow (in - max + (kneeWidth / 2.0f), 2.0f)) / (2.0f * kneeWidth));
                    ret = clamp(ret, -max, max);
                }
                else 
                    ret = in > 0.0f
                        ? max
                        : -max;
            }

            return ret; 
        }

    };
}