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
#include <array>

using namespace rack;

using float_4 = ::rack::simd::float_4;

namespace sspo
{
    class Adsr_4
    {
    public:
        enum Stages
        {
            PRE_ATTACK_STAGE,
            ATTACK_STAGE,
            DECAY_STAGE,
            SUSTAIN_STAGE,
            RELEASE_STAGE,
            EOC_STAGE,
            NUM_STAGES
        };

        Adsr_4()
        {
            //set tco used to set curve shapes, these resemble to RC curves on a CM33100
            attackTco = expf (-1.5f);
            decayTco = expf (-4.95);
            releaseTco = decayTco;

            stageScalars[SUSTAIN_STAGE] = float_4 (1.0f);
            stageOffsets[SUSTAIN_STAGE] = float_4::zero();

            stageScalars[EOC_STAGE] = float_4::zero();
            stageOffsets[EOC_STAGE] = float_4::zero();

            reset();
        }

        void setOutputLevelsDEBUG (float_4 l)
        {
            currentLevels = l;
        }

        void reset()
        {
            currentLevels = float_4::zero();
            currentStage = float_4 (EOC_STAGE);
            setParameters (float_4 (0.005f),
                           float_4 (0.2f),
                           float_4 (0.5f),
                           float_4 (0.5f),
                           44100.0f);
            lastGates = float_4 ::zero();
        }

        float_4 step (float_4 gates)
        {
            //check gates for changes.
            //a simple subtraction 0 = no change 1= gate on, -1 gate off
            auto gateChanged = gates - lastGates;

            //update on gate on
            currentStage = simd::ifelse ((gateChanged == 1.0f) & ~resetTriggerOnMask,
                                         ATTACK_STAGE,
                                         currentStage);
            currentStage = simd::ifelse ((gateChanged == 1.0f) & resetTriggerOnMask,
                                         PRE_ATTACK_STAGE,
                                         currentStage);

            //update gate off
            currentStage = simd::ifelse (gateChanged == -1.0f, RELEASE_STAGE, currentStage);

            lastGates = gates; //

            //update current levels

            //TODO make float_4
            //the old school way float 4 as 4 channels
            for (auto i = 0U; i < 4; ++i)
            {
                currentLevels[i] = stageOffsets[currentStage[i]][i] + currentLevels[i] * stageScalars[currentStage[i]][i];
            }

            //check for stage updates
            //todo make float_4
            for (auto i = 0U; i < 4; ++i)
            {
                if (currentStage[i] == PRE_ATTACK_STAGE && currentLevels[i] < 0.0f)
                    currentStage[i]++;
                if (currentStage[i] == ATTACK_STAGE && currentLevels[i] >= 1.0f)
                    currentStage[i]++;
                if (currentStage[i] == DECAY_STAGE && currentLevels[i] <= sustainLevels[i])
                    currentStage[i]++;
                if (currentStage[i] == RELEASE_STAGE && currentLevels[i] <= 0.01f)
                    currentStage[i]++;
            }

            return currentLevels;
        }

        void setParameters (float_4 attackTimeS,
                            float_4 decayTimeS,
                            float_4 sustainLevel,
                            float_4 releaseTimeS,
                            float sampleRate)
        {
            auto attackSamples = attackTimeS * sampleRate;
            stageScalars[ATTACK_STAGE] = simd::exp (
                -simd::log ((1.0 + attackTco) / attackTco) / attackSamples);
            stageOffsets[ATTACK_STAGE] = (1.0f + attackTco) * (1.0f - stageScalars[ATTACK_STAGE]);

            auto decaySamples = decayTimeS * sampleRate;
            stageScalars[DECAY_STAGE] = simd::exp (
                -simd::log ((1.0 + decayTco) / decayTco) / decaySamples);
            stageOffsets[DECAY_STAGE] = (sustainLevel - decayTco) * (1.0f - stageScalars[DECAY_STAGE]);

            auto releaseSamples = releaseTimeS * sampleRate;
            stageScalars[RELEASE_STAGE] = simd::exp (
                -simd::log ((1.0 + releaseTco) / releaseTco) / releaseSamples);
            stageOffsets[RELEASE_STAGE] = releaseTco * (1.0f - stageScalars[RELEASE_STAGE]);

            sustainLevels = sustainLevel;
        }

        const float_4& getCurrentStages()
        {
            return currentStage;
        }

        const float_4& getCurrentLevels()
        {
            return currentLevels;
        }

        void setResetOnTrigger (bool x)
        {
            isResetOnTrigger = x;
            resetTriggerOnMask = isResetOnTrigger ? float_4::mask() : float_4::zero();
        }

    private:
        float_4 lastGates{ 0 };
        float_4 currentLevels{ 0 };
        float_4 currentStage{ EOC_STAGE };
        bool isResetOnTrigger = false;
        std::array<float_4, NUM_STAGES> stageScalars;
        std::array<float_4, NUM_STAGES> stageOffsets;
        float_4 sustainLevels{ 0 };
        float attackTco{ 0 };
        float decayTco{ 0 };
        float releaseTco{ 0 };
        const float_4 preAttackTimeS{ 0.002f, 0.002f, 0.002f, 0.002f }; //2ms pre attack in reset mode
        float_4 resetTriggerOnMask{ 0 };
    };
} // namespace sspo