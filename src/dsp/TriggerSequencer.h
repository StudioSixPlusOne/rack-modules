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
    public:
        TriggerSequencer();
        void reset();
        void resetSequence();
        int getMaxLength() const;
        int getLength() const;
        void setLength (int len);
        const std::bitset<MAX_LENGTH>& getSequence() const;
        bool getActive() const;
        void setActive (bool m);
        void invertActive();
        /**
         * Primary state is the current main output state of the sequencer
         * @return
         */
        bool getPrimaryState() const;

        /**
         * Alt state is the current secondary state of the sequencer
         * @return
         */
        bool getAltState() const;
        float getPrimaryProbability() const;

        /**
         * probability of a set step being used 0 < x < 2.0
         * when x = 1.0 all and only set steps are activated
         * when x < 1.0 probability of set steps being used in the sequence
         * when x > 1 all set steps are used + probability of non set steps
         *
         * @param primaryProb
         */
        void setPrimaryProbability (float primaryProb);
        float getAltProbability() const;
        void setAltProbability (float altProb);
        int getIndex() const;
        bool getStep (int x) const;
        bool getCurrentStep() const;
        bool getCurrentStepPlaying() const;
        void setStep (int step, bool x);
        void invertStep (int step);
        bool step (bool trigger);
        void setIndex (int i);
        void setSequence (int64_t i);
        void setEuclidean (int hits, int len);

        /**
         *
         * @param rotateRight direction of rotation
         * @param beforeLoop true only rotates before loop, false rotate complete sequence
         */
        void rotate (bool rotateRight, bool beforeLoop);

    private:
        static constexpr int maxLength = MAX_LENGTH;
        int length = 16;
        std::bitset<MAX_LENGTH> sequence;
        bool active = false;
        bool primaryState = false;
        bool altState = false;
        float primaryProbability = 1.0f;
        float altProbability = 1.0f;
        int index = -1;
    };

    template <int MAX_LENGTH>
    bool TriggerSequencer<MAX_LENGTH>::step (bool trigger)
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

    template <int MAX_LENGTH>
    bool TriggerSequencer<MAX_LENGTH>::getStep (int x) const
    {
        x = rack::math::clamp (x, 0, maxLength);
        x = rack::math::clamp (x, 0, maxLength);
        return sequence[x];
    }

    template <int MAX_LENGTH>
    void TriggerSequencer<MAX_LENGTH>::reset()
    {
        index = -1;
    }

    template <int MAX_LENGTH>
    void TriggerSequencer<MAX_LENGTH>::resetSequence()
    {
        sequence.reset();
    }

    template <int MAX_LENGTH>
    TriggerSequencer<MAX_LENGTH>::TriggerSequencer()
    {
        reset();
        defaultGenerator.seed (time (0));
    }

    template <int MAX_LENGTH>
    int TriggerSequencer<MAX_LENGTH>::getMaxLength() const
    {
        return MAX_LENGTH;
    }

    template <int MAX_LENGTH>
    int TriggerSequencer<MAX_LENGTH>::getLength() const
    {
        return length;
    }

    template <int MAX_LENGTH>
    void TriggerSequencer<MAX_LENGTH>::setLength (int len)
    {
        length = len;
    }

    template <int MAX_LENGTH>
    const std::bitset<MAX_LENGTH>& TriggerSequencer<MAX_LENGTH>::getSequence() const
    {
        return sequence;
    }

    template <int MAX_LENGTH>
    bool TriggerSequencer<MAX_LENGTH>::getActive() const
    {
        return active;
    }

    template <int MAX_LENGTH>
    void TriggerSequencer<MAX_LENGTH>::setActive (bool m)
    {
        active = m;
    }

    template <int MAX_LENGTH>
    void TriggerSequencer<MAX_LENGTH>::invertActive()
    {
        active = ! active;
    }

    template <int MAX_LENGTH>
    bool TriggerSequencer<MAX_LENGTH>::getPrimaryState() const
    {
        return primaryState;
    }

    template <int MAX_LENGTH>
    bool TriggerSequencer<MAX_LENGTH>::getAltState() const
    {
        return altState;
    }

    template <int MAX_LENGTH>
    void TriggerSequencer<MAX_LENGTH>::setPrimaryProbability (float primaryProb)
    {
        TriggerSequencer::primaryProbability = rack::math::clamp (primaryProb, 0.0f, 2.0f);
    }

    template <int MAX_LENGTH>
    float TriggerSequencer<MAX_LENGTH>::getAltProbability() const
    {
        return altProbability;
    }

    template <int MAX_LENGTH>
    void TriggerSequencer<MAX_LENGTH>::setAltProbability (float altProb)
    {
        TriggerSequencer::altProbability = rack::math::clamp (altProb, 0.0f, 1.0f);
    }

    template <int MAX_LENGTH>
    void TriggerSequencer<MAX_LENGTH>::setIndex (int i)
    {
        if (i > -1 && i < MAX_LENGTH)
            index = i;
    }

    template <int MAX_LENGTH>
    void TriggerSequencer<MAX_LENGTH>::setSequence (int64_t i)
    {
        sequence = i;
    }

    /// set the pattern using Euclidean Algorithm.
    /// clears the existing pattern
    /// generates patten with no shift
    /// based on code by Count Modular
    template <int MAX_LENGTH>
    void TriggerSequencer<MAX_LENGTH>::setEuclidean (int hits, int len)
    {
        // check to see if valid pattern
        if (len < 1 || hits < 1)
        {
            sequence.reset();
            return;
        }

        if (len > MAX_LENGTH || hits > len)
            return;

        len = rack::math::clamp (len, 1, MAX_LENGTH);
        auto currentHits = rack::math::clamp (hits, 1, len);

        //Initial pattern, all hits at the start

        int remaining = len - currentHits;
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

        int pos = 0; // current position in the bit pattern
        int hitCounter = 0; // hit counter
        int remain = 0; // remainder counter

        std::bitset<MAX_LENGTH> prevSequence;

        while (cNumRem > 0)
        {
            prevSequence = sequence;
            pos = 0;
            hitCounter = cNumHits;
            remain = cNumRem;
            cHitPos = 0;
            cRemPos = cNumHits * cHitSpan;
            nNumHits = 0;
            done = false;

            while (pos < len)
            {
                if (hitCounter > 0)
                {
                    for (int i = 0; i < cHitSpan; i++)
                        sequence[pos++] = prevSequence[cHitPos++];

                    hitCounter--;

                    if (! done)
                    {
                        if (remain == 1)
                        {
                            nNumHits = cNumRem;
                            nHitSpan += cRemSpan;
                            nRemSpan = cHitSpan;
                            done = true;
                        }
                        else if (hitCounter == 0)
                        {
                            nNumHits = cNumHits;
                            nHitSpan += cRemSpan;
                            done = true;
                        }
                    }
                }

                if (remain > 0)
                {
                    for (int i = 0; i < cRemSpan; i++)
                        sequence[pos++] = prevSequence[cRemPos++];

                    remain--;

                    if (! done)
                    {
                        if (hitCounter == 0)
                        {
                            nNumHits = cNumHits;
                            nHitSpan = cHitSpan;
                            done = true;
                        }
                        else if (remain == 0)
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
            cNumRem = (len - (cNumHits * cHitSpan)) / cRemSpan;

            // if either number of sequences is 1, we're done
            if (cNumHits == 1 || cNumRem <= 1)
                break;
        }
    }

    template <int MAX_LENGTH>
    void TriggerSequencer<MAX_LENGTH>::rotate (bool rotateRight, bool beforeLoop)
    {
        auto rotateLength = beforeLoop ? length : MAX_LENGTH;
        if (rotateRight)
        {
            bool overflow = sequence[rotateLength - 1];
            for (auto i = rotateLength - 1; i > 0; --i)
                sequence[i] = sequence[i - 1];
            sequence[0] = overflow;
        }
        else // rotate left
        {
            bool overflow = sequence[0];
            for (auto i = 0; i < rotateLength - 1; ++i)
            {
                sequence[i] = sequence[i + 1];
            }
            sequence[rotateLength - 1] = overflow;
        }
    }

    template <int MAX_LENGTH>
    void TriggerSequencer<MAX_LENGTH>::invertStep (int step)
    {
        sequence[step] = ! sequence[step];
    }

    template <int MAX_LENGTH>
    void TriggerSequencer<MAX_LENGTH>::setStep (int step, bool x)
    {
        sequence[step] = x;
    }

    template <int MAX_LENGTH>
    bool TriggerSequencer<MAX_LENGTH>::getCurrentStepPlaying() const
    {
        return getCurrentStep() && active;
    }

    template <int MAX_LENGTH>
    bool TriggerSequencer<MAX_LENGTH>::getCurrentStep() const
    {
        return getStep (index);
    }

    template <int MAX_LENGTH>
    int TriggerSequencer<MAX_LENGTH>::getIndex() const
    {
        return index;
    }

    template <int MAX_LENGTH>
    float TriggerSequencer<MAX_LENGTH>::getPrimaryProbability() const
    {
        return primaryProbability;
    }
} // namespace sspo
