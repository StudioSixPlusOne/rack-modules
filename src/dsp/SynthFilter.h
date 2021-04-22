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

namespace sspo
{
    inline float fastTanh (float x)
    {
        //        return x * (27 + x * x) / (27 + 9 * x * x);
        return std::tanh (x);
    }

    class SynthFilter
    {
    public:
        constexpr static float cutoffMin = 20.0f;
        constexpr static float cutoffMax = 20000.0f;
        constexpr static float cutoffDefault = 20000.0f;
        constexpr static float QDefault = 0.707f;
        constexpr static auto LD_PI = 3.14159265358979323846264338327950288419716939937510L;
        constexpr static auto k_pi = static_cast<float> (LD_PI);
        constexpr static auto k_2pi = k_pi + k_pi;

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

    protected:
        float cutoff{ cutoffDefault };
        float Q{ QDefault };
        float sampleRate{ 1.0f };
        float aux{ 0.0f };
        bool useNPL{ false };
        float saturation{ 1.0 };
        Type type{ Type::LPF2 };
    };

    class OnePoleFilter : public SynthFilter
    {
    public:
        OnePoleFilter()
        {
            type = Type::LPF1;
            reset();
        }

        void setFeedback (float newFeedback)
        {
            feedback = newFeedback;
        }

        void setPreGain (float newPreGain)
        {
            preGain = newPreGain;
        }

        void setBeta (float newBeta)
        {
            beta = newBeta;
        }

        float getFeedbackOut()
        {
            return beta * (z1 + feedback * feedbackIn);
        }

        void setFeedForward (float x)
        {
            feedforward = x;
        }

        float process (const float in)
        {
            if (type != Type::LPF1 && type != Type::HPF1)
                return in;
            auto xn = in;

            xn = xn * preGain + feedback + feedbackOut * getFeedbackOut();
            auto vn = (inputGain * xn - z1) * feedforward;

            auto lp = vn + z1;
            auto hp = xn - lp;
            z1 = vn + lp;

            if (type == Type::LPF1)
                return lp;
            else
                return hp;
        }

        void setParameters (const float newCutoff, const float newQ, const float newAux, const float newSampleRate)
        {
            cutoff = newCutoff;
            Q = newQ;
            aux = newAux;
            sampleRate = newSampleRate;
            calcCoeffs();
        }

        void setType (Type newType)
        {
            type = newType;
        }

        void reset()
        {
            z1 = 0.0f;
            feedback = 0.0f;
        }

    private:
        void calcCoeffs()
        {
            auto wd = k_2pi * cutoff;
            auto T = 1.0f / sampleRate;
            auto wa = (2 / T) * std::tan (wd * T / 2);
            auto g = wa * T / 2;

            feedforward = g / (1.0f + g);
        }

        float feedforward{ 1.0f };
        float beta{ 0.0f };
        float preGain{ 1.0f };
        float feedbackIn{ 0.0f };
        float feedbackOut{ 0.0f };
        float inputGain{ 1.0f };
        float feedback{ 0.0f };
        float z1{ 0.0f };
    };

    class MoogLadderFilter : public SynthFilter
    {
    public:
        static std::vector<Type> types()
        {
            return { Type::LPF2, Type::LPF4, Type::HPF2, Type::HPF4, Type::BPF2, Type::BPF4 };
        }

        MoogLadderFilter()
        {
            lpf1.setType (Type::LPF1);
            lpf2.setType (Type::LPF1);
            lpf3.setType (Type::LPF1);
            lpf4.setType (Type::LPF1);

            type = Type::LPF4;

            reset();
        }

        ~MoogLadderFilter()
        {
        }

        void setParameters (const float newCutoff,
                            const float newQ,
                            const float newSaturation,
                            const float newAux,
                            const float newSampleRate)
        {
            if ((newCutoff == cutoff) && (newQ == Q) && (newSaturation == saturation) && (newSampleRate == sampleRate))
                return;

            cutoff = newCutoff;
            aux = newAux;
            Q = newQ;
            sampleRate = newSampleRate;
            saturation = newSaturation;
            lpf1.setParameters (cutoff, 0, 0, sampleRate);
            lpf2.setParameters (cutoff, 0, 0, sampleRate);
            lpf3.setParameters (cutoff, 0, 0, sampleRate);
            lpf4.setParameters (cutoff, 0, 0, sampleRate);

            K = (4.0f) * (Q - 1.0f) / (10.0f - 1.0f);
            calcCoeffs();
        }

        float process (const float in)
        {
            if (type == Type::BSF2 || type == Type::LPF1 || type == Type ::HPF1)
                return in;
            auto sigma = lpf1.getFeedbackOut()
                         + lpf2.getFeedbackOut()
                         + lpf3.getFeedbackOut()
                         + lpf4.getFeedbackOut();
            auto xn = in;
            xn *= 1.0f + aux * K;
            auto U = (xn - K * sigma) * alpha;

            if (useNPL)
                U = fastTanh (saturation * U);

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

        void setType (Type newType)
        {
            type = newType;
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

            OberheimXpander (const float a, const float b, const float c, const float d, const float e)
            {
                A = a;
                B = b;
                C = c;
                D = d;
                E = e;
            }

            float A{ 0.0f };
            float B{ 0.0f };
            float C{ 0.0f };
            float D{ 0.0f };
            float E{ 0.0f };
        } typeCoeffs;

        void calcCoeffs()
        {
            auto wd = k_2pi * cutoff;
            auto T = 1.0f / static_cast<float> (sampleRate);
            auto wa = (2 / T) * std::tan (wd * T / 2);
            auto g = wa * T / 2;

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

            switch (type)
            {
                case Type::LPF4:
                    typeCoeffs = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
                    break;
                case Type::LPF2:
                    typeCoeffs = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
                    break;
                case Type::BPF4:
                    typeCoeffs = { 0.0f, 0.0f, 4.0f, -8.0f, 4.0f };
                    break;
                case Type::BPF2:
                    typeCoeffs = { 0.0f, 2.0f, -2.0f, 0.0f, 0.0f };
                    break;
                case Type::HPF4:
                    typeCoeffs = { 1.0f, -4.0f, 6.0f, -4.0f, 1.0f };
                    break;
                case Type::HPF2:
                    typeCoeffs = { 1.0f, -2.0f, 1.0f, 0.0f, 0.0f };
                    break;
                default: //lpf4
                    typeCoeffs = { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
                    break;
            }
        }

        OnePoleFilter lpf1;
        OnePoleFilter lpf2;
        OnePoleFilter lpf3;
        OnePoleFilter lpf4;

        float K{ 0.0f };
        float gamma{ 0.0f };
        float alpha{ 1.0f };
    };
} // namespace sspo