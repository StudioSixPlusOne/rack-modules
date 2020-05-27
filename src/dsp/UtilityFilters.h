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

#include "AudioMath.h"
#include "simd/functions.hpp"

using namespace sspo::AudioMath;

namespace sspo
{
    template <typename T>
    struct BiQuad
    {
        BiQuad()
        {
            clear();
        }

        void setCoeffs (const T a0,
                        const T a1,
                        const T a2,
                        const T b1,
                        const T b2,
                        const T c0 = 1.0,
                        const T d0 = 0.0)
        {
            coeffs.a0 = a0;
            coeffs.a1 = a1;
            coeffs.a2 = a2;
            coeffs.b1 = b1;
            coeffs.b2 = b2;
            coeffs.c0 = c0;
            coeffs.d0 = d0;
        }

        void clear()
        {
            xz1 = {};
            xz2 = {};
            yz1 = {};
            yz1 = {};
        }

        T process (const T in)
        {
            /*            auto out = z1 + coeffs.a0 * in;
            z1 = coeffs.a1 * in + z2 - coeffs.b1 * out;
            z2 = coeffs.a2 * in - coeffs.b2 * out; */

            auto out = coeffs.a0 * in + coeffs.a1 * xz1
                       + coeffs.a2 * xz2 - coeffs.b1 * yz1 - coeffs.b2 * yz2;
            yz2 = yz1;
            yz1 = out;
            xz2 = xz1;
            xz1 = in;
            return out * coeffs.c0 + in * coeffs.d0;
        }

        void setLinkwitzRileyLp2 (const T sr, const T freq)
        {
            T fc = rack::simd::ifelse (freq < sr * 0.5, freq, freq * 0.95);
            T omega_c = k_pi * fc;
            T theta_c = k_pi * fc / sr;

            T k = omega_c / tan (theta_c);
            T denominator = k * k + omega_c * omega_c + 2.0 * k * omega_c;
            T b1Num = -2.0 * k * k + 2.0 * omega_c * omega_c;
            T b2Num = -2.0 * k * omega_c + k * k + omega_c * omega_c;

            setCoeffs (omega_c * omega_c / denominator,
                       2.0 * omega_c * omega_c / denominator,
                       omega_c * omega_c / denominator,
                       b1Num / denominator,
                       b2Num / denominator);
        }

        void setLinkwitzRileyHp2 (const T sr, const T freq)
        {
            T fc = rack::simd::ifelse (freq < sr * 0.5, freq, freq * 0.95);
            T omega_c = k_pi * fc;
            T theta_c = k_pi * fc / sr;

            T k = omega_c / tan (theta_c);
            T denominator = k * k + omega_c * omega_c + 2.0 * k * omega_c;
            T b1Num = -2.0 * k * k + 2.0 * omega_c * omega_c;
            T b2Num = -2.0 * k * omega_c + k * k + omega_c * omega_c;

            setCoeffs (k * k / denominator,
                       -2.0 * k * k / denominator,
                       k * k / denominator,
                       b1Num / denominator,
                       b2Num / denominator);
        }

    private:
        //memory
        T xz1{};
        T xz2{};
        T yz1{};
        T yz2{};

        //coefficents
        struct BiquadCoeffecients
        {
            float a0{};
            float a1{};
            float a2{};
            float b1{};
            float b2{};
            float c0{};
            float d0{};
        } coeffs;
    };

    template <typename T>
    struct LinkwitzRileyLP4
    {
        BiQuad<T> f1;

        LinkwitzRileyLP4()
        {
            f1.clear();
        }

        void setParameters (const T sr, const T fc)
        {
            f1.setLinkwitzRileyLp2 (sr, fc);
        }

        T process (const T in)
        {
            T out = f1.process (in);
            return out;
        }
    };

    template <typename T>
    struct LinkwitzRileyHP4
    {
        BiQuad<T> f1;

        LinkwitzRileyHP4()
        {
            f1.clear();
        }

        void setParameters (const T sr, const T fc)
        {
            f1.setLinkwitzRileyHp2 (sr, fc);
        }

        T process (const T in)
        {
            T out = f1.process (in);
            return -out;
        }
    };

} // namespace sspo