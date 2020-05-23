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
#include <memory>
#include <assert.h>

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
class EvaDescription : public IComposite
{
public:
    Config getParam (int i) override;
    int getNumParams() override;
};

/**
 * Complete Maccomo composite
 *
 * If TBase is WidgetComposite, this class is used as the implementation part of the KSDelay module.
 * If TBase is TestComposite, this class may stand alone for unit tests.
 */

template <class TBase>
class EvaComp : public TBase
{
public:
    EvaComp (Module* module) : TBase (module)
    {
    }

    EvaComp() : TBase()
    {
    }

    virtual ~EvaComp()
    {
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<EvaDescription<TBase>>();
    }

    enum ParamIds
    {
        ATTENUVERTER_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        ONE_INPUT,
        TWO_INPUT,
        THREE_INPUT,
        FOUR_INPUT,
        ATTENUATION_CV,
        NUM_INPUTS
    };
    enum OutputIds
    {
        MAIN_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    constexpr static int inputCount = 4;

    int maxInputChannels()
    {
        auto ret = 0;
        for (auto i = 0; i < NUM_INPUTS; ++i)
        {
            if (TBase::inputs[i].getChannels() > ret)
                ret = TBase::inputs[i].getChannels();
        }
        return ret;
    }

    void step() override;
};

template <class TBase>
inline void EvaComp<TBase>::step()
{
    auto channels = maxInputChannels();

    for (auto c = 0; c < channels; c += 4)
    {
        float_4 out{};
        for (auto i = 0; i < inputCount; ++i)
            out += TBase::inputs[i].template getPolyVoltageSimd<float_4> (c);

        auto attenuation = TBase::params[ATTENUVERTER_PARAM].getValue()
                           + (TBase::inputs[ATTENUATION_CV].getPolyVoltage (c) / 5.0f);
        attenuation = clamp (attenuation, -1.0f, 1.0f);
        out *= attenuation;

        //set output
        out.store (TBase::outputs[MAIN_OUTPUT].getVoltages (c));
    }

    TBase::outputs[MAIN_OUTPUT].setChannels (channels);
}

template <class TBase>
int EvaDescription<TBase>::getNumParams()
{
    return EvaComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config EvaDescription<TBase>::getParam (int i)
{
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Code type", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        //TODO
        case EvaComp<TBase>::ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 1.0f, "Attenuverter", " ", 0, 1, 0.0f };
            break;
        default:
            assert (false);
    }
    return ret;
}