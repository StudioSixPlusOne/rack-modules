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
#include "CircularBuffer.h"
#include "UtilityFilters.h"
#include "HardLimiter.h"

using namespace rack;

using float_4 = ::rack::simd::float_4;

namespace sspo
{
    template <typename T>
    class StereoAudioDelay
    {
    public:
        enum class FeedbackMode
        {
            stereo_mode,
            pingPong_mode,
            external_mode,
            cascade_mode
        };

        enum class TimeChangeMode
        {
            instant,
            pitched
        };

        StereoAudioDelay()
        {
            reset();
        }

        void reset()
        {
            for (auto i = 0; i < CHANNEL_COUNT; ++i)
            {
                circularBuffers[i].clear();
                filters[i].clear();
                cutoffs[i] = T (2000.0f);
                maxCutoff = simd::fmin (T (samplerate / 2.0f), T (20000.0f));
                delaySamples[i] = T (2000.0f);
                fbMode = FeedbackMode::stereo_mode;
                tcMode = TimeChangeMode::instant;
                limiters[i].setTimes (0.00f, 0.0025f);
                limiters[i].setSampleRate (samplerate);
                limiters[i].threshold = -0.50f;
            }

            feedback = T (0.5f);
        }

        void setSampleRate (float sr)
        {
            samplerate = sr;
            maxCutoff = simd::fmin (T (sr / 2.0f), T (20000.0f));

            setFilters (cutoffs[0], cutoffs[1]);
            for (auto i = 0; i < CHANNEL_COUNT; ++i)
            {
                circularBuffers[i].reset (samplerate * MAX_DURATION);
                dcBlockers->setSamplerate (sr);
            }
        }

        /// Dj style filter -1 to 1, -1 0 = lpf, 0 1 hpf
        /// deadspot -0.05 to 0.05
        /// \param leftCutoff
        /// \param rightCutoff
        void setFilters (T leftCutoff, T rightCutoff)
        {
            // calculate filter cutoff and modes
            // positive values = voct = leftCutoff * 10.0f -5.0f;
            T positiveFrequency[CHANNEL_COUNT]{};
            T negativeFrequency[CHANNEL_COUNT]{};
            positiveFrequency[0] = dsp::FREQ_C4 * rack::simd::pow (2.0f, leftCutoff * 10.0f - 5.0f);
            positiveFrequency[1] = dsp::FREQ_C4 * rack::simd::pow (2.0f, rightCutoff * 10.0f - 5.0f);

            // negative values 0 = +5, -1 = -1
            // negative values = voct = leftCuoff * -10.0f + 5.0f

            negativeFrequency[0] = dsp::FREQ_C4 * rack::simd::pow (2.0f, leftCutoff * -10.0f + 5.0f);
            negativeFrequency[1] = dsp::FREQ_C4 * rack::simd::pow (2.0f, rightCutoff * -10.0f + 5.0f);
            // note filters per channel can have different modes
            // set all as lp, and hp them merge coefficients

            useLp[0] = leftCutoff < 0.0f;
            useLp[1] = rightCutoff < 0.0f;

            for (auto i = 0; i < CHANNEL_COUNT; ++i)
            {
                hpFilterCoefficients[i].setButterworthHp2 (samplerate, positiveFrequency[i]);
                lpFilterCoefficients[i].setButterworthLp2 (samplerate, negativeFrequency[i]);
                filters[i].coeffs.a0 = simd::ifelse (useLp[i], lpFilterCoefficients[i].coeffs.a0, hpFilterCoefficients[i].coeffs.a0);
                filters[i].coeffs.a1 = simd::ifelse (useLp[i], lpFilterCoefficients[i].coeffs.a1, hpFilterCoefficients[i].coeffs.a1);
                filters[i].coeffs.a2 = simd::ifelse (useLp[i], lpFilterCoefficients[i].coeffs.a2, hpFilterCoefficients[i].coeffs.a2);
                filters[i].coeffs.b1 = simd::ifelse (useLp[i], lpFilterCoefficients[i].coeffs.b1, hpFilterCoefficients[i].coeffs.b1);
                filters[i].coeffs.b2 = simd::ifelse (useLp[i], lpFilterCoefficients[i].coeffs.b2, hpFilterCoefficients[i].coeffs.b2);
                filters[i].coeffs.c0 = simd::ifelse (useLp[i], lpFilterCoefficients[i].coeffs.c0, hpFilterCoefficients[i].coeffs.c0);
                filters[i].coeffs.d0 = simd::ifelse (useLp[i], lpFilterCoefficients[i].coeffs.d0, hpFilterCoefficients[i].coeffs.d0);
            }
        }

        void setDelayTimeSamples (T leftSamples, T rightSamples)
        {
            delaySamples[0] = leftSamples;
            delaySamples[1] = rightSamples;
        }

        void setFeedback (T fb)
        {
            feedback = simd::clamp (fb, 0.0f, 1.25f);
        }

        std::pair<T, T> process (T left, T right)
        {
            std::pair<T, T> result;
            auto in = std::make_pair (left, right);

            for (auto i = 0; i < 2; ++i)
            {
                auto x = dcBlockers[i].process (in.first);
                auto wet = circularBuffers[i].readBuffer (delaySamples[i]);
                auto dry = left + wet * feedback;
                dry = 5.0f * limiters[i].process (dry / 5.0f);
                circularBuffers[i].writeBuffer (dry);
                result.first = wet;
                result = std::make_pair (result.second, result.first);
            }

            return result;
        }

    private:
        static constexpr float MAX_DURATION = 5.0f;
        float samplerate = 44100.0f;
        static constexpr int CHANNEL_COUNT = 2;
        sspo::CircularBuffer<T> circularBuffers[CHANNEL_COUNT];
        sspo::BiQuad<T> filters[CHANNEL_COUNT];
        sspo::BiQuad<T> hpFilterCoefficients[CHANNEL_COUNT];
        sspo::BiQuad<T> lpFilterCoefficients[CHANNEL_COUNT];
        T cutoffs[CHANNEL_COUNT] = { T (2000.0f), T (2000.0f) };
        T useLp[CHANNEL_COUNT];
        T resonance = T (0.707f);
        const T minCutoff = T (20.0F);
        T maxCutoff = T (20000.0f);
        T delaySamples[CHANNEL_COUNT];
        T feedback;
        sspo::DcBlocker<T> dcBlockers[CHANNEL_COUNT];
        sspo::Compressor<T> limiters[CHANNEL_COUNT];

        FeedbackMode fbMode;
        TimeChangeMode tcMode;
    };
} // namespace sspo