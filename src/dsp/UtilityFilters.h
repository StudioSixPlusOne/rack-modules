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
#include "simd/vector.hpp"
#include "simd/sse_mathfun_extension.h"
#include "simd/sse_mathfun.h"

using float_4 = rack::simd::float_4;

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
            /*auto out = xz1 + coeffs.a0 * in;
            xz1 = coeffs.a1 * in + xz2 - coeffs.b1 * out;
            xz2 = coeffs.a2 * in - coeffs.b2 * out;*/

            // alternative calc with 4 memory buffers give an improved performance
            auto out = coeffs.a0 * in + coeffs.a1 * xz1
                       + coeffs.a2 * xz2 - coeffs.b1 * yz1 - coeffs.b2 * yz2;
            yz2 = yz1;
            yz1 = out;
            xz2 = xz1;
            xz1 = in;
            return out * coeffs.c0 + in * coeffs.d0;
        }

        //-3db at fc 12dB/Octave slope 90 Degree phase at fc
        void setButterworthLp2 (const T sr, const T freq)
        {
            T fc = rack::simd::ifelse (freq < sr * 0.5f, freq, freq * 0.95f);
            auto C = 1.0f / rack::simd::tan ((k_pi * fc) / sr);
            auto a0 = 1.0f / (1.0f + 1.414213562f * C + C * C);

            setCoeffs (a0,
                       2.0f * a0,
                       a0,
                       2.0f * a0 * (1.0f - C * C),
                       a0 * (1.0f - 1.414213562f * C + C * C));
        }
        //-3db at fc 12dB/Octave slope 90 Degree phase at fc
        void setButterworthHp2 (const T sr, const T freq)
        {
            T fc = rack::simd::ifelse (freq < sr * 0.5f, freq, freq * 0.95f);
            auto C = rack::simd::tan ((k_pi * fc) / sr);
            auto a0 = 1.0f / (1.0f + 1.414213562f * C + C * C);

            setCoeffs (a0,
                       -2.0f * a0,
                       a0,
                       2.0f * a0 * (C * C - 1.0f),
                       a0 * (1.0f - 1.414213562f * C + C * C));
        }
        //-6db at fc 12dB/Octave slope 90 Degree phase at fc
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
        //-6db at fc 12dB/Octave slope 90 Degree phase at fc
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

        //flat frequency response -90degree phase at freq
        void setAllPass1stOrder (const T sr, const T freq)
        {
            T fc = rack::simd::ifelse (freq < sr * 0.5, freq, freq * 0.95);
            T alpha = (rack::simd::tan (k_pi * fc / sr) - 1) / (rack::simd::tan (k_pi * fc / sr) + 1);
            setCoeffs (alpha, 1.0f, 0.0f, alpha, 0.0f, 1.0f, 0.0f);
        }

        //coefficents
        struct BiquadCoeffecients
        {
            T a0{};
            T a1{};
            T a2{};
            T b1{};
            T b2{};
            T c0{};
            T d0{};
        } coeffs;

    private:
        //memory
        T xz1{};
        T xz2{};
        T yz1{};
        T yz2{};
    };

    //smid biquad, takes coeffs from four BiQuad<float> objects to run in parallel
    struct MixedBiquadSimd : public BiQuad<float_4>
    {
        void mergeCoeffs (BiQuad<float>& b1, BiQuad<float>& b2, BiQuad<float>& b3, BiQuad<float>& b4)
        {
            setCoeffs ({ b1.coeffs.a0, b2.coeffs.a0, b3.coeffs.a0, b4.coeffs.a0 },
                       { b1.coeffs.a1, b2.coeffs.a1, b3.coeffs.a1, b4.coeffs.a1 },
                       { b1.coeffs.a2, b2.coeffs.a2, b3.coeffs.a2, b4.coeffs.a2 },
                       { b1.coeffs.b1, b2.coeffs.b1, b3.coeffs.b1, b4.coeffs.b1 },
                       { b1.coeffs.b2, b2.coeffs.b2, b3.coeffs.b2, b4.coeffs.b2 });
        }
    };

    template <typename T>
    struct LinkwitzRileyLP2
    {
        BiQuad<T> f1;

        LinkwitzRileyLP2()
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
    struct LinkwitzRileyHP2
    {
        BiQuad<T> f1;

        LinkwitzRileyHP2()
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
            return out;
        }
    };

    template <typename T>
    struct LinkwitzRileyLP4
    {
        BiQuad<T> f1;
        BiQuad<T> f2;

        LinkwitzRileyLP4()
        {
            f1.clear();
            f2.clear();
        }

        void setParameters (const T sr, const T fc)
        {
            f1.setButterworthLp2 (sr, fc);
            f2.setButterworthLp2 (sr, fc);
        }

        T process (const T in)
        {
            T out = f1.process (in);
            out = f2.process (out);
            return out;
        }
    };

    template <typename T>
    struct LinkwitzRileyHP4
    {
        BiQuad<T> f1;
        BiQuad<T> f2;

        LinkwitzRileyHP4()
        {
            f1.clear();
            f2.clear();
        }

        void setParameters (const T sr, const T fc)
        {
            f1.setButterworthHp2 (sr, fc);
            f2.setButterworthHp2 (sr, fc);
        }

        T process (const T in)
        {
            T out = f1.process (in);
            out = f2.process (out);
            return out;
        }
    };

    /// IIR Decimator
    /// oversample, upsample tate
    /// quality, number of sequential filters
    template <int oversample, int quality, typename T>
    struct Decimator
    {
        BiQuad<T> filters[quality];

        Decimator()
        {
            for (auto i = 0; i < quality; ++i)
            {
                // the oversample filter has been set at niquist, to remove unwanted
                // noise in the audio spectrum
                filters[i].setButterworthLp2 (10000.0f, 10000.0f / (1.0f * oversample));
            }
        }

        T process (const T* input)
        {
            T x = 0;
            for (auto i = 0; i < oversample; ++i)
            {
                x = filters[0].process (input[i]);
                for (auto j = 1; j < quality; ++j)
                {
                    x = filters[j].process (x);
                }
            }
            // we simply return the last sample
            return x;
        }
    };
} // namespace sspo