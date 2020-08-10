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
    /// single track trigger sequencer, with variable loop points
    /// primary and alt outputs, with independent probability
    template <int MAX_LENGTH>
    struct TriggerSequencer
    {
    private:
        static constexpr int maxLength = MAX_LENGTH;
        int length = 16;
        std::bitset<maxLength> sequence;
        bool active = false;
        bool primaryState;

    private:
        bool altState;
        float primaryProbabilty = 1.0f;
        float altProbability = 1.0f;
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

        int getMaxLength() { return maxLength; }
        int getLength() { return length; }
        void setLength (int len) { length = len; }
        const std::bitset<MAX_LENGTH>& getSequence() { return sequence; }
        bool getActive() { return active; }
        void setActive (bool m) { active = m; }
        void invertActive() { active = ! active; }
        bool isPrimaryState() const
        {
            return primaryState;
        }

        bool isAltState() const
        {
            return altState;
        }

        float getPrimaryProbabilty() const
        {
            return primaryProbabilty;
        }

        void setPrimaryProbabilty (float primaryProbabilty)
        {
            TriggerSequencer::primaryProbabilty = primaryProbabilty;
        }

        float getAltProbability() const
        {
            return altProbability;
        }

        void setAltProbability (float altProbability)
        {
            TriggerSequencer::altProbability = altProbability;
        }

        int getIndex() { return index; }
        bool getStep (int x)
        {
            x = rack::math::clamp (x, 0, maxLength);
            x = rack::math::clamp (x, 0, maxLength);
            return sequence[x];
        }
        bool getCurrentStep() { return getStep (index); }
        bool getCurrentStepPlaying() { return getCurrentStep() && active; }
        void setStep (int step, bool x) { sequence[step] = x; }
        void invertStep (int step) { sequence[step] = ! sequence[step]; }

        bool step (bool trigger)
        {
            if (trigger)
            {
                index++;
                index = index % length;
                auto ret = sequence[index] && active;
                return ret;
            }
            else
                return false;
        }
        void setIndex (int i)
        {
            if (i > -1 && i < MAX_LENGTH)
                index = i;
        }
        void setSequence (int64_t i)
        {
            sequence = i;
        }
    };
} // namespace sspo
