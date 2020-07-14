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

#include <memory>
#include "IComposite.h"
#include "TriggerSequencer.h"

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
class IversonDescription : public IComposite
{
public:
    Config getParam (int i) override;
    int getNumParams() override;
};

/**
 * Complete Iverson composite
 *
 * If TBase is WidgetComposite, this class is used as the implementation part of the KSDelay module.
 * If TBase is TestComposite, this class may stand alone for unit tests.
 */

template <class TBase>
class IversonComp : public TBase
{
public:
    enum ParamIds
    {
        GRID_1_1_PARAM,
        GRID_2_1_PARAM,
        GRID_3_1_PARAM,
        GRID_4_1_PARAM,
        GRID_5_1_PARAM,
        GRID_6_1_PARAM,
        GRID_7_1_PARAM,
        GRID_8_1_PARAM,
        GRID_9_1_PARAM,
        GRID_10_1_PARAM,
        GRID_11_1_PARAM,
        GRID_12_1_PARAM,
        GRID_13_1_PARAM,
        GRID_14_1_PARAM,
        GRID_15_1_PARAM,
        GRID_16_1_PARAM,
        GRID_1_2_PARAM,
        GRID_2_2_PARAM,
        GRID_3_2_PARAM,
        GRID_4_2_PARAM,
        GRID_5_2_PARAM,
        GRID_6_2_PARAM,
        GRID_7_2_PARAM,
        GRID_8_2_PARAM,
        GRID_9_2_PARAM,
        GRID_10_2_PARAM,
        GRID_11_2_PARAM,
        GRID_12_2_PARAM,
        GRID_13_2_PARAM,
        GRID_14_2_PARAM,
        GRID_15_2_PARAM,
        GRID_16_2_PARAM,
        GRID_1_3_PARAM,
        GRID_2_3_PARAM,
        GRID_3_3_PARAM,
        GRID_4_3_PARAM,
        GRID_5_3_PARAM,
        GRID_6_3_PARAM,
        GRID_7_3_PARAM,
        GRID_8_3_PARAM,
        GRID_9_3_PARAM,
        GRID_10_3_PARAM,
        GRID_11_3_PARAM,
        GRID_12_3_PARAM,
        GRID_13_3_PARAM,
        GRID_14_3_PARAM,
        GRID_15_3_PARAM,
        GRID_16_3_PARAM,
        GRID_1_4_PARAM,
        GRID_2_4_PARAM,
        GRID_3_4_PARAM,
        GRID_4_4_PARAM,
        GRID_5_4_PARAM,
        GRID_6_4_PARAM,
        GRID_7_4_PARAM,
        GRID_8_4_PARAM,
        GRID_9_4_PARAM,
        GRID_10_4_PARAM,
        GRID_11_4_PARAM,
        GRID_12_4_PARAM,
        GRID_13_4_PARAM,
        GRID_14_4_PARAM,
        GRID_15_4_PARAM,
        GRID_16_4_PARAM,
        GRID_1_5_PARAM,
        GRID_2_5_PARAM,
        GRID_3_5_PARAM,
        GRID_4_5_PARAM,
        GRID_5_5_PARAM,
        GRID_6_5_PARAM,
        GRID_7_5_PARAM,
        GRID_8_5_PARAM,
        GRID_9_5_PARAM,
        GRID_10_5_PARAM,
        GRID_11_5_PARAM,
        GRID_12_5_PARAM,
        GRID_13_5_PARAM,
        GRID_14_5_PARAM,
        GRID_15_5_PARAM,
        GRID_16_5_PARAM,
        GRID_1_6_PARAM,
        GRID_2_6_PARAM,
        GRID_3_6_PARAM,
        GRID_4_6_PARAM,
        GRID_5_6_PARAM,
        GRID_6_6_PARAM,
        GRID_7_6_PARAM,
        GRID_8_6_PARAM,
        GRID_9_6_PARAM,
        GRID_10_6_PARAM,
        GRID_11_6_PARAM,
        GRID_12_6_PARAM,
        GRID_13_6_PARAM,
        GRID_14_6_PARAM,
        GRID_15_6_PARAM,
        GRID_16_6_PARAM,
        GRID_1_7_PARAM,
        GRID_2_7_PARAM,
        GRID_3_7_PARAM,
        GRID_4_7_PARAM,
        GRID_5_7_PARAM,
        GRID_6_7_PARAM,
        GRID_7_7_PARAM,
        GRID_8_7_PARAM,
        GRID_9_7_PARAM,
        GRID_10_7_PARAM,
        GRID_11_7_PARAM,
        GRID_12_7_PARAM,
        GRID_13_7_PARAM,
        GRID_14_7_PARAM,
        GRID_15_7_PARAM,
        GRID_16_7_PARAM,
        GRID_1_8_PARAM,
        GRID_2_8_PARAM,
        GRID_3_8_PARAM,
        GRID_4_8_PARAM,
        GRID_5_8_PARAM,
        GRID_6_8_PARAM,
        GRID_7_8_PARAM,
        GRID_8_8_PARAM,
        GRID_9_8_PARAM,
        GRID_10_8_PARAM,
        GRID_11_8_PARAM,
        GRID_12_8_PARAM,
        GRID_13_8_PARAM,
        GRID_14_8_PARAM,
        GRID_15_8_PARAM,
        GRID_16_8_PARAM,
        MUTE_1_PARAM,
        MUTE_2_PARAM,
        MUTE_3_PARAM,
        MUTE_4_PARAM,
        MUTE_5_PARAM,
        MUTE_6_PARAM,
        MUTE_7_PARAM,
        MUTE_8_PARAM,
        LENGTH_1_PARAM,
        LENGTH_2_PARAM,
        LENGTH_3_PARAM,
        LENGTH_4_PARAM,
        LENGTH_5_PARAM,
        LENGTH_6_PARAM,
        LENGTH_7_PARAM,
        LENGTH_8_PARAM,
        PAGE_LEFT_PARAM,
        PAGE_RIGHT_PARAM,
        RUN_PARAM,
        RESET_PARAM,
        CLOCK_PARAM,
        SET_LENGTH_PARAM,
        MIDI_LEARN_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        RUN_INPUT,
        RESET_INPUT,
        CLOCK_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        TRIGGER_1_OUTPUT,
        TRIGGER_2_OUTPUT,
        TRIGGER_3_OUTPUT,
        TRIGGER_4_OUTPUT,
        TRIGGER_5_OUTPUT,
        TRIGGER_6_OUTPUT,
        TRIGGER_7_OUTPUT,
        TRIGGER_8_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        GRID_1_1_LIGHT,
        GRID_2_1_LIGHT,
        GRID_3_1_LIGHT,
        GRID_4_1_LIGHT,
        GRID_5_1_LIGHT,
        GRID_6_1_LIGHT,
        GRID_7_1_LIGHT,
        GRID_8_1_LIGHT,
        GRID_9_1_LIGHT,
        GRID_10_1_LIGHT,
        GRID_11_1_LIGHT,
        GRID_12_1_LIGHT,
        GRID_13_1_LIGHT,
        GRID_14_1_LIGHT,
        GRID_15_1_LIGHT,
        GRID_16_1_LIGHT,
        GRID_1_2_LIGHT,
        GRID_2_2_LIGHT,
        GRID_3_2_LIGHT,
        GRID_4_2_LIGHT,
        GRID_5_2_LIGHT,
        GRID_6_2_LIGHT,
        GRID_7_2_LIGHT,
        GRID_8_2_LIGHT,
        GRID_9_2_LIGHT,
        GRID_10_2_LIGHT,
        GRID_11_2_LIGHT,
        GRID_12_2_LIGHT,
        GRID_13_2_LIGHT,
        GRID_14_2_LIGHT,
        GRID_15_2_LIGHT,
        GRID_16_2_LIGHT,
        GRID_1_3_LIGHT,
        GRID_2_3_LIGHT,
        GRID_3_3_LIGHT,
        GRID_4_3_LIGHT,
        GRID_5_3_LIGHT,
        GRID_6_3_LIGHT,
        GRID_7_3_LIGHT,
        GRID_8_3_LIGHT,
        GRID_9_3_LIGHT,
        GRID_10_3_LIGHT,
        GRID_11_3_LIGHT,
        GRID_12_3_LIGHT,
        GRID_13_3_LIGHT,
        GRID_14_3_LIGHT,
        GRID_15_3_LIGHT,
        GRID_16_3_LIGHT,
        GRID_1_4_LIGHT,
        GRID_2_4_LIGHT,
        GRID_3_4_LIGHT,
        GRID_4_4_LIGHT,
        GRID_5_4_LIGHT,
        GRID_6_4_LIGHT,
        GRID_7_4_LIGHT,
        GRID_8_4_LIGHT,
        GRID_9_4_LIGHT,
        GRID_10_4_LIGHT,
        GRID_11_4_LIGHT,
        GRID_12_4_LIGHT,
        GRID_13_4_LIGHT,
        GRID_14_4_LIGHT,
        GRID_15_4_LIGHT,
        GRID_16_4_LIGHT,
        GRID_1_5_LIGHT,
        GRID_2_5_LIGHT,
        GRID_3_5_LIGHT,
        GRID_4_5_LIGHT,
        GRID_5_5_LIGHT,
        GRID_6_5_LIGHT,
        GRID_7_5_LIGHT,
        GRID_8_5_LIGHT,
        GRID_9_5_LIGHT,
        GRID_10_5_LIGHT,
        GRID_11_5_LIGHT,
        GRID_12_5_LIGHT,
        GRID_13_5_LIGHT,
        GRID_14_5_LIGHT,
        GRID_15_5_LIGHT,
        GRID_16_5_LIGHT,
        GRID_1_6_LIGHT,
        GRID_2_6_LIGHT,
        GRID_3_6_LIGHT,
        GRID_4_6_LIGHT,
        GRID_5_6_LIGHT,
        GRID_6_6_LIGHT,
        GRID_7_6_LIGHT,
        GRID_8_6_LIGHT,
        GRID_9_6_LIGHT,
        GRID_10_6_LIGHT,
        GRID_11_6_LIGHT,
        GRID_12_6_LIGHT,
        GRID_13_6_LIGHT,
        GRID_14_6_LIGHT,
        GRID_15_6_LIGHT,
        GRID_16_6_LIGHT,
        GRID_1_7_LIGHT,
        GRID_2_7_LIGHT,
        GRID_3_7_LIGHT,
        GRID_4_7_LIGHT,
        GRID_5_7_LIGHT,
        GRID_6_7_LIGHT,
        GRID_7_7_LIGHT,
        GRID_8_7_LIGHT,
        GRID_9_7_LIGHT,
        GRID_10_7_LIGHT,
        GRID_11_7_LIGHT,
        GRID_12_7_LIGHT,
        GRID_13_7_LIGHT,
        GRID_14_7_LIGHT,
        GRID_15_7_LIGHT,
        GRID_16_7_LIGHT,
        GRID_1_8_LIGHT,
        GRID_2_8_LIGHT,
        GRID_3_8_LIGHT,
        GRID_4_8_LIGHT,
        GRID_5_8_LIGHT,
        GRID_6_8_LIGHT,
        GRID_7_8_LIGHT,
        GRID_8_8_LIGHT,
        GRID_9_8_LIGHT,
        GRID_10_8_LIGHT,
        GRID_11_8_LIGHT,
        GRID_12_8_LIGHT,
        GRID_13_8_LIGHT,
        GRID_14_8_LIGHT,
        GRID_15_8_LIGHT,
        GRID_16_8_LIGHT,
        MUTE_1_LIGHT,
        MUTE_2_LIGHT,
        MUTE_3_LIGHT,
        MUTE_4_LIGHT,
        MUTE_5_LIGHT,
        MUTE_6_LIGHT,
        MUTE_7_LIGHT,
        MUTE_8_LIGHT,
        PAGE_1_LIGHT,
        PAGE_2_LIGHT,
        RUN_LIGHT,
        RESET_LIGHT,
        CLOCK_LIGHT,
        SET_LENGTH_LIGHT,
        MIDI_LEARN_LIGHT,
        NUM_LIGHTS
    };

    static constexpr int MAX_SEQUENCE_LENGTH = 64;

    float sampleRate = 1.0f;
    float sampleTime = 1.0f;

    IversonComp (Module* module) : TBase (module)
    {
    }

    IversonComp() : TBase()
    {
    }

    virtual ~IversonComp()
    {
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<IversonDescription<TBase>>();
    }

    void setSampleRate (float rate)
    {
        sampleRate = rate;
        sampleTime = 1.0f / rate;
    }

    // must be called after setSampleRate
    void init()
    {
    }

    void step() override;
};

template <class TBase>
inline void IversonComp<TBase>::step()
{
}

template <class TBase>
int IversonDescription<TBase>::getNumParams()
{
    return IversonComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config IversonDescription<TBase>::getParam (int i)
{
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Code type", "unit", 0.0f, 1.0f, 0.0f };
    if (i <= IversonComp<TBase>::GRID_16_8_PARAM)
    {
        ret = { 0.0f, 1.0f, 0.0f, " ", " ", 0, 1, 0.0f };
        return ret;
    }
    if (i <= IversonComp<TBase>::MUTE_8_PARAM)
    {
        ret = { 0.0f, 1.0f, 0.0f, " ", " ", 0, 1, 0.0f };
        return ret;
    }

    if (i <= IversonComp<TBase>::LENGTH_8_PARAM)
    {
        ret = { 0.0f, IversonComp<TBase>::MAX_SEQUENCE_LENGTH, IversonComp<TBase>::MAX_SEQUENCE_LENGTH, " ", " ", 0, 1, 0.0f };
        return ret;
    }

    switch (i)
    {
        case IversonComp<TBase>::PAGE_LEFT_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Page Left", " ", 0, 1, 0.0f };
            break;
        case IversonComp<TBase>::PAGE_RIGHT_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Page Right", " ", 0, 1, 0.0f };
            break;
        case IversonComp<TBase>::RUN_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Run", " ", 0, 1, 0.0f };
            break;
        case IversonComp<TBase>::RESET_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Reset", " ", 0, 1, 0.0f };
            break;
        case IversonComp<TBase>::CLOCK_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Clock", " ", 0, 1, 0.0f };
            break;
        case IversonComp<TBase>::SET_LENGTH_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Lemgth", " ", 0, 1, 0.0f };
            break;
        case IversonComp<TBase>::MIDI_LEARN_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Midi Learn", " ", 0, 1, 0.0f };
            break;
        default:
            assert (false);
    }
    return ret;
}
