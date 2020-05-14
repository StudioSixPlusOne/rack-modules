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
#include <cassert>
#include <functional>
#include <sstream>
#include <vector>

#include "AudioMath.h"

namespace sspo
{
    namespace AudioMath
    {
        namespace LookupTable
        {
            template <typename T>
            struct Table
            {
                T minX = 0;
                T maxX = 0;
                T interval = 0;
                std::vector<T> table;
            };

            template <typename T>
            inline T process (Table<T>& source, const T x)
            {
                assert(source.table.size() != 0 && "Lookup table empty");
                assert(source.minX != source.maxX && "Lookup table min equal max");
                assert(source.interval != 0 && "Lokkup interval 0");

                //assert(x >= source.minX && "Lookuptable index too low");
                //assert(x <= source.maxX && "Lookuptable index too greate"); 

                auto index = static_cast<int> ((x / source.interval) - source.minX / source.interval);
                // commented out due to incressing time of lookup by 60%
                index = rack::math::clamp (index, 0, static_cast<int> (source.table.size() - 2));
                T fraction = ((x / source.interval) - source.minX / source.interval) - index;
                T ret = linearInterpolate (static_cast<T> (source.table[index]), static_cast<T> (source.table[index + 1]), fraction);
                return ret;
            }

            template <typename T>
            inline Table<T> makeTable (const T min, const T max, const T interval, std::function<T (const T x)> funct)
            {
                assert (min < max);
                assert (interval > 0);

                Table<T> ret;
                ret.minX = min;
                ret.maxX = max;
                ret.interval = interval;

                for (T i = min; i < max; i += interval)
                {
                    ret.table.push_back (funct (i));
                }

                return ret;
            }

            inline std::string makeHeader (Table<float>& data, const std::string name = "NONAMEGIVEN")
            {
                static constexpr int maxValPerLine = 8;
                std::stringstream ss;
                ss.precision (14);
                ss << std::stringstream::fixed;

                ss << "#pragma once\n";
                ss << "#include <vector>\n\n\n";

                ss << "struct " << name << " : sspo::AudioMath::LookupTable::Table<float>\n";
                ss << "{\n";
                ss << "\t float mixX = " << data.minX << "f;\n";
                ss << "\t float maxX = " << data.maxX << "f;\n";
                ss << "\t float interval = " << data.interval << "f;\n\n";
                ss << "\t std::vector<float> table = {\n\t\t";
                auto i = 0;
                for (auto v : data.table)
                {
                    ss << v << "f, ";
                    i++;
                    if (i == maxValPerLine)
                    {
                        i = 0;
                        ss << "\n\t\t";
                    }
                }
                ss << "\n\t\t };\n";

                ss << "};\n";

                /*                 T minX = 0;
                T maxX = 0;
                T interval = 0;
                std::vector<T> table;  */

                return ss.str();
            }

            inline float unisonSpreadScalar (const float x)
            {
                return (10028.7312891634 * std::pow (x, 11)) - (50818.8652045924 * std::pow (x, 10)) + (111363.4808729368 * std::pow (x, 9)) - (138150.6761080548 * std::pow (x, 8)) + (106649.6679158292 * std::pow (x, 7)) - (53046.9642751875 * std::pow (x, 6)) + (17019.9518580080 * std::pow (x, 5)) - (3425.0836591318 * std::pow (x, 4)) + (404.2703938388 * std::pow (x, 3)) - (24.1878824391 * std::pow (x, 2)) + (0.6717417634 * x) + 0.0030115596;
            }

            struct Lookup
            {
                Lookup()
                {
                    sineTable = sspo::AudioMath::LookupTable::makeTable<float> (-k_2pi - 0.1f, k_2pi + 0.1f, 0.001f, [] (const float x) -> float { return std::sin (x); });
                    pow2Table = LookupTable::makeTable<float> (-10.1f, 10.1f, 0.001f, [] (const float x) -> float { return std::pow (2.0f, x); });
                    pow10Table = LookupTable::makeTable<float> (-10.1f, 10.1f, 0.001f, [] (const float x) -> float { return std::pow (10.0f, x); });
                    log10Table = LookupTable::makeTable<float> (0.00001f, 10.1f, 0.001f, [] (const float x) -> float { return std::log10 (x); });
                    unisonSpreadTable = LookupTable::makeTable<float> (0.0f, 1.1f, 0.01f, [] (const float x) -> float { return unisonSpreadScalar (x); });
                }

                sspo::AudioMath::LookupTable::Table<float> sineTable;
                sspo::AudioMath::LookupTable::Table<float> pow2Table;
                sspo::AudioMath::LookupTable::Table<float> pow10Table;
                sspo::AudioMath::LookupTable::Table<float> log10Table;
                sspo::AudioMath::LookupTable::Table<float> unisonSpreadTable;

                float sin (const float x) { return sspo::AudioMath::LookupTable::process (sineTable, x); }
                float pow2 (const float x) { return sspo::AudioMath::LookupTable::process (pow2Table, x); }
                float pow10 (const float x) { return sspo::AudioMath::LookupTable::process (pow10Table, x); }
                float log10 (const float x) { return sspo::AudioMath::LookupTable::process (log10Table, x); }
                float unisonSpread (const float x) { return sspo::AudioMath::LookupTable::process (unisonSpreadTable, x); }
            };

        } // namespace LookupTable
    } // namespace AudioMath
} // namespace sspo

static sspo::AudioMath::LookupTable::Lookup lookup;