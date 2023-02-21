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
    class SampleAndHold
    {
    public:
        const T& step (T in, T trigger);
        void setUseDroop (T droop);

    protected:
        T currentValue{ 0 };
        T beta{ 1.0f };
        T offset{ 0 };
    };

    template <typename T>
    void SampleAndHold<T>::setUseDroop (T droop)
    {
        beta = simd::ifelse(droop == 1.0f, 0.9999999f, 1.0f);
        offset = simd::ifelse(droop == 1.0f, 0.000000f, 0.0f);
    }

    template <typename T>
    const T& SampleAndHold<T>::step (T in, T trigger)
    {
        currentValue = simd::ifelse(trigger >= 1.0f, in, currentValue) * beta - offset;
        return currentValue;
    }


} // namespace sspo