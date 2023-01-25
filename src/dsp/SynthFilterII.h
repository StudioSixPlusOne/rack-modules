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

//Filters for use in a synth, based on descriptions and diagrams from
//Designing software synthesizer plug-ins in c++  Will Pirkle

#pragma once

#include <cmath>
#include <vector>
#include <functional>
#include "AudioMath.h"
#include "UtilityFilters.h"
//#include "simd/vector.hpp"
#include "simd/functions.hpp"
#include "simd/sse_mathfun.h"
#include "simd/sse_mathfun_extension.h"

namespace sspo
{
    namespace synthFilterII
    {
        template <typename T>
        class SynthFilter
        {
        public:
            constexpr static float cutoffMin{ 20.0f };
            constexpr static float cutoffMax = { 20000.0f };
            constexpr static float cutoffDefault{ 20000.0f };
            constexpr static float QDefault{ 0.707f };
            constexpr static auto LD_PI = 3.14159265358979323846264338327950288419716939937510L;
            constexpr static float k_pi = float (LD_PI);

            enum class Type
            {
                LPF1,
                HPF1,
                LPF2,
                HPF2,
                BPF2,
                BSF2,
                LPF4,
                HPF4,
                BPF4
            };

            SynthFilter() = default;

            void setUseNonLinearProcessing (const bool x)
            {
                useNPL = x;
            }

            void setUseOversample (const bool x)
            {
                useOverSample = x;
            }

        protected:
            T cutoff{ T (cutoffDefault) };
            T Q{ T (QDefault) };
            float sampleRate{ 1.0f };
            T aux{ T (0.0f) };
            bool useNPL{ false };
            bool useOverSample{ false };
            T saturation{ T (1.0) };
            Type type{ Type::LPF2 };
        };

        template <typename T>
        class OnePoleFilter : public SynthFilter<T>
        {
        public:
            OnePoleFilter()
            {
                SynthFilter<T>::type = SynthFilter<T>::Type::LPF1;
                reset();
            }

            void setFeedback (T newFeedback)
            {
                feedback = newFeedback;
            }

            void setPreGain (T newPreGain)
            {
                preGain = newPreGain;
            }

            void setBeta (T newBeta)
            {
                beta = newBeta;
            }

            T getFeedbackOut()
            {
                return beta * (z1 + feedback * feedbackIn);
            }

            void setFeedForward (T x)
            {
                feedforward = x;
            }

            T process (const T in)
            {
                if (SynthFilter<T>::type != SynthFilter<T>::Type::LPF1 && SynthFilter<T>::type != SynthFilter<T>::Type::HPF1)
                    return in;
                auto xn = in;

                xn = xn * preGain + feedback + feedbackOut * getFeedbackOut();
                auto vn = (inputGain * xn - z1) * feedforward;

                auto lp = vn + z1;
                auto hp = xn - lp;
                z1 = vn + lp;

                if (SynthFilter<T>::type == SynthFilter<T>::Type::LPF1)
                    return lp;
                else
                    return hp;
            }

            void setParameters (const T newCutoff, const T newQ, const T newAux, const float newSampleRate)
            {
                SynthFilter<T>::cutoff = newCutoff;
                SynthFilter<T>::Q = newQ;
                SynthFilter<T>::aux = newAux;
                SynthFilter<T>::sampleRate = newSampleRate;
                calcCoeffs();
            }

            void setType (typename SynthFilter<T>::Type newType)
            {
                SynthFilter<T>::type = newType;
            }

            void reset()
            {
                z1 = T (0.0f);
                feedback = T (0.0f);
            }

        private:
            void calcCoeffs()
            {
                auto wd = AudioMath::k_2pi * SynthFilter<T>::cutoff;
                auto T1 = 1.0f / SynthFilter<T>::sampleRate;
                auto wa = (2 / T1) * rack::simd::tan (wd * T1 / 2);
                auto g = wa * T1 / 2;

                feedforward = g / (1.0f + g);
            }

            T feedforward{ 1.0f };
            T beta{ 0.0f };
            T preGain{ 1.0f };
            T feedbackIn{ 0.0f };
            T feedbackOut{ 0.0f };
            T inputGain{ 1.0f };
            T feedback{ 0.0f };
            T z1{ 0.0f };
        };

        template <typename T>
        class LadderFilter : public SynthFilter<T>
        {
        public:
            static std::vector<typename SynthFilter<T>::Type> types()
            {
                return { SynthFilter<T>::Type::LPF2,
                         SynthFilter<T>::Type::LPF4,
                         SynthFilter<T>::Type::HPF2,
                         SynthFilter<T>::Type::HPF4,
                         SynthFilter<T>::Type::BPF2,
                         SynthFilter<T>::Type::BPF4 };
            }

            LadderFilter()
            {
                lpf1.setType (SynthFilter<T>::Type::LPF1);
                lpf2.setType (SynthFilter<T>::Type::LPF1);
                lpf3.setType (SynthFilter<T>::Type::LPF1);
                lpf4.setType (SynthFilter<T>::Type::LPF1);

                SynthFilter<T>::type = SynthFilter<T>::Type::LPF4;

                reset();
            }

            ~LadderFilter() = default;

            void setParameters (const T newCutoff,
                                const T newQ,
                                const T newSaturation,
                                const T newAux,
                                const float newSampleRate)
            {
                //            if ((newCutoff == SynthFilter<T>::cutoff) && (newQ == SynthFilter<T>::Q)
                //                && (newSaturation == SynthFilter<T>::saturation) && (newSampleRate == SynthFilter<T>::sampleRate))
                //                return;

                SynthFilter<T>::cutoff = newCutoff;
                SynthFilter<T>::aux = newAux;
                SynthFilter<T>::Q = newQ;
                SynthFilter<T>::sampleRate = newSampleRate;
                SynthFilter<T>::saturation = newSaturation;
                lpf1.setParameters (SynthFilter<T>::cutoff, 0, 0, SynthFilter<T>::sampleRate);
                lpf2.setParameters (SynthFilter<T>::cutoff, 0, 0, SynthFilter<T>::sampleRate);
                lpf3.setParameters (SynthFilter<T>::cutoff, 0, 0, SynthFilter<T>::sampleRate);
                lpf4.setParameters (SynthFilter<T>::cutoff, 0, 0, SynthFilter<T>::sampleRate);

                K = (4.0f) * (SynthFilter<T>::Q - 1.0f) / (10.0f - 1.0f);
                calcCoeffs();
            }

            //this must be defined in the module, normally as a lambda
            std::function<T (T in, T drive)> nonLinearProcess;

            T process (const T in)
            {
                if (SynthFilter<T>::type == SynthFilter<T>::Type::BSF2
                    || SynthFilter<T>::type == SynthFilter<T>::Type::LPF1
                    || SynthFilter<T>::type == SynthFilter<T>::Type ::HPF1)
                    return in;
                auto sigma = lpf1.getFeedbackOut()
                             + lpf2.getFeedbackOut()
                             + lpf3.getFeedbackOut()
                             + lpf4.getFeedbackOut();
                auto xn = in;
                xn *= 1.0f + SynthFilter<T>::aux * K;
                auto U = (xn - K * sigma) * alpha;

                if (SynthFilter<T>::useNPL)
                {
                    if (SynthFilter<T>::useOverSample)
                    {
                        upsampler.process (U, oversampleBuffer);
                        for (auto i = 0; i < oversampleRate; ++i)
                            oversampleBuffer[i] = nonLinearProcess (U, SynthFilter<T>::saturation);
                        U = decimator.process (oversampleBuffer);
                    }
                    else
                    {
                        U = nonLinearProcess (U, SynthFilter<T>::saturation);
                    }
                }

                auto f1 = lpf1.process (U);
                auto f2 = lpf2.process (f1);
                auto f3 = lpf3.process (f2);
                auto f4 = lpf4.process (f3);

                return typeCoeffs.A * U
                       + typeCoeffs.B * f1
                       + typeCoeffs.C * f2
                       + typeCoeffs.D * f3
                       + typeCoeffs.E * f4;
            }

            void setType (typename SynthFilter<T>::Type newType)
            {
                SynthFilter<T>::type = newType;
                calcCoeffs();
            }

            void reset()
            {
                lpf1.reset();
                lpf2.reset();
                lpf3.reset();
                lpf4.reset();
            }

        private:
            struct OberheimXpander
            {
                OberheimXpander()
                {
                }

                OberheimXpander (const T a, const T b, const T c, const T d, const T e)
                {
                    A = a;
                    B = b;
                    C = c;
                    D = d;
                    E = e;
                }

                T A{ 0.0f };
                T B{ 0.0f };
                T C{ 0.0f };
                T D{ 0.0f };
                T E{ 0.0f };
            } typeCoeffs;

            void calcCoeffs()
            {
                auto wd = AudioMath::k_2pi * SynthFilter<T>::cutoff;
                auto T1 = 1.0f / static_cast<float> (SynthFilter<T>::sampleRate);
                auto wa = (2 / T1) * tan (wd * T1 / 2);
                auto g = wa * T1 / 2;

                auto G = g / (1.0f + g);

                lpf1.setFeedForward (G);
                lpf2.setFeedForward (G);
                lpf3.setFeedForward (G);
                lpf4.setFeedForward (G);

                lpf1.setBeta (G * G * G / (1.0f + g));
                lpf2.setBeta (G * G / (1.0f + g));
                lpf3.setBeta (G / (1.0f + g));
                lpf4.setBeta (1.0f / (1.0f + g));

                gamma = G * G * G * G;
                alpha = 1.0f / (1.0f + K * gamma);

                switch (SynthFilter<T>::type)
                {
                    case SynthFilter<T>::Type::LPF4:
                        typeCoeffs = { T (0.0f), T (0.0f), T (0.0f), T (0.0f), T (1.0f) };
                        break;
                    case SynthFilter<T>::Type::LPF2:
                        typeCoeffs = { T (0.0f), T (0.0f), T (1.0f), T (0.0f), T (0.0f) };
                        break;
                    case SynthFilter<T>::Type::BPF4:
                        typeCoeffs = { T (0.0f), T (0.0f), T (4.0f), T (-8.0f), T (4.0f) };
                        break;
                    case SynthFilter<T>::Type::BPF2:
                        typeCoeffs = { T (0.0f), T (2.0f), T (-2.0f), T (0.0f), T (0.0f) };
                        break;
                    case SynthFilter<T>::Type::HPF4:
                        typeCoeffs = { T (1.0f), T (-4.0f), T (6.0f), T (-4.0f), T (1.0f) };
                        break;
                    case SynthFilter<T>::Type::HPF2:
                        typeCoeffs = { T (1.0f), T (-2.0f), T (1.0f), T (0.0f), T (0.0f) };
                        break;
                    default: //lpf4
                        typeCoeffs = { T (0.0f), T (0.0f), T (0.0f), T (0.0f), T (1.0f) };
                        break;
                }
            }

            OnePoleFilter<T> lpf1{};
            OnePoleFilter<T> lpf2{};
            OnePoleFilter<T> lpf3{};
            OnePoleFilter<T> lpf4{};

            T K{ 0.0f };
            T gamma{ 0.0f };
            T alpha{ 1.0f };

            static constexpr int oversampleRate = 4;
            sspo::Upsampler<oversampleRate, 1, T> upsampler;
            sspo::Decimator<oversampleRate, 1, T> decimator;
            T oversampleBuffer[oversampleRate];
        };
    } // namespace synthFilterII
} // namespace sspo