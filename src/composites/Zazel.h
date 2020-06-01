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
#include "AudioMath.h"
#include "easing.h"
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

template <class TBase>
class ZazelDescription : public IComposite
{
public:
    Config getParam (int i) override;
    int getNumParams() override;
};

/**
 * Complete Zazel composite
 *
 * If TBase is WidgetComposite, this class is used as the implementation part of the KSDelay module.
 * If TBase is TestComposite, this class may stand alone for unit tests.
 */

template <class TBase>
class ZazelComp : public TBase
{
public:
    ZazelComp (Module* module) : TBase (module)
    {
    }

    ZazelComp() : TBase()
    {
    }

    virtual ~ZazelComp()
    {
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<ZazelDescription<TBase>>();
    }

    enum ParamIds
    {
        EASING_ATTENUVERTER_PARAM,
        EASING_PARAM,
        START_ATTENUVERTER_PARAM,
        START_PARAM,
        END_ATTENUVERTER_PARAM,
        END_PARAM,
        DURATION_ATTENUVERTER_PARAM,
        DURATION_PARAM,
        ONESHOT_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        EASING_INPUT,
        START_INPUT,
        END_INPUT,
        DURATION_INPUT,
        STOP_CONT_INPUT,
        CLOCK_INPUT,
        START_CONT_INPUT,
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

    //member variables

    float sampleRate = 1.0f;
    float sampleTime = 1.0f;
    std::vector<Easings::Easing> easings;

    void setSampleRate (float rate)
    {
        sampleRate = rate;
        sampleTime = 1.0f / rate;
    }

    // must be called after setSampleRate
    void init()
    {
        //TODO
        easings = Easings::getEasingVector();
    }

    void step() override;
};

template <class TBase>
inline void ZazelComp<TBase>::step()
{
    //TODO step
}

template <class TBase>
int ZazelDescription<TBase>::getNumParams()
{
    return ZazelComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config ZazelDescription<TBase>::getParam (int i)
{
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Code type", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        case ZazelComp<TBase>::EASING_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "Easing Cv", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::EASING_PARAM:
            ret = { 0.0f, 10.0f, 0.0f, "Easing", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::START_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.f, "Start Cv", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::START_PARAM:
            ret = { -10.0f, 10.0f, 0.f, "Start", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::END_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "End Cv", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::END_PARAM:
            ret = { -10.0f, 10.0f, 0.f, "End", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::DURATION_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.f, "Duration Cv", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::DURATION_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Duration", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::ONESHOT_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Oneshot / cycle ", " ", 0, 1, 0.0f };
            break;
        default:
            assert (false);
    }
    return ret;
}