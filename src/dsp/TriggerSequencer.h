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

#include <vector>
#include <bitset>
#include "asserts.h"
#include "AudioMath.h"
#include "common.hpp"
#include "math.hpp"

namespace sspo
{
    template <int MAX_LENGTH>
    struct TriggerSequencer
    {
    private:
        static constexpr int maxLength = MAX_LENGTH;
        int length = 16;
        std::bitset<maxLength> sequence{ false };
        bool mute = false;
        int index = -1;

    public:
        void reset()
        {
            index = -1;
        }

        TriggerSequencer()
        {
            reset();
        }

        int getLength() { return maxLength; }
        void setLength (int len) { length = len; }
        const std::bitset<MAX_LENGTH>& getSequence() { return sequence; }
        bool getMute() { return mute; }
        void setMute (bool m) { mute = m; }
        void invertMute() { mute = ! mute; }
        int getIndex() { return index; }
        bool getStep (int x)
        {
            x = rack::math::clamp (x, 0, maxLength);
            return sequence[x];
        }
        bool getCurrentStep() { return getStep (index); }
        void setStep (int step, bool x) { sequence[step] = x; }

        bool step (bool trigger)
        {
            if (trigger)
            {
                index++;
                index = index % length;
                auto ret = sequence[index] && ! mute;
                return ret;
            }
            else
                return false;
        }
    };
} // namespace sspo

/// single track trigger sequencer, with variable loop points
template <int MAX_LENGTH>
#include "AudioMath.h"
