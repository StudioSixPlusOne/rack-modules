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
#include <string>

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
class ThruDescription : public IComposite
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
class ThruComp : public TBase
{
public:
    ThruComp (Module* module) : TBase (module)
    {
    }

    ThruComp() : TBase()
    {
    }

    virtual ~ThruComp()
    {
    }

    /** Implement IComposite
    */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<ThruDescription<TBase>>();
    }

    void setSampleRate (float rate)
    {
    }

    // must be called after setSampleRate
    void init()
    {
        for (auto& d : dirtyLabels)
            d = true;
    }

    void step() override;

    json_t* dataToJson()
    {
        json_t* rootJ = json_object();
        json_object_set_new (rootJ, "runDataFromJson", json_integer (1));
        json_object_set_new (rootJ, "label0", json_stringn (lables[0].c_str(), lables[0].size()));
        json_object_set_new (rootJ, "label1", json_stringn (lables[1].c_str(), lables[1].size()));
        json_object_set_new (rootJ, "label2", json_stringn (lables[2].c_str(), lables[2].size()));
        json_object_set_new (rootJ, "label3", json_stringn (lables[3].c_str(), lables[3].size()));
        json_object_set_new (rootJ, "label4", json_stringn (lables[4].c_str(), lables[4].size()));
        return rootJ;
    }

    void dataFromJson (json_t* rootJ)
    {
        json_t* labelText0 = json_object_get (rootJ, "label0");
        if (labelText0)
        {
            lables[0] = json_string_value (labelText0);
            dirtyLabels[0] = true;
        }

        json_t* labelText1 = json_object_get (rootJ, "label1");
        if (labelText1)
        {
            lables[1] = json_string_value (labelText1);
            dirtyLabels[1] = true;
        }

        json_t* labelText2 = json_object_get (rootJ, "label2");
        if (labelText2)
        {
            lables[2] = json_string_value (labelText2);
            dirtyLabels[2] = true;
        }

        json_t* labelText3 = json_object_get (rootJ, "label3");
        if (labelText3)
        {
            lables[3] = json_string_value (labelText3);
            dirtyLabels[3] = true;
        }

        json_t* labelText4 = json_object_get (rootJ, "label4");
        if (labelText4)
        {
            lables[4] = json_string_value (labelText4);
            dirtyLabels[4] = true;
        }
    }

    enum ParamId
    {
        NUM_PARAMS
    };
    enum InputId
    {
        ONE_INPUT,
        TWO_INPUT,
        THREE_INPUT,
        FOUR_INPUT,
        FIVE_INPUT,
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
    static constexpr int TRACK_COUNT = 5;
    std::array<std::string, TRACK_COUNT> lables;
    std::array<bool, TRACK_COUNT> dirtyLabels;
};

template <class TBase>
inline void ThruComp<TBase>::step()
{
    // loop over inputs
    for (auto i = 0U; i < 5; ++i)
    {
        int channels = TBase::inputs[ONE_INPUT + i].getChannels();
        for (auto c = 0U; c < channels; c += 4)
        {
            auto in = TBase::inputs[ONE_INPUT + i].template getPolyVoltageSimd<float_4> (c);
            TBase::outputs[ONE_OUTPUT + i].setVoltageSimd (in, c);
        }
        TBase::outputs[ONE_OUTPUT + i].setChannels (channels);
    }
}

template <class TBase>
int ThruDescription<TBase>::getNumParams()
{
    return ThruComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config ThruDescription<TBase>::getParam (int i)
{
    auto freqBase = static_cast<float> (std::pow (2, 10.0f));
    auto freqMul = static_cast<float> (dsp::FREQ_C4 / std::pow (2, 5.f));
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Name", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        default:
            assert (false);
    }
    return ret;
}