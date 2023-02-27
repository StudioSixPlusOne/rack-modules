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

#include "IComposite.h"
#include "../dsp/UtilityFilters.h"
#include "HardLimiter.h"
#include <memory>
#include <vector>
#include <array>
#include "simd/functions.hpp"
#include "simd/sse_mathfun.h"
#include "simd/sse_mathfun_extension.h"

#include "jansson.h"

using float_4 = ::rack::simd::float_4;

namespace rack
{
    namespace engine
    {
        struct Module;
    }
} // namespace rack

using Module = ::rack::engine::Module;
using namespace rack;

template <class TBase>
class LalaStereoDescription : public IComposite
{
public:
    Config getParam (int i) override;
    int getNumParams() override;
};

/**
* Complete composite
*
* If TBase is WidgetComposite, this class is used as the implementation part of the  module.
* If TBase is TestComposite, this class may stand alone for unit tests.
*/

template <class TBase>
class LalaStereoComp : public TBase
{
public:
    LalaStereoComp (Module* module) : TBase (module)
    {
    }

    LalaStereoComp() : TBase()
    {
    }

    virtual ~LalaStereoComp()
    {
    }

    /** Implement IComposite
    */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<LalaStereoDescription<TBase>>();
    }

    void setSampleRate (float rate)
    {
        sampleRate = rate;
        sampleTime = 1.0f / rate;
        sr_4 = { sampleRate, sampleRate, sampleRate, sampleRate };
        auto m = rate / 2.0f;
        maxFreq = { m, m, m, m };
    }

    // must be called after setSampleRate
    void init()
    {
        //resize arrays
        //initialise dsp object

        sspo::AudioMath::defaultGenerator.seed (time (NULL));
    }

    void step() override;

    json_t* dataToJson()
    {
        json_t* rootJ = json_object();
        json_object_set_new (rootJ, "runDataFromJson", json_integer (1));
        return rootJ;
    }

    void dataFromJson (json_t* rootJ)
    {
    }

    enum ParamId
    {
        FREQ_PARAM,
        FREQ_CV_PARAM,
        NUM_PARAMS
    };
    enum InputId
    {
        FREQ_CV_INPUT,
        LEFT_INPUT,
        RIGHT_INPUT,
        NUM_INPUTS
    };
    enum OutputId
    {
        LEFT_HIGH_OUTPUT,
        RIGHT_HIGH_OUTPUT,
        LEFT_LOW_OUTPUT,
        RIGHT_LOW_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightId
    {
        NUM_LIGHTS
    };

    static constexpr auto divisorRate = 4U;
    constexpr static float dcInFilterCutoff = 5.5f;
    static constexpr float minFreq = 0.0f;
    float_4 maxFreq{ 20000.0f };
    static constexpr int SIMD_MAX_CHANNELS = 4;
    float sampleRate = 1.0f;
    float sampleTime = 1.0f;
    float_4 sr_4{ sampleRate, sampleRate, sampleRate, sampleRate };
    std::array<sspo::LinkwitzRileyLP4<float_4>, SIMD_MAX_CHANNELS> lpFiltersL;
    std::array<sspo::LinkwitzRileyHP4<float_4>, SIMD_MAX_CHANNELS> hpFiltersL;
    std::array<sspo::LinkwitzRileyLP4<float_4>, SIMD_MAX_CHANNELS> lpFiltersR;
    std::array<sspo::LinkwitzRileyHP4<float_4>, SIMD_MAX_CHANNELS> hpFiltersR;
};

template <class TBase>
inline void LalaStereoComp<TBase>::step()
{
    auto channelsL = TBase::inputs[LEFT_INPUT].getChannels();
    auto channelsR = TBase::inputs[RIGHT_INPUT].getChannels();
    auto freqParam = TBase::params[FREQ_PARAM].getValue();
    freqParam = freqParam * 10.0f - 5.0f;

    for (auto c = 0; c < channelsL; c += 4)
    {
        auto fcv = TBase::inputs[FREQ_CV_INPUT].template getPolyVoltageSimd<float_4> (c);
        fcv *= TBase::params[FREQ_CV_PARAM].getValue();
        fcv += freqParam;
        float_4 freq = dsp::FREQ_C4 * simd::pow (2.0f, fcv);
        freq = simd::clamp (freq, minFreq, maxFreq);
        lpFiltersL[c / 4].setParameters (sr_4, freq);
        hpFiltersL[c / 4].setParameters (sr_4, freq);
        float_4 in = TBase::inputs[LEFT_INPUT].template getPolyVoltageSimd<float_4> (c);
        auto lowOut = lpFiltersL[c / 4].process (in);
        lowOut = sspo::voltageSaturate (lowOut);
        auto highOut = hpFiltersL[c / 4].process (in);
        highOut = sspo::voltageSaturate (highOut);

        //simd'ed out = std::isfinite (out) ? out : 0;
        lowOut = rack::simd::ifelse ((movemask (lowOut == lowOut) != 0xF), float_4 (0.0f), lowOut);
        highOut = rack::simd::ifelse ((movemask (highOut == highOut) != 0xF), float_4 (0.0f), highOut);

        lowOut.store (TBase::outputs[LEFT_LOW_OUTPUT].getVoltages (c));
        highOut.store (TBase::outputs[LEFT_HIGH_OUTPUT].getVoltages (c));
    }

    for (auto c = 0; c < channelsR; c += 4)
    {
        auto fcv = TBase::inputs[FREQ_CV_INPUT].template getPolyVoltageSimd<float_4> (c);
        fcv *= TBase::params[FREQ_CV_PARAM].getValue();
        fcv += freqParam;
        float_4 freq = dsp::FREQ_C4 * simd::pow (2.0f, fcv);
        freq = simd::clamp (freq, minFreq, maxFreq);
        lpFiltersR[c / 4].setParameters (sr_4, freq);
        hpFiltersR[c / 4].setParameters (sr_4, freq);
        float_4 in = TBase::inputs[RIGHT_INPUT].template getPolyVoltageSimd<float_4> (c);
        auto lowOut = lpFiltersR[c / 4].process (in);
        lowOut = sspo::voltageSaturate (lowOut);
        auto highOut = hpFiltersR[c / 4].process (in);
        highOut = sspo::voltageSaturate (highOut);

        lowOut = rack::simd::ifelse ((movemask (lowOut == lowOut) != 0xF), float_4 (0.0f), lowOut);
        highOut = rack::simd::ifelse ((movemask (highOut == highOut) != 0xF), float_4 (0.0f), highOut);

        lowOut.store (TBase::outputs[RIGHT_LOW_OUTPUT].getVoltages (c));
        highOut.store (TBase::outputs[RIGHT_HIGH_OUTPUT].getVoltages (c));
    }

    TBase::outputs[LEFT_LOW_OUTPUT].setChannels (channelsL);
    TBase::outputs[LEFT_HIGH_OUTPUT].setChannels (channelsL);

    TBase::outputs[RIGHT_LOW_OUTPUT].setChannels (channelsR);
    TBase::outputs[RIGHT_HIGH_OUTPUT].setChannels (channelsR);
}

template <class TBase>
int LalaStereoDescription<TBase>::getNumParams()
{
    return LalaStereoComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config LalaStereoDescription<TBase>::getParam (int i)
{
    auto freqBase = static_cast<float> (std::pow (2, 10.0f));
    auto freqMul = static_cast<float> (dsp::FREQ_C4 / std::pow (2, 5.f));
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Name", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        case LalaStereoComp<TBase>::FREQ_PARAM:
            ret = { 0.0f, 1.0f, 0.5f, "FREQ", " ", 0.0f, 1.0f, 0.0f };
            break;

        case LalaStereoComp<TBase>::FREQ_CV_PARAM:
            ret = { 0.0f, 1.0f, 0.5f, "FREQ_CV", " ", 0.0f, 1.0f, 0.0f };
            break;

        default:
            assert (false);
    }
    return ret;
}