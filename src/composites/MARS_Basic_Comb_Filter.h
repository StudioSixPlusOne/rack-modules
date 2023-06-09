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
#include "CircularBuffer.h"
#include "Reverb.h"
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
class MARS_Basic_Comb_FilterDescription : public IComposite
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
class MARS_Basic_Comb_FilterComp : public TBase
{
public:
    MARS_Basic_Comb_FilterComp (Module* module) : TBase (module)
    {
    }

    MARS_Basic_Comb_FilterComp() : TBase()
    {
    }

    virtual ~MARS_Basic_Comb_FilterComp()
    {
    }

    /** Implement IComposite
    */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<MARS_Basic_Comb_FilterDescription<TBase>>();
    }

    void setSampleRate (float rate)
    {
        sampleRate = rate;
        sampleTime = 1.0f / rate;

        // set samplerate on any dsp objects

        for (auto& cf : combFilters)
        {
            cf.setSampleRate (rate);
        }
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
        DELAYTIME_PARAM,
        FEEDBACK_PARAM,
        NUM_PARAMS
    };
    enum InputId
    {
        MAIN_INPUT,
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

    static constexpr auto maxDelayTimemSec = 1000.0f;
    float sampleRate = 1.0f;
    float sampleTime = 1.0f;
    std::array<sspo::Reverb::CombFilter<float>, 16> combFilters;
};

template <class TBase>
inline void MARS_Basic_Comb_FilterComp<TBase>::step()
{
    //read parameters as these are constant

    auto delayTime = TBase::params[DELAYTIME_PARAM].getValue();
    auto feedback = TBase::params[FEEDBACK_PARAM].getValue();

    auto channels = TBase::inputs[MAIN_INPUT].getChannels();

    for (auto c = 0; c < channels; ++c)
    {
        combFilters[c].setParameters (feedback, delayTime);

        //Monophonic

        auto in = TBase::inputs[MAIN_INPUT].getVoltage();
        auto y = combFilters[c].step (in);
        y = std::isfinite (y) ? y : 0;

        TBase::outputs[MAIN_OUTPUT].setVoltage (y, c);
    }

    TBase::outputs[MAIN_OUTPUT].setChannels (channels);
}

template <class TBase>
int MARS_Basic_Comb_FilterDescription<TBase>::getNumParams()
{
    return MARS_Basic_Comb_FilterComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config MARS_Basic_Comb_FilterDescription<TBase>::getParam (int i)
{
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Name", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        case MARS_Basic_Comb_FilterComp<TBase>::DELAYTIME_PARAM:
            ret = { 0.0f, MARS_Basic_Comb_FilterComp<TBase>::maxDelayTimemSec, 100.0f, "Delay Time", " mSec", 0.0f, 1.0f, 0.0f };
            break;

        case MARS_Basic_Comb_FilterComp<TBase>::FEEDBACK_PARAM:
            ret = { 0.0f, 1.0f, 0.5f, "Feedback", " %", 0.0f, 1.0f, 0.0f };
            break;

        default:
            assert (false);
    }
    return ret;
}