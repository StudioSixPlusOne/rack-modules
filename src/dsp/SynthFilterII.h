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
#include "WaveShaper.h"
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
                auto xn = in;

                xn = xn * preGain + feedback + feedbackOut * getFeedbackOut();
                auto vn = (inputGain * xn - z1) * feedforward;

                auto lp = vn + z1;
                z1 = vn + lp;

                return lp;
            }

            void setParameters (const T newCutoff, const T newQ, const T newAux, const float newSampleRate)
            {
                SynthFilter<T>::cutoff = newCutoff;
                SynthFilter<T>::Q = newQ;
                SynthFilter<T>::aux = newAux;
                SynthFilter<T>::sampleRate = newSampleRate;
                calcCoeffs();
            }

            void setSampleRate (float sr)
            {
                SynthFilter<T>::sampleRate = sr;
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
            LadderFilter()
            {
                reset();
            }

            ~LadderFilter() = default;

            void setSampleRate (float sr)
            {
                SynthFilter<T>::sampleRate = sr;
                lpf1.setSampleRate (sr);
                lpf2.setSampleRate (sr);
                lpf3.setSampleRate (sr);
                lpf4.setSampleRate (sr);
                //                calcCoeffs();
            }

            void setFcQSat (const T newCutoff,
                            const T newQ,
                            const T newSaturation)
            {
                SynthFilter<T>::cutoff = newCutoff;
                SynthFilter<T>::Q = newQ;
                SynthFilter<T>::saturation = newSaturation;

                lpf1.setParameters (SynthFilter<T>::cutoff, 0, 0, SynthFilter<T>::sampleRate);
                lpf2.setParameters (SynthFilter<T>::cutoff, 0, 0, SynthFilter<T>::sampleRate);
                lpf3.setParameters (SynthFilter<T>::cutoff, 0, 0, SynthFilter<T>::sampleRate);
                lpf4.setParameters (SynthFilter<T>::cutoff, 0, 0, SynthFilter<T>::sampleRate);

                calcCoeffs();
            }

            void setNldTypes (int inNld, int resNld, int s1, int s2, int s3, int s4)
            {
                inNldType = inNld;
                resNldType = resNld;
                s1Stype = s1;
                s2Stype = s2;
                s3Stype = s3;
                s4Stype = s4;
            }

            void setFcQSat (const T newCutoff1,
                            const T newCutoff2,
                            const T newCutoff3,
                            const T newCutoff4,
                            const T newQ,
                            const T newSaturation)
            {
                SynthFilter<T>::cutoff = newCutoff1;
                SynthFilter<T>::Q = newQ;
                SynthFilter<T>::saturation = newSaturation;

                lpf1.setParameters (newCutoff1, 0, 0, SynthFilter<T>::sampleRate);
                lpf2.setParameters (newCutoff2, 0, 0, SynthFilter<T>::sampleRate);
                lpf3.setParameters (newCutoff3, 0, 0, SynthFilter<T>::sampleRate);
                lpf4.setParameters (newCutoff4, 0, 0, SynthFilter<T>::sampleRate);

                calcCoeffs();
            }

            void setAux (T newAux)
            {
                SynthFilter<T>::aux = newAux;
                calcCoeffs();
            }

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

                calcCoeffs();
            }

            //this must be defined in the module, normally as a lambda
            std::function<T (T in, T drive)> nonLinearProcess;

            T process (const T in)
            {
                auto sigma = lpf1.getFeedbackOut()
                             + lpf2.getFeedbackOut()
                             + lpf3.getFeedbackOut()
                             + lpf4.getFeedbackOut();
                sigma *= K;
                sigma = linearInterpolate (sigma, z1, T (feedbackPathCrossfade));
                WaveShaper::nld.process (sigma, sigma, resNldType);
                T xn;
                WaveShaper::nld.process (xn, in, inNldType);
                xn *= 1.0f + SynthFilter<T>::aux * K;
                auto U = (xn - sigma) * alpha;
                WaveShaper::nld.process (U, U, s1Stype);
                auto f1 = lpf1.process (U);
                WaveShaper::nld.process (f1, f1, s2Stype);
                auto f2 = lpf2.process (f1);
                WaveShaper::nld.process (f2, f2, s3Stype);
                auto f3 = lpf3.process (f2);
                WaveShaper::nld.process (f3, f3, s4Stype);
                auto f4 = lpf4.process (f3);

                z1 = typeCoeffs.A * U
                     + typeCoeffs.B * f1
                     + typeCoeffs.C * f2
                     + typeCoeffs.D * f3
                     + typeCoeffs.E * f4;

                return z1;
            }

            void setCoeffs (T a, T b, T c, T d, T e)
            {
                typeCoeffs.A = a;
                typeCoeffs.B = b;
                typeCoeffs.C = c;
                typeCoeffs.D = d;
                typeCoeffs.E = e;
            }

            void setFeedbackPath (const float path)
            {
                feedbackPathCrossfade = path;
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

                //                lpf1.setFeedForward (G);
                //                lpf2.setFeedForward (G);
                //                lpf3.setFeedForward (G);
                //                lpf4.setFeedForward (G);
                //
                lpf1.setBeta (G * G * G / (1.0f + g));
                lpf2.setBeta (G * G / (1.0f + g));
                lpf3.setBeta (G / (1.0f + g));
                lpf4.setBeta (1.0f / (1.0f + g));

                gamma = G * G * G * G;
                alpha = 1.0f / (1.0f + K * gamma);

                K = (4.0f) * (SynthFilter<T>::Q - 1.0f) / (10.0f - 1.0f);
            }

            OnePoleFilter<T> lpf1{};
            OnePoleFilter<T> lpf2{};
            OnePoleFilter<T> lpf3{};
            OnePoleFilter<T> lpf4{};

            int resNldType{ 0 };
            int inNldType{ 0 };
            int s1Stype{ 0 };
            int s2Stype{ 0 };
            int s3Stype{ 0 };
            int s4Stype{ 0 };

            T K{ 0.0f };
            T gamma{ 0.0f };
            T alpha{ 1.0f };
            T z1{ 0.0f };

            float feedbackPathCrossfade{ 0 };
        };
    } // namespace synthFilterII
} // namespace sspo