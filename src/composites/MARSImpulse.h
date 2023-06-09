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
#include "SchmittTrigger_4.h"
#include <memory>
#include <vector>
#include <array>

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
class MARSImpulseDescription : public IComposite
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
class MARSImpulseComp : public TBase
{
public:
    MARSImpulseComp (Module* module) : TBase (module)
    {
    }

    MARSImpulseComp() : TBase()
    {
    }

    virtual ~MARSImpulseComp()
    {
    }

    /** Implement IComposite
    */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<MARSImpulseDescription<TBase>>();
    }

    void setSampleRate (float rate)
    {
        // set samplerate on any dsp objects
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
        IMPULSE_LEVEL_PARAM,
        TRIGGER_PARAM,
        NUM_PARAMS
    };
    enum InputId
    {
        TRIGGER_INPUT,
        NUM_INPUTS
    };
    enum OutputId
    {
        MAIN_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightId
    {
        NUM_LIGHTS
    };

    sspo::SchmittTrigger<float> schmittTrigger;
};

template <class TBase>
inline void MARSImpulseComp<TBase>::step()
{
    //read parameters as these are constant across all poly channels

    auto level = TBase::params[IMPULSE_LEVEL_PARAM].getValue();

    auto trigger = TBase::inputs[TRIGGER_INPUT].getVoltage();
    trigger += TBase::params[TRIGGER_PARAM].getValue();

    //process audio

    auto out = schmittTrigger.process (trigger) * level;

    TBase::outputs[MAIN_OUTPUT].setVoltage (out);

    TBase::outputs[MAIN_OUTPUT].setChannels (1);
}

template <class TBase>
int MARSImpulseDescription<TBase>::getNumParams()
{
    return MARSImpulseComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config MARSImpulseDescription<TBase>::getParam (int i)
{
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Name", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        case MARSImpulseComp<TBase>::IMPULSE_LEVEL_PARAM:
            ret = { -10.0f, 10.0f, 5.0f, "IMPULSE_LEVEL", " ", 0.0f, 1.0f, 0.0f };
            break;

        case MARSImpulseComp<TBase>::TRIGGER_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "TRIGGER", " ", 0.0f, 1.0f, 0.0f };
            break;

        default:
            assert (false);
    }
    return ret;
}