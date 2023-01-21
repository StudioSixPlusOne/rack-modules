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

#include "simd/functions.hpp"
#include "simd/sse_mathfun.h"
#include "simd/sse_mathfun_extension.h"
//#include "simd/vector.hpp"

using float_4 = ::rack::simd::float_4;

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
            inline T process (Table<T>& source, const T x) noexcept
            {
                assert (source.table.size() != 0 && "Lookup table empty");
                assert (source.minX != source.maxX && "Lookup table min equal max");
                assert (source.interval != 0 && "Lokkup interval 0");

                //assert(x >= source.minX && "Lookuptable index too low");
                //assert(x <= source.maxX && "Lookuptable index too greate");

                auto index = static_cast<int> ((x / source.interval) - source.minX / source.interval);
                // commented out due to incressing time of lookup by 60%
                //                index = rack::simd::clamp (index, 0, static_cast<int> (source.table.size() - 2));
                T fraction = ((x / source.interval) - source.minX / source.interval) - index;
                return linearInterpolate (static_cast<T> (source.table[index]), static_cast<T> (source.table[index + 1]), fraction);
            }

            // original take using slow vector access to read from table
            // perf.exe reports 23% of 1% usage
            //            template <typename T>
            //            inline float_4 process (Table<T>& source,  float_4 x)
            //            {
            //                assert (source.table.size() != 0 && "Lookup table empty");
            //                assert (source.minX != source.maxX && "Lookup table min equal max");
            //                assert (source.interval != 0 && "Lookup interval 0");
            //
            //                //x = rack::simd::clamp(x, float_4(source.minX), float_4(source.maxX));
            //                //assert(x >= source.minX && "Lookuptable index too low");
            //                //assert(x <= source.maxX && "Lookuptable index too greate");
            //
            //                float_4 index = rack::simd::floor((x / source.interval) - source.minX / source.interval);
            //                //index = simd::clamp(index, float_4(0), float_4(source.table.size() - 1.0f));
            //
            //                float_4 fraction = ((x / source.interval) - source.minX / source.interval) - index;
            //                float_4 lower = float_4 (source.table[index[0]],
            //                                         source.table[index[1]],
            //                                         source.table[index[2]],
            //                                         source.table[index[3]]);
            //                float_4 upper = float_4 (source.table[index[0] + 1],
            //                                         source.table[index[1] + 1],
            //                                         source.table[index[2] + 1],
            //                                         source.table[index[3] + 1]);
            //                return  linearInterpolate (lower, upper, fraction);
            //
            //            }

            // second attempt without creating new float_4;
            // member float_4 are used, this makes Lookup tables no reentrant
            // a separate instance is require for each module instance
            // dont use lookup.xxx() for simd
            // perf.exe reports 23% of 1% usage

            template <typename T>
            inline float_4 process (Table<T>& source, float_4 x)
            {
                //				const float_4 lower = float_4(0.0f, 0.45f, 0.33f, 0.77f);
                //				const float_4 upper {0.1f, 0.55f, 0.45f, 0.9f};

                assert (source.table.size() != 0 && "Lookup table empty");
                assert (source.minX != source.maxX && "Lookup table min equal max");
                assert (source.interval != 0 && "Lookup interval 0");

                //x = rack::simd::clamp(x, float_4(source.minX), float_4(source.maxX));
                //assert(x >= source.minX && "Lookuptable index too low");
                //assert(x <= source.maxX && "Lookuptable index too greate");

                auto indexFull = (x / source.interval) - source.minX / source.interval;
                auto index = rack::simd::floor (indexFull);
                auto fraction = (indexFull) - index;

                auto indexPlus1 = index + 1;

                return {
                    linearInterpolate (source.table[index[0]], source.table[indexPlus1[0]], fraction[0]),
                    linearInterpolate (source.table[index[1]], source.table[indexPlus1[1]], fraction[1]),
                    linearInterpolate (source.table[index[2]], source.table[indexPlus1[2]], fraction[2]),
                    linearInterpolate (source.table[index[3]], source.table[indexPlus1[3]], fraction[3])
                };


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
                    sineTable = sspo::AudioMath::LookupTable::makeTable<float> (-4 * k_2pi - 0.1f, 4 * k_2pi + 0.1f, 0.001f, [] (const float x) -> float
                                                                                { return std::sin (x); });
                    pow2Table = LookupTable::makeTable<float> (-10.1f, 10.1f, 0.001f, [] (const float x) -> float
                                                               { return std::pow (2.0f, x); });
                    pow10Table = LookupTable::makeTable<float> (-10.1f, 10.1f, 0.001f, [] (const float x) -> float
                                                                { return std::pow (10.0f, x); });
                    log10Table = LookupTable::makeTable<float> (0.00001f, 10.1f, 0.001f, [] (const float x) -> float
                                                                { return std::log10 (x); });
                    unisonSpreadTable = LookupTable::makeTable<float> (0.0f, 1.1f, 0.01f, [] (const float x) -> float
                                                                       { return unisonSpreadScalar (x); });

                    hulaSineTable = sspo::AudioMath::LookupTable::makeTable<float> (-4 * k_2pi - 0.1f, 4 * k_2pi + 0.1f, 0.001f, [] (const float x) -> float
                                                                                    { return std::sin (x) + (rand01() - 0.5f) * 1e-4f; });
                }

                sspo::AudioMath::LookupTable::Table<float> sineTable;
                sspo::AudioMath::LookupTable::Table<float> pow2Table;
                sspo::AudioMath::LookupTable::Table<float> pow10Table;
                sspo::AudioMath::LookupTable::Table<float> log10Table;
                sspo::AudioMath::LookupTable::Table<float> unisonSpreadTable;
                sspo::AudioMath::LookupTable::Table<float> hulaSineTable;

                float sin (const float x) { return sspo::AudioMath::LookupTable::process (sineTable, x); }
                float pow2 (const float x) { return sspo::AudioMath::LookupTable::process (pow2Table, x); }
                float_4 pow2 (const float_4 x) { return sspo::AudioMath::LookupTable::process (pow2Table, x); }
                float pow10 (const float x) { return sspo::AudioMath::LookupTable::process (pow10Table, x); }
                float log10 (const float x) { return sspo::AudioMath::LookupTable::process (log10Table, x); }
                float unisonSpread (const float x) { return sspo::AudioMath::LookupTable::process (unisonSpreadTable, x); }
                float hulaSin (const float x) { return sspo::AudioMath::LookupTable::process (hulaSineTable, x); }
                float_4 hulaSin4 (const float_4 x) { return sspo::AudioMath::LookupTable::process (hulaSineTable, x); }
            };

        } // namespace LookupTable
    } // namespace AudioMath
} // namespace sspo

static sspo::AudioMath::LookupTable::Lookup lookup;