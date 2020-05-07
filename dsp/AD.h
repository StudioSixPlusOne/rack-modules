/*
 * Copyright (c) 2019, 2020 Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>
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

using namespace sspo::AudioMath;

namespace sspo
{
    template<typename T>
    struct AttackDecay
    {
        enum class State { off, attack, decay, shutdown };

        AttackDecay() = default;

        inline State getState () const noexcept { return state; }
        inline void setState (State s) noexcept { state = s; }

        inline bool isActive () const noexcept 
        {
            return  (state != State::off);
        }  

        void setSampleRate( T sr)
        {
            sampleRate = sr;
            sampleTime = 1.0 / sr; 
        }

        T tick () noexcept 
        {
            switch (state)
            {
            case State::off:
            {
                break;
            }
            case State::attack:
            {
                lastSample = attackOffset + lastSample * attackCoeff;
                if (lastSample > 1.0f || attackTime <= 0.0f)
                {
                    lastSample = 1.0;
                    state = State::decay;
                    break;
                }
                break;
            }
            case State::decay:
            {
                lastSample = decayOffset + lastSample * decayCoeff;
                if (lastSample <= sustainLevel || decayTime <= 0.0f)
                {
                    lastSample = 0.0f;
                    state = State::off;
                    break;
                }
                break;
            }
            
            

            case State::shutdown:
            {
                state = State::off;
                break;
            }

            }

            return lastSample; 
        }  

        void calcAttackTime ()
        {
            auto samples = static_cast<float>(sampleRate)* attackTime;
            attackCoeff = std::exp (-std::log ((1.0f + attackTCO) / attackTCO) / samples);
            attackOffset = (1.0f + attackTCO) * (1.0f - attackCoeff);
        }

        void calcDecayTime ()
        {
            auto samples = static_cast<float>(sampleRate)* decayTime;
            decayCoeff = std::exp (-std::log ((1.0f + decayTCO) / decayTCO) / samples);
            decayOffset = (sustainLevel - decayTCO) * (1.0f - decayCoeff);
        }

        inline void setParameters (const T attack, const T decay, const T sustain = 0)
        {
            auto updated{ false };
            if (! areSame (attackTime, attack))         { attackTime = attack;        updated = true; }
            if (! areSame (decayTime, decay))           { decayTime = decay;          updated = true; }
            if (! areSame (sustainLevel, sustain))      { sustainLevel = sustain;     updated = true; }

            if (updated)
            {
                calcAttackTime();
                calcDecayTime();
            }
        }

        void noteOn ()
        {
            calcAttackTime();
            calcDecayTime();
            state = State::attack;
        }

        void stop () noexcept
        {
            state = State::off;
        }

        void reset ()
        {
            state = State::off;
            lastSample = 0.0f;
        }

        void shutDown ()
        {
            shutdownInc = lastSample / shutdownTime / static_cast<float>(sampleRate);
            state = State::shutdown;
        }

        protected:

        State state{ State::off };

        //times in ms
        T attackTime{ 0.1f };
        T decayTime{ 0.1f };
        T sustainLevel{ 0.99f };
        T shutdownTime{ 0.01f };

        //internal coefficients offsets and TCO (Time Constant Overshoot) per state
        T attackCoeff{ 0.0f };
        T attackOffset{ 0.0f };
        T attackTCO = std::exp (-1.5f);

        T decayCoeff{ 0.0f };
        T decayOffset{ 0.0f };
        T decayTCO = std::exp (-4.95f);

        T shutdownInc{ 0.01f };

        // current output
        T lastSample{ 0.0f };

        T sampleRate{ 1.0f };
        T sampleTime{ 1.0f };



    };
}