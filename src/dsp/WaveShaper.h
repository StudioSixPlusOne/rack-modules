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

#include <cassert>
#include <functional>
#include <sstream>
#include <vector>

#include "AudioMath.h"

#include "simd/functions.hpp"
#include "simd/sse_mathfun.h"
#include "simd/sse_mathfun_extension.h"
//#include "simd/vector.hpp"

using float_4 = ::rack::simd::float_4;

namespace sspo
{
    namespace AudioMath
    {

        ///
        /// A waveshaper optomised for use with audio and 12bit lookup table
        /// can be used for both mathematical and procedural generated tables
        /// tablrs should based on -1.0 tp 1.0 to represent +-10v and extended
        /// to +-1.2 to represent +=12v
        /// at 12 bit this gives us a resolution 2.4 / 4096 = 5.8597375e-4
        namespace WaveShaper
        {
            static constexpr auto length = 4096U;
            static constexpr auto mask = length - 1;
            static constexpr auto minValue = -1.2f;
            static constexpr auto maxValue = 1.2f;
            static constexpr auto range = maxValue - minValue;
            static constexpr auto interval = range / length;

            using Table = std::array<float, length>;

            inline float process (Table& source, const float in) noexcept
            {
                auto x = in; // rack::simd::clamp(in, minValue, maxValue - interval);
                auto index = (x - minValue) / interval;
                int preIndex = index;
                int postIndex = (preIndex + 1);
                float fraction = index - preIndex;

                preIndex &= mask;
                postIndex &= mask;
                return linearInterpolate (source[preIndex], source[preIndex + 1], fraction);
            }

            inline Table makeTable (std::function<float (const float x)> funct)
            {
                Table table;
                float phase = minValue;
                for (auto& t : table)
                {
                    t = funct (phase);
                    phase += interval;
                }
                return table;
            }

            class WaveShapers
            {
            public:
                struct Definition
                {
                    Definition (Table newTable, std::string newName)
                    {
                        table = newTable;
                        name = newName;
                    }
                    Table table;
                    std::string name;
                };

                size_t size()
                {
                    return shapes.size();
                }

                void addShape (Table table, std::string name)
                {
                    shapes.push_back (Definition (table, name));
                }

                float process (float x, int definitionIndex)
                {
                    return WaveShaper::process (shapes[definitionIndex].table, x);
                }

                float_4 process (float_4 in, int definitionIndex)
                {
                    if (definitionIndex == 0)
                        return in;

                    auto x = rack::simd::clamp (in, minValue, maxValue - interval);
                    return float_4 (process (x[0], definitionIndex),
                                    process (x[1], definitionIndex),
                                    process (x[2], definitionIndex),
                                    process (x[3], definitionIndex));
                }

            private:
                std::vector<Definition> shapes;
            };

            class Nld : public WaveShapers
            {
            public:
                Nld()
                {
                    addShape (linearShape, "Linear");
                    addShape (tanhShape, "tanh(x)");
                    addShape (tanh2Shape, "tanh(2x)");
                    addShape (cosShape, "cos(x)");
                };

                float linearShaper (float x) { return x; }
                float tanhShaper (float x) { return process (x, 1); }
                float tanh2Shaper (float x) { return process (x, 2); }
                float cosShaper (float x) { return process (x, 3); }

                float_4 linearShaper (float_4 x) { return x; }
                float_4 tanhShaper (float_4 x) { return process (x, 1); }
                float_4 tanh2Shaper (float_4 x) { return process (x, 2); }
                float_4 cosShaper (float_4 x) { return process (x, 3); }

                Table linearShape = makeTable ([] (const float x) -> float
                                               { return x; });

                Table tanhShape = makeTable ([] (const float x) -> float
                                             { return tanhf (x); });

                Table tanh2Shape = makeTable ([] (const float x) -> float
                                              { return tanhf (2.0f * x); });

                Table cosShape = makeTable ([] (const float x) -> float
                                            { return cosf (k_pi * x); });
            };

        } // namespace WaveShaper
    } // namespace AudioMath
} // namespace sspo
