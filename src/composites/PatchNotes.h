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
class PatchNotesDescription : public IComposite
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
class PatchNotesComp : public TBase
{
public:
    PatchNotesComp (Module* module) : TBase (module)
    {
    }

    PatchNotesComp() : TBase()
    {
    }

    virtual ~PatchNotesComp()
    {
    }

    /** Implement IComposite
    */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<PatchNotesDescription<TBase>>();
    }

    void setSampleRate (float rate)
    {
    }

    // must be called after setSampleRate
    void init()
    {
        //resize arrays
        //initialise dsp object

        dirty = true;
        text = "Words a few of them";
    }

    void step() override;

    json_t* dataToJson()
    {
        json_t* rootJ = json_object();
        json_object_set_new (rootJ, "runDataFromJson", json_integer (1));

        json_object_set_new (rootJ, "NoteText", json_stringn (text.c_str(), text.size()));

        return rootJ;
    }

    void dataFromJson (json_t* rootJ)
    {
        json_t* noteText = json_object_get (rootJ, "NoteText");
        if (noteText)
        {
            text = json_string_value (noteText);
            dirty = true;
        }
    }

    enum ParamId
    {
        NUM_PARAMS
    };
    enum InputId
    {
        NUM_INPUTS
    };
    enum OutputId
    {
        NUM_OUTPUTS
    };
    enum LightId
    {
        NUM_LIGHTS
    };

    std::string text;
    bool dirty;
};

template <class TBase>
inline void PatchNotesComp<TBase>::step()
{
    //Nothing to do, just a text widget!
}

template <class TBase>
int PatchNotesDescription<TBase>::getNumParams()
{
    return PatchNotesComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config PatchNotesDescription<TBase>::getParam (int i)
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