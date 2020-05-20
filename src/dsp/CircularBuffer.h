/*
 * Copyright (c) 2019 Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>
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
#include <memory>

#include "AudioMath.h"

template <typename T>
class CircularBuffer
{
public:
    CircularBuffer()
    {
        reset (4096);
    }
    CircularBuffer (const unsigned int minBufferSize)
    {
        reset (minBufferSize);
    }

    void reset (const unsigned int minBufferSize)
    {
        writeIndex = 0;
        bufferLength = static_cast<unsigned int> (std::pow (2, std::ceil (std::log (minBufferSize) / std::log (2))));
        wrapBits = bufferLength - 1;
        buffer.reset (new T[bufferLength]);
        clear();
    }

    void clear()
    {
        for (auto i = 0; i < static_cast<int> (bufferLength); ++i)
        {
            buffer[i] = 0;
        }
    }

    inline void writeBuffer (const T newValue) noexcept
    {
        writeIndex++;
        writeIndex &= wrapBits;
        buffer[writeIndex] = std::isnan (newValue) || std::isinf (newValue) ? 0 : newValue;
    }

    inline T readBuffer (const int delaySamples) const noexcept
    {
        return buffer[(writeIndex - delaySamples) & wrapBits];
    }

    inline T readBuffer (const float delaySamples, const bool interpolate = true) const noexcept
    {
        auto y1 = readBuffer (static_cast<int> (delaySamples));
        auto y2 = readBuffer (static_cast<int> (delaySamples) + 1);
        auto fract = delaySamples - static_cast<int> (delaySamples);
        return sspo::AudioMath::linearInterpolate (y1, y2, fract);
    }

    int size()
    {
        return static_cast<int> (bufferLength);
    }

private:
    std::unique_ptr<T[]> buffer{ nullptr };
    unsigned int writeIndex{ 0 };
    unsigned int bufferLength{ 0 };
    unsigned int wrapBits{ 0 };
};
