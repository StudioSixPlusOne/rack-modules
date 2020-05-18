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

#include "AudioMath.h"
#include <assert.h>
#include <random>
#include <time.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>

namespace am = sspo::AudioMath;

namespace sspo
{
    /** test signal in the range -1.0 1.0
     * meed to be multiplied by *5 for vcv audio gates * 10
     * this is slow and should not be used for realtime processing
     * not realtime safe as may perform memory allocations
     */
    namespace TestSignal
    {
        /** wrapper for std::vector<float> where += concatenates
         */
        struct Signal : std::vector<float>
        {
        };

        inline Signal& operator+= (Signal& s1, const Signal& s2)
        {
            s1.insert (s1.end(), s2.begin(), s2.end());
            return s1;
        }

        inline Signal operator+ (const Signal& s1, const Signal& s2)
        {
            Signal ret;
            ret += s1;
            ret += s2;
            return ret;
        }

        inline Signal operator* (const Signal& s1, float x)
        {
            Signal ret;
            for (auto s : s1)
                ret.push_back (s * x);

            return ret;
        }

        inline Signal makeZeros (int length)
        {
            Signal ret;
            ret.assign (length, 0.0f);
            return ret;
        }

        inline Signal makeFixed (int length, float val)
        {
            Signal ret;
            ret.assign (length, val);
            return ret;
        }

        inline Signal makeNoise (int length)
        {
            Signal ret;
            std::default_random_engine defaultGenerator{ time (NULL) };
            std::uniform_real_distribution<float> distribution{ -1.0f, 1.0 - FLT_EPSILON };
            for (auto i = 0; i < length; ++i)
                ret.push_back (distribution (defaultGenerator));

            return ret;
        }

        inline Signal makeDriac (int length)
        {
            Signal ret;
            ret.assign (length, 0.0f);
            ret[0] = 1.0;
            return ret;
        }

        inline Signal makeTrigger (int length, int highLength)
        {
            Signal ret;
            if (length >= highLength)
            {
                for (auto i = 0; i < highLength; ++i)
                {
                    ret.push_back (1.0f);
                }
                for (auto i = 0; i < length - highLength; ++i)
                {
                    ret.push_back (0.0f);
                }
            }
            return ret;
        }

        inline Signal makeClockTrigger (int interval, int count)
        {
            Signal ret;
            auto trigger = makeTrigger (interval, 44);
            for (auto i = 0; i < count; ++i)
                ret += trigger;

            return ret;
        }

        inline Signal makeSine (int length, float freq, float sr, float amp = 1.0f)
        {
            auto inc = 1.0 / sr;
            auto phase = 0.0;
            Signal ret;
            for (auto i = 0; i < length; ++i)
            {
                ret.push_back (amp * std::sin (am::k_2pi * freq * phase));
                phase += inc;
            }
            return ret;
        }

        inline Signal mix (const Signal& s1, const Signal& s2)
        {
            assert (s1.size() == s2.size());
            Signal ret;
            for (auto i = 0; i < s1.size(); ++i)
                ret.push_back (s1[i] + s2[i]);

            return ret;
        }

        inline bool toFile (const Signal& s1, const std::string filename)
        {
            std::ofstream outStream{ filename };
            if (! outStream)
                return false;

            outStream << std::setprecision (14);
            for (auto x : s1)
                outStream << x << "\n";
            return true;
        }

        inline Signal fromFile (const std::string filename)
        {
            Signal ret;
            std::ifstream inStream{ filename };
            if (inStream)
            {
                for (float x; inStream >> x;)
                    ret.push_back (x);
            }
            return ret;
        }

        inline Signal noiseFromFile()
        {
            std::string filename (".//test//signal//noise500000.dat");
            return fromFile (filename);
        }

    } // namespace TestSignal
} // namespace sspo