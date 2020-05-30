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

#include <algorithm>
#include <cstdlib>
#include <vector>
#include <memory>
#include <time.h>

using namespace sspo::AudioMath;

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
class PolyShiftRegisterDescription : public IComposite
{
public:
    Config getParam (int i) override;
    int getNumParams() override;
};

/**
 * Complete KSDelay composite
 *
 * If TBase is WidgetComposite, this class is used as the implementation part of the KSDelay module.
 * If TBase is TestComposite, this class may stand alone for unit tests.
 */
template <class TBase>
class PolyShiftRegisterComp : public TBase
{
public:
    enum ParamIds
    {
        CHANNELS_PARAM,
        TRIGGER_PROB_PARAM,
        SHUFFLE_PROB_PARAM,
        ACCENT_A_PROB_PARAM,
        ACCENT_A_OFFSET_PARAM,
        ACCENT_B_PROB_PARAM,
        ACCENT_B_OFFSET_PARAM,
        ACCENT_RNG_PROB_PARAM,
        ACCENT_RNG_OFFSET_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        CHANNELS_INPUT,
        TRIGGER_PROB_INPUT,
        SHUFFLE_PROB_INPUT,
        ACCENT_A_PROB_INPUT,
        ACCENT_A_OFFSET_INPUT,
        ACCENT_B_PROB_INPUT,
        ACCENT_B_OFFSET_INPUT,
        ACCENT_RNG_PROB_INPUT,
        ACCENT_RNG_MAX_INPUT,
        MAIN_INPUT,
        RESET_INPUT,
        TRIGGER_INPUT,
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

    static constexpr int maxChannels = 16;
    //16 historc values kept per channel to allow for individual shuffles
    std::vector<std::vector<float>> channelData;
    dsp::SchmittTrigger clockTrigger;
    dsp::SchmittTrigger resetTrigger;
    int currentChannels = 1;

    // random number generated per trigger used for accents
    std::vector<float> triggerRng;
    std::vector<float> accentAOffsets;
    std::vector<float> accentBOffsets;
    std::vector<float> accentRngOffsets;

    PolyShiftRegisterComp (Module* module) : TBase (module)
    {
    }

    PolyShiftRegisterComp() : TBase()
    {
    }

    virtual ~PolyShiftRegisterComp()
    {
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<PolyShiftRegisterDescription<TBase>>();
    }

    // must be called after setSampleRate
    void init()
    {
        defaultGenerator.seed (time (nullptr));

        channelData.resize (maxChannels);
        for (auto& cd : channelData)
        {
            cd.resize (maxChannels);
            for (auto& c : cd)
                c = -0.0f;
        }

        triggerRng.resize (maxChannels);
        for (auto& t : triggerRng)
            t = 0.0f;

        accentAOffsets.resize (maxChannels);
        for (auto& a : accentAOffsets)
            a = 0.0f;

        accentBOffsets.resize (maxChannels);
        for (auto& a : accentBOffsets)
            a = 0.0f;

        accentRngOffsets.resize (maxChannels);
        for (auto& a : accentRngOffsets)
            a = 0.0f;
    }

    void shift (float in, int bufferChannel = 0)
    {
        for (auto i = maxChannels; i > 0; --i)
            channelData[bufferChannel][i] = channelData[bufferChannel][i - 1];
        channelData[bufferChannel][0] = in;
    }

    void triggerRngGenerator()
    {
        for (auto i = 0; i < currentChannels; ++i)
            triggerRng[i] = rand01();
    }

    void generateAccents (std::vector<float>& accentOffsets,
                          ParamIds accentProbParam,
                          InputIds accentProbCv,
                          ParamIds accentOffsetParam,
                          InputIds accentOffsetCv,
                          int bufferChannel,
                          bool rng = false)
    {
        if (TBase::inputs[accentProbCv].getChannels() > 1)
        {
            auto scale = rng ? rand01() : 1.0f;
            accentOffsets[bufferChannel] = scale
                                           * fixedAccent (accentProbParam,
                                                          accentProbCv,
                                                          accentOffsetParam,
                                                          accentOffsetCv,
                                                          bufferChannel);
        }
        else
        {
            auto scale = rng ? rand01() : 1.0f;
            auto accent = scale
                          * fixedAccent (accentProbParam,
                                         accentProbCv,
                                         accentOffsetParam,
                                         accentOffsetCv,
                                         0);
            for (auto& a : accentOffsets)
                a = accent;
        }
    }

    void generateAccents (int bufferChannel)
    {
        generateAccents (accentAOffsets,
                         ACCENT_A_PROB_PARAM,
                         ACCENT_A_PROB_INPUT,
                         ACCENT_A_OFFSET_PARAM,
                         ACCENT_A_OFFSET_INPUT,
                         bufferChannel);

        generateAccents (accentBOffsets,
                         ACCENT_B_PROB_PARAM,
                         ACCENT_B_PROB_INPUT,
                         ACCENT_B_OFFSET_PARAM,
                         ACCENT_B_OFFSET_INPUT,
                         bufferChannel);

        generateAccents (accentRngOffsets,
                         ACCENT_RNG_PROB_PARAM,
                         ACCENT_RNG_PROB_INPUT,
                         ACCENT_RNG_OFFSET_PARAM,
                         ACCENT_RNG_MAX_INPUT,
                         bufferChannel,
                         true);
    }

    float fixedAccent (ParamIds probParam,
                       InputIds probInput,
                       ParamIds offsetParam,
                       InputIds offsetInput,
                       int c)
    {
        auto ret = 0.0f;
        auto accentProb = clamp (TBase::params[probParam].getValue()
                                     + TBase::inputs[probInput].getPolyVoltage (c) / 10.0f,
                                 0.0f,
                                 1.0f);
        auto useAccent = accentProb > rand01();
        if (useAccent)
            ret = clamp (TBase::params[offsetParam].getValue()
                             + TBase::inputs[offsetInput].getPolyVoltage (c),
                         -10.0f,
                         10.0f);

        return ret;
    }

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;
    typedef float T; // use floats for all signals
};

template <class TBase>
inline void PolyShiftRegisterComp<TBase>::step()
{
    //channels, monophonic as these all act on a single buffer not per output channel
    auto channels = static_cast<int> (clamp (TBase::params[CHANNELS_PARAM].getValue()
                                                 + TBase::inputs[CHANNELS_INPUT].getVoltage(),
                                             1.0f,
                                             static_cast<float> (maxChannels)));

    //trigger and shuffle act per channel buffer
    auto triggered = clockTrigger.process (TBase::inputs[TRIGGER_INPUT].getVoltage());
    auto monoTriggerProb = TBase::inputs[TRIGGER_PROB_INPUT].getChannels() < 2;
    float triggerProb = clamp (TBase::params[TRIGGER_PROB_PARAM].getValue()
                                   + TBase::inputs[TRIGGER_PROB_INPUT].getVoltage() / 10.0f,
                               0.0f,
                               1.0f);
    auto ignoreTrigger = triggerProb > rand01();
    auto shifted = false;
    for (auto c = 0; c < maxChannels; ++c)
    {
        triggerProb = clamp (TBase::params[TRIGGER_PROB_PARAM].getValue()
                                 + TBase::inputs[TRIGGER_PROB_INPUT].getPolyVoltage (c) / 10.0f,
                             0.0f,
                             1.0f);

        auto shuffleProb = clamp (TBase::params[SHUFFLE_PROB_PARAM].getValue()
                                      + TBase::inputs[SHUFFLE_PROB_INPUT].getPolyVoltage (c) / 10.0f,
                                  0.0f,
                                  1.0f);

        if (! monoTriggerProb)
            ignoreTrigger = triggerProb > rand01();

        if (triggered && ! ignoreTrigger)
        {
            shift (TBase::inputs[MAIN_INPUT].getVoltage(), c);
            shifted = true;

            triggerRngGenerator();
            auto useShuffle = shuffleProb > rand01();
            if (useShuffle)
                std::random_shuffle (channelData[c].begin(), channelData[c].end());
            generateAccents (c);
        }
    }
    if (shifted)
    {
        currentChannels++;
        currentChannels = clamp (currentChannels, 1, channels);
    }

    if (resetTrigger.process (TBase::inputs[RESET_INPUT].getVoltage()))
        currentChannels = 1;

    //output channels
    for (auto c = 0; c < std::min (currentChannels, channels); ++c)
    {
        auto out = channelData[c][c];
        out += accentAOffsets[c];
        out += accentBOffsets[c];
        out += accentRngOffsets[c];
        TBase::outputs[MAIN_OUTPUT].setVoltage (out, c);
    }

    TBase::outputs[MAIN_OUTPUT].setChannels (std::min (currentChannels, channels));
}

template <class TBase>
int PolyShiftRegisterDescription<TBase>::getNumParams()
{
    return PolyShiftRegisterComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config PolyShiftRegisterDescription<TBase>::getParam (int i)
{
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Code type", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        case PolyShiftRegisterComp<TBase>::CHANNELS_PARAM:
            ret = { 1.0f, 16.0f, 4.0f, "Channels", " ", 0.0f, 1.0f, 0.0f };
            break;
        case PolyShiftRegisterComp<TBase>::TRIGGER_PROB_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Trigger miss probability", " ", 0.0f, 1.0f, 0.0f };
            break;
        case PolyShiftRegisterComp<TBase>::SHUFFLE_PROB_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Shuffle probability", " ", 0.0f, 1.0f, 0.0f };
            break;
        case PolyShiftRegisterComp<TBase>::ACCENT_A_PROB_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Accent A probability", " ", 0.0f, 1.0f, 0.0f };
            break;
        case PolyShiftRegisterComp<TBase>::ACCENT_A_OFFSET_PARAM:
            ret = { -10.0f, 10.0f, 0.0f, "Accent A offset", " ", 0.0f, 1.0f, 0.0f };
            break;
        case PolyShiftRegisterComp<TBase>::ACCENT_B_PROB_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Accent A probability", " ", 0.0f, 1.0f, 0.0f };
            break;
        case PolyShiftRegisterComp<TBase>::ACCENT_B_OFFSET_PARAM:
            ret = { -10.0f, 10.0f, 0.0f, "Accent B offset", " ", 0.0f, 1.0f, 0.0f };
            break;
        case PolyShiftRegisterComp<TBase>::ACCENT_RNG_PROB_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Accent RNG probability", " ", 0.0f, 1.0f, 0.0f };
            break;
        case PolyShiftRegisterComp<TBase>::ACCENT_RNG_OFFSET_PARAM:
            ret = { -10.0f, 10.0f, 0.0f, "Accent RNG maximum", " ", 0.0f, 1.0f, 0.0f };
            break;
        default:
            assert (false);
    }
    return ret;
}
