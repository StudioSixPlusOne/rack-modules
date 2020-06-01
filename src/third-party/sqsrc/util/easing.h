/*
erms of Use || robertpenner.com
TERMS OF USE - EASING EQUATIONS
Open source under  the http://www.opensource.org/licenses/bsd-license.php" BSD License
Copyright © 2001 Robert Penner
All rights reserved.

Redistribution and use in source and binary forms,
with or without modification, are permitted provided that the following
conditions are met

Redistributions of source code must retain the above copyright  notice,
 this list of conditions and the following disclaimer.
Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.
Neither
the name of the author nor the names of contributors may be used to
endorse or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS
AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <cmath>
#include "AudioMath.h"
#include <vector>
#include <memory>

namespace Easings
{
    // t = current time, zero based
    // b = begin value
    // c = change, end value
    // d = duration, total time
    struct Easing
    {
        virtual ~Easing(){};
        virtual float easeIn (float t, float b, float c, float d) { return 0; }
        virtual float easeOut (float t, float b, float c, float d) { return 0; }
        virtual float easeInOut (float t, float b, float c, float d) { return 0; }
    };

    struct Back : public Easing
    {
        float easeIn (float t, float b, float c, float d) override
        {
            float s = 1.70158f;
            float postFix = t /= d;
            return c * (postFix) *t * ((s + 1) * t - s) + b;
        }
        float easeOut (float t, float b, float c, float d) override
        {
            float s = 1.70158f;
            t = t / d - 1;
            return c * (t * t * ((s + 1) * t + s) + 1) + b;
        }

        float easeInOut (float t, float b, float c, float d) override
        {
            float s = 1.70158f;
            s *= (1.525f);
            if ((t /= d / 2) < 1)
            {
                return c / 2 * (t * t * ((s + 1) * t - s)) + b;
            }
            float postFix = t -= 2;
            return c / 2 * ((postFix) *t * ((s + 1) * t + s) + 2) + b;
        }
    };

    struct Bounce : Easing
    {
        float easeIn (float t, float b, float c, float d) override
        {
            return c - easeOut (d - t, 0, c, d) + b;
        }
        float easeOut (float t, float b, float c, float d) override
        {
            if ((t /= d) < (1 / 2.75f))
            {
                return c * (7.5625f * t * t) + b;
            }
            else if (t < (2 / 2.75f))
            {
                float postFix = t -= (1.5f / 2.75f);
                return c * (7.5625f * (postFix) *t + .75f) + b;
            }
            else if (t < (2.5 / 2.75))
            {
                float postFix = t -= (2.25f / 2.75f);
                return c * (7.5625f * (postFix) *t + .9375f) + b;
            }
            else
            {
                float postFix = t -= (2.625f / 2.75f);
                return c * (7.5625f * (postFix) *t + .984375f) + b;
            }
        }

        float easeInOut (float t, float b, float c, float d) override
        {
            if (t < d / 2)
                return easeIn (t * 2, 0, c, d) * 0.5f + b;
            else
                return easeOut (t * 2 - d, 0, c, d) * 0.5f + c * 0.5f + b;
        }
    };

    struct Circ : Easing
    {
        float easeIn (float t, float b, float c, float d) override
        {
            t = t / d;
            return -c * (std::sqrt (1 - (t) *t) - 1) + b;
        }
        float easeOut (float t, float b, float c, float d) override
        {
            t = t / d - 1;
            return c * std::sqrt (1 - (t) *t) + b;
        }

        float easeInOut (float t, float b, float c, float d) override
        {
            t /= d / 2;
            if ((t) < 1)
                return -c / 2 * (std::sqrt (1 - t * t) - 1) + b;
            t -= 2;
            return c / 2 * (std::sqrt (1 - t * (t)) + 1) + b;
        }
    };

    struct Cubic : Easing
    {
        float easeIn (float t, float b, float c, float d) override
        {
            t /= d;
            return c * (t) *t * t + b;
        }
        float easeOut (float t, float b, float c, float d) override
        {
            t = t / d - 1;
            return c * ((t) *t * t + 1) + b;
        }

        float easeInOut (float t, float b, float c, float d) override
        {
            t /= d / 2;
            if ((t) < 1)
                return c / 2 * t * t * t + b;
            t -= 2;
            return c / 2 * ((t) *t * t + 2) + b;
        }
    };

    struct Elastic : Easing
    {
        float easeIn (float t, float b, float c, float d) override
        {
            if (t == 0)
                return b;
            t /= d;
            if (t == 1)
                return b + c;
            float p = d * .3f;
            float a = c;
            float s = p / 4;
            t -= 1;
            float postFix = a * std::pow (2, 10 * (t)); // this is a fix, again, with post-increment operators
            return -(postFix * std::sin ((t * d - s) * (2.0f * sspo::AudioMath::k_pi) / p)) + b;
        }

        float easeOut (float t, float b, float c, float d) override
        {
            if (t == 0)
                return b;
            if ((t /= d) == 1)
                return b + c;
            float p = d * .3f;
            float a = c;
            float s = p / 4;
            return (a * std::pow (2, -10 * t) * sin ((t * d - s) * (2 * sspo::AudioMath::k_pi) / p) + c + b);
        }

        float easeInOut (float t, float b, float c, float d) override
        {
            if (t == 0)
                return b;
            t /= d / 2;
            if ((t) == 2)
                return b + c;
            float p = d * (.3f * 1.5f);
            float a = c;
            float s = p / 4;

            if (t < 1)
            {
                t -= 1;
                float postFix = a * std::pow (2, 10 * (t)); // postIncrement is evil
                return -.5f * (postFix * std::sin ((t * d - s) * (2.0f * sspo::AudioMath::k_pi) / p)) + b;
            }
            t -= 1;
            float postFix = a * std::pow (2, -10 * (t)); // postIncrement is evil
            return postFix * std::sin ((t * d - s) * (2 * sspo::AudioMath::k_pi) / p) * .5f + c + b;
        }
    };

    struct Expo : Easing
    {
        float easeIn (float t, float b, float c, float d) override
        {
            return (t == 0) ? b : c * std::pow (2, 10 * (t / d - 1)) + b;
        }
        float easeOut (float t, float b, float c, float d) override
        {
            return (t == d) ? b + c : c * (-std::pow (2, -10 * t / d) + 1) + b;
        }

        float easeInOut (float t, float b, float c, float d) override
        {
            if (t == 0)
                return b;
            if (t == d)
                return b + c;
            if ((t /= d / 2) < 1)
                return c / 2 * std::pow (2, 10 * (t - 1)) + b;
            return c / 2 * (-std::pow (2, -10 * --t) + 2) + b;
        }
    };

    struct Linear : Easing
    {
        float easeIn (float t, float b, float c, float d) override
        {
            return c * t / d + b;
        }
        float easeOut (float t, float b, float c, float d) override
        {
            return c * t / d + b;
        }

        float easeInOut (float t, float b, float c, float d) override
        {
            return c * t / d + b;
        }
    };

    struct Quad : Easing
    {
        float easeIn (float t, float b, float c, float d) override
        {
            t /= d;
            return c * (t) *t + b;
        }
        float easeOut (float t, float b, float c, float d) override
        {
            t /= d;
            return -c * (t) * (t - 2) + b;
        }

        float easeInOut (float t, float b, float c, float d) override
        {
            t /= d;
            if ((t / 2) < 1)
                return ((c / 2) * (t * t)) + b;
            return -c / 2 * (((t - 2) * (t)) - 1) + b;
            /*
            originally return -c/2 * (((t-2)*(--t)) - 1) + b;

            I've had to swap (--t)*(t-2) due to diffence in behaviour in 
            pre-increment operators between java and c++, after hours
            of joy
            */
        }
    };

    struct Quart : Easing
    {
        float easeIn (float t, float b, float c, float d) override
        {
            t /= d;
            return c * (t) *t * t * t + b;
        }
        float easeOut (float t, float b, float c, float d) override
        {
            t = t / d - 1;
            return -c * ((t) *t * t * t - 1) + b;
        }

        float easeInOut (float t, float b, float c, float d) override
        {
            t /= d / 2;
            if ((t) < 1)
                return c / 2 * t * t * t * t + b;
            t -= 2;
            return -c / 2 * ((t) *t * t * t - 2) + b;
        }
    };

    struct Quint : Easing
    {
        float easeIn (float t, float b, float c, float d) override
        {
            t /= d;
            return c * (t) *t * t * t * t + b;
        }
        float easeOut (float t, float b, float c, float d) override
        {
            t = t / d - 1;
            return c * ((t) *t * t * t * t + 1) + b;
        }

        float easeInOut (float t, float b, float c, float d) override
        {
            t /= d / 2;
            if ((t) < 1)
                return c / 2 * t * t * t * t * t + b;
            t -= 2;
            return c / 2 * ((t) *t * t * t * t + 2) + b;
        }
    };

    struct Sine : Easing
    {
        float easeIn (float t, float b, float c, float d) override
        {
            return -c * std::cos (t / d * (sspo::AudioMath::k_pi / 2)) + c + b;
        }
        float easeOut (float t, float b, float c, float d) override
        {
            return c * std::sin (t / d * (sspo::AudioMath::k_pi / 2)) + b;
        }

        float easeInOut (float t, float b, float c, float d) override
        {
            return -c / 2 * (cos (sspo::AudioMath::k_pi * t / d) - 1) + b;
        }
    };

    struct EasingFactory
    {
        enum class EasingNames
        {
            back,
            bounce,
            circ,
            cubic,
            elastic,
            expo,
            linear,
            quad,
            quart,
            quint,
            sine
        };

        std::vector<std::shared_ptr<Easings::Easing>> getEasingVector()
        {
            std::vector<std::shared_ptr<Easings::Easing>> ret;
            ret.push_back (std::make_shared<Back>());
            ret.push_back (std::make_shared<Bounce>());
            ret.push_back (std::make_shared<Circ>());
            ret.push_back (std::make_shared<Cubic>());
            ret.push_back (std::make_shared<Elastic>());
            ret.push_back (std::make_shared<Expo>());
            ret.push_back (std::make_shared<Linear>());
            ret.push_back (std::make_shared<Quad>());
            ret.push_back (std::make_shared<Quart>());
            ret.push_back (std::make_shared<Quint>());
            ret.push_back (std::make_shared<Sine>());

            return ret;
        }
    };
} // namespace Easings
