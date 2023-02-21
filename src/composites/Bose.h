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
#include <array>

#include "jansson.h"

#include "SchmittTrigger_4.h"
#include "SampleAndHold.h"

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
class BoseDescription : public IComposite
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
class BoseComp : public TBase
{
public:
    BoseComp (Module* module) : TBase (module)
    {
    }

    BoseComp() : TBase()
    {
    }

    virtual ~BoseComp()
    {
    }

    /** Implement IComposite
    */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<BoseDescription<TBase>>();
    }

    void setSampleRate (float rate)
    {
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
        POLAR_PARAM,
        DROOP_PARAM,
        SCALE_ONE_PARAM,
        SCALE_TWO_PARAM,
        SCALE_THREE_PARAM,
        SCALE_FOUR_PARAM,
        SCALE_FIVE_PARAM,
        NUM_PARAMS
    };
    enum InputId
    {
        TRIGGER_INPUT,
        NUM_INPUTS
    };
    enum OutputId
    {
        ONE_OUTPUT,
        TWO_OUTPUT,
        THREE_OUTPUT,
        FOUR_OUTPUT,
        FIVE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightId
    {
        NUM_LIGHTS
    };


    static constexpr int SIMD_MAX_CHANNELS = 4;
    std::array<sspo::SchmittTrigger_4, SIMD_MAX_CHANNELS> schmitts;
    std::array<std::array<sspo::SampleAndHold<float_4>,  SIMD_MAX_CHANNELS>, 5> shs;
    std::array<float,5> gains;
    float_4 nextRandom;

};

template <class TBase>
inline void BoseComp<TBase>::step()
{
    auto channels = TBase::inputs[TRIGGER_INPUT].getChannels();

    //read parameters as these are constant across all poly channels
    auto useDroop = float_4(TBase::params[DROOP_PARAM].getValue());
    auto offset = float_4(TBase::params[POLAR_PARAM].getValue() * 5.0f);
    for (auto i = 0U; i < 5; ++i)
    {
        gains[i] = TBase::params[SCALE_ONE_PARAM + i].getValue();
    }

    //loop over poly channels, using float_4. so 4 channels
    for (auto c = 0; c < channels; c += 4)
    {
        auto trigger = TBase::inputs[TRIGGER_INPUT].template getPolyVoltageSimd<float_4> (c);
        trigger = schmitts[c/4].process(trigger);

        for (auto i = 0U; i < 5; ++i)
        {

            if (TBase::outputs[ONE_OUTPUT + i].isConnected())
            {
                for (auto j = 0U; j < 4; ++j)
                {
                    if(trigger.s[j] >= 0.9f)
                        nextRandom.s[j] = rand01();
                }
                shs[i][c / 4].setUseDroop (useDroop);
                auto out = shs[i][c / 4].step (nextRandom, trigger);

                //process audio

                TBase::outputs[ONE_OUTPUT + i].setVoltageSimd ((out * 10.0f - offset) * gains[i], c);
            }
        }
    }
    for (auto i = 0U; i < 5; ++i)
    {
        TBase::outputs[ONE_OUTPUT + i].setChannels (channels);
    }
}

template <class TBase>
int BoseDescription<TBase>::getNumParams()
{
    return BoseComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config BoseDescription<TBase>::getParam (int i)
{
    auto freqBase = static_cast<float> (std::pow (2, 10.0f));
    auto freqMul = static_cast<float> (dsp::FREQ_C4 / std::pow (2, 5.f));
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Name", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        case BoseComp<TBase>::POLAR_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "POLAR", " ", 0.0f, 1.0f, 0.0f };
            break;

        case BoseComp<TBase>::DROOP_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "DROOP", " ", 0.0f, 1.0f, 0.0f };
            break;

        case BoseComp<TBase>::SCALE_ONE_PARAM:
            ret = { -1.0f, 1.0f, 1.0f, "SCALE_ONE", " ", 0.0f, 1.0f, 0.0f };
            break;

        case BoseComp<TBase>::SCALE_TWO_PARAM:
            ret = { -1.0f, 1.0f, 1.0f, "SCALE_TWO", " ", 0.0f, 1.0f, 0.0f };
            break;

        case BoseComp<TBase>::SCALE_THREE_PARAM:
            ret = { -1.0f, 1.0f, 1.0f, "SCALE_THREE", " ", 0.0f, 1.0f, 0.0f };
            break;

        case BoseComp<TBase>::SCALE_FOUR_PARAM:
            ret = { -1.0f, 1.0f, 1.0f, "SCALE_FOUR", " ", 0.0f, 1.0f, 0.0f };
            break;

        case BoseComp<TBase>::SCALE_FIVE_PARAM:
            ret = { -1.0f, 1.0f, 1.0f, "SCALE_FIVE", " ", 0.0f, 1.0f, 0.0f };
            break;

        default:
            assert (false);
    }
    return ret;
}