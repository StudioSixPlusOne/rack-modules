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

#include "IComposite.h"
#include "UtilityFilters.h"

#include "simd/functions.hpp"
#include "simd/sse_mathfun.h"
#include "simd/sse_mathfun_extension.h"
#include "simd/vector.hpp"

#include <memory>
#include <vector>

namespace rack
{
    namespace engine
    {
        struct Module;
    }
} // namespace rack
using Module = ::rack::engine::Module;
using namespace rack;
using float_4 = ::rack::simd::float_4;

template <class TBase>
class LaLaDescription : public IComposite
{
public:
    Config getParam (int i) override;
    int getNumParams() override;
};

/**
 * Complete LaLa composite
 *
 * If TBase is WidgetComposite, this class is used as the implementation part of the KSDelay module.
 * If TBase is TestComposite, this class may stand alone for unit tests.
 */

template <class TBase>
class LaLaComp : public TBase
{
public:
    LaLaComp (Module* module) : TBase (module)
    {
    }

    LaLaComp() : TBase()
    {
    }

    virtual ~LaLaComp()
    {
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<LaLaDescription<TBase>>();
    }

    enum ParamIds
    {
        FREQ_PARAM,
        FREQ_CV_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        FREQ_CV_INPUT,
        MAIN_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        HIGH_OUTPUT,
        LOW_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    // member variables
    static constexpr int maxChannels = 16;
    float sampleRate = 1.0f;
    float sampleTime = 1.0f;
    float maxFreq = 0.5f;
    std::vector<sspo::LinkwitzRileyLP2<float_4>> lps;
    std::vector<sspo::LinkwitzRileyHP2<float_4>> hps;

    void setSampleRate (float rate)
    {
        sampleRate = rate;
        sampleTime = 1.0f / rate;
        maxFreq = rate / 2.0f;
    }
    // must be called after setSampleRate
    void init()
    {
        hps.resize (maxChannels / 4);
        lps.resize (maxChannels / 4);
    }

    inline void step() override;
};

template <class TBase>
inline void LaLaComp<TBase>::step()
{
    //TODO step
}

template <class TBase>
int LaLaDescription<TBase>::getNumParams()
{
    return LaLaComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config LaLaDescription<TBase>::getParam (int i)
{
    auto freqBase = static_cast<float> (std::pow (2, 10.0f));
    auto freqMul = static_cast<float> (dsp::FREQ_C4 / std::pow (2, 5.f));
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Code type", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        //TODO
        case LaLaComp<TBase>::FREQ_PARAM:
            ret = { 0.0f, 1.125f, 0.5f, "Frequency", " Hz", freqBase, freqMul };
            break;
        case LaLaComp<TBase>::FREQ_CV_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "Frequency CV", " ", 0.0f, 1.0f, 0.0f };
            break;
        default:
            assert (false);
    }
    return ret;
}