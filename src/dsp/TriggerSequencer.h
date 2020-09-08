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
#include <ctime>
#include "asserts.h"
#include "AudioMath.h"
#include "common.hpp"
#include "math.hpp"

using namespace sspo::AudioMath;

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
        std::bitset<MAX_LENGTH> sequence;
        bool active = false;
        bool primaryState;
        bool altState;
        float primaryProbability = 1.0f;
        float altProbability = 1.0f;
        int index = -1;

    public:
        void reset()
        {
            index = -1;
        }

        void resetSequence()
        {
            sequence.reset();
        }

        TriggerSequencer()
        {
            reset();
            defaultGenerator.seed (time (nullptr));
        }

        int getMaxLength() { return maxLength; }
        int getLength() { return length; }
        void setLength (int len) { length = len; }
        const std::bitset<MAX_LENGTH>& getSequence() { return sequence; }
        bool getActive() { return active; }
        void setActive (bool m) { active = m; }
        void invertActive() { active = ! active; }
        /**
         * Primary state is the current main output state of the sequencer
         * @return
         */
        bool getPrimaryState() const
        {
            return primaryState;
        }
        /**
         * Alt state is the current secondary state of the sequencer
         * @return
         */
        bool getAltState() const
        {
            return altState;
        }

        float getPrimaryProbability() const
        {
            return primaryProbability;
        }

        /**
         * probability of a set step being used 0 < x < 2.0
         * when x = 1.0 all and only set steps are activated
         * when x < 1.0 probability of set steps being used in the sequence
         * when x > 1 all set steps are used + probability of non set steps
         *
         * @param primaryProbability
         */
        void setPrimaryProbability (float primaryProbability)
        {
            TriggerSequencer::primaryProbability = rack::math::clamp (primaryProbability, 0.0f, 2.0f);
        }

        float getAltProbability() const
        {
            return altProbability;
        }

        void setAltProbability (float altProbability)
        {
            TriggerSequencer::altProbability = rack::math::clamp (altProbability, 0.0f, 1.0f);
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
                primaryState = false;
                altState = false;
                index++;
                index = index % length;
                auto ret = sequence[index] && active;
                if (sequence[index] && active) //primary
                {
                    if (primaryProbability <= 2.0f)
                    {
                        if (primaryProbability >= rand01())
                            primaryState = true;
                    }
                }
                else if (! sequence[index] && active) //not primary
                {
                    if (primaryProbability > 1.0f)
                    {
                        if (primaryProbability - 1.0 >= rand01())
                            primaryState = true;
                    }
                    if (! primaryState && altProbability > rand01())
                        altState = true;
                }
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

        /// set the pattern using Euclidean Algorithm.
        /// clears the existing pattern
        /// generates patten with no shift

        void setEuclidean (int hits, int length)
        {
            auto currentHits = hits;

            // check to see if valid pattern
            if (length < 1 || hits < 1)
            {
                sequence.reset();
                return;
            }

            length = rack::math::clamp (length, 1, MAX_LENGTH);
            currentHits = rack::math::clamp (currentHits, 1, length);

            //Inital pattern, all hits at the start

            int remaining = length - currentHits;
            for (int i = 0; i < MAX_LENGTH; ++i)
                sequence[i] = (i < currentHits);

            int cNumHits = currentHits;
            int cNumRem = remaining;
            int cHitSpan = 1;
            int cRemSpan = 1;
            int cHitPos = 0;
            int cRemPos = 0;
            int nNumHits = 0;
            int nHitSpan = 1;
            int nRemSpan = 1;

            bool done = false;

            int p = 0; // current position in the bit pattern
            int h = 0; // hit counter
            int r = 0; // remainder counter

            std::bitset<MAX_LENGTH> prevSequence;

            while (cNumRem > 0)
            {
                prevSequence = sequence;
                p = 0;
                h = cNumHits;
                r = cNumRem;
                cHitPos = 0;
                cRemPos = cNumHits * cHitSpan;
                nNumHits = 0;
                done = false;

                while (p < length)
                {
                    if (h > 0)
                    {
                        for (int i = 0; i < cHitSpan; i++)
                            sequence[p++] = prevSequence[cHitPos++];

                        h--;

                        if (! done)
                        {
                            if (r == 1)
                            {
                                nNumHits = cNumRem;
                                nHitSpan += cRemSpan;
                                nRemSpan = cHitSpan;
                                done = true;
                            }
                            else if (h == 0)
                            {
                                nNumHits = cNumHits;
                                nHitSpan += cRemSpan;
                                done = true;
                            }
                        }
                    }

                    if (r > 0)
                    {
                        for (int i = 0; i < cRemSpan; i++)
                            sequence[p++] = prevSequence[cRemPos++];

                        r--;

                        if (! done)
                        {
                            if (h == 0)
                            {
                                nNumHits = cNumHits;
                                nHitSpan = cHitSpan;
                                done = true;
                            }
                            else if (r == 0)
                            {
                                nNumHits = cNumRem;
                                nHitSpan += cRemSpan;
                                nRemSpan = cHitSpan;
                                done = true;
                            }
                        }
                    }
                }

                // reset the number of hit and remainder sequences
                cNumHits = nNumHits;
                cHitSpan = nHitSpan;

                // reset the individual sequence widths
                cRemSpan = nRemSpan;
                cNumRem = (length - (cNumHits * cHitSpan)) / cRemSpan;

                // if either number of sequences is 1, we're done
                if (cNumHits == 1 || cNumRem <= 1)
                    break;
            }
        }
    };
} // namespace sspo
