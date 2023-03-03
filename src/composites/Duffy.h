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
#include <memory>
#include <vector>

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
class DuffyDescription : public IComposite
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
class DuffyComp : public TBase
{
public:
    DuffyComp (Module* module) : TBase (module)
    {
    }

    DuffyComp() : TBase()
    {
    }

    virtual ~DuffyComp()
    {
    }

    /** Implement IComposite
    */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<DuffyDescription<TBase>>();
    }

    void setSampleRate (float rate)
    {
    }

    // must be called after setSampleRate
    void init()
    {
        //resize arrays
        //initialise dsp object
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
        SHIFT_PARAM,
        NUM_PARAMS
    };
    enum InputId
    {
        DOWN_INPUT,
        UP_INPUT,
        ONE_INPUT,
        TWO_INPUT,
        THREE_INPUT,
        RESET_INPUT,
        NUM_INPUTS
    };
    enum OutputId
    {
        ONE_OUTPUT,
        TWO_OUTPUT,
        THREE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightId
    {
        NUM_LIGHTS
    };

    static constexpr int SIMD_MAX_CHANNELS = 4;
    static constexpr auto semitoneVoltage = 1.0 / 12.0f;
    int currentTranspose = 0;

    dsp::SchmittTrigger upTrigger;
    dsp::SchmittTrigger downTrigger;
    dsp::SchmittTrigger resetTrigger;
};

template <class TBase>
inline void DuffyComp<TBase>::step()
{
    //check reset
    if (resetTrigger.process (TBase::inputs[RESET_INPUT].getVoltage()))
    {
        currentTranspose = 0;
    }

    //check if up down pressed
    if (downTrigger.process (TBase::inputs[DOWN_INPUT].getVoltage()))
    {
        currentTranspose -= int (TBase::params[SHIFT_PARAM].getValue());
        while (currentTranspose > 12)
            currentTranspose -= 12;

        while (currentTranspose < 0)
            currentTranspose += 12;
    }

    if (upTrigger.process (TBase::inputs[UP_INPUT].getVoltage()))
    {
        currentTranspose += int (TBase::params[SHIFT_PARAM].getValue());
        while (currentTranspose > 11)
            currentTranspose -= 12;

        while (currentTranspose < 0)
            currentTranspose += 12;
    }

    auto voctOffset = currentTranspose * semitoneVoltage;

    constexpr int numPorts = 3;
    for (auto i = 0U; i < numPorts; ++i)
    {
        //loop over poly channels, using float_4. so 4 channels
        auto channels = TBase::inputs[ONE_INPUT + i].getChannels();
        for (auto c = 0; c < channels; c += 4)
        {
            auto in = TBase::inputs[ONE_INPUT + i].template getPolyVoltageSimd<float_4> (c);
            auto out = in + voctOffset;

            TBase::outputs[ONE_OUTPUT + i]
                .setVoltageSimd (out, c);
        }
        TBase::outputs[ONE_OUTPUT + i].setChannels (channels);
    }
}

template <class TBase>
int DuffyDescription<TBase>::getNumParams()
{
    return DuffyComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config DuffyDescription<TBase>::getParam (int i)
{
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Name", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        case DuffyComp<TBase>::SHIFT_PARAM:
            ret = { -12.0f, 12.0f, 7.0f, "Shift", " ", 0.0f, 1.0f, 0.0f };
            break;

        default:
            assert (false);
    }
    return ret;
}