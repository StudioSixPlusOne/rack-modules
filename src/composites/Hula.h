/*
 * Copyright (c) 2021 Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>
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
#include "LookupTable.h"
#include "AudioMath.h"
#include "dsp/UtilityFilters.h"

namespace rack
{
    namespace engine
    {
        struct Module;
    }
} // namespace rack
using Module = ::rack::engine::Module;
using namespace rack;

using namespace sspo::AudioMath::LookupTable;
using namespace sspo::AudioMath;

template <class TBase>
class HulaDescription : public IComposite
{
public:
    Config getParam (int i) override;
    int getNumParams() override;
};

/**
 * Complete Hulacomposite
 *
 * If TBase is WidgetComposite, this class is used as the implementation part of the KSDelay module.
 * If TBase is TestComposite, this class may stand alone for unit tests.
 */

template <class TBase>
class HulaComp : public TBase
{
public:
    HulaComp (Module* module) : TBase (module)
    {
    }

    HulaComp() : TBase()
    {
        //decimator = dsp::Decimator<oversampleCount, oversampleQuality>(1.0f);
    }

    virtual ~HulaComp()
    {
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<HulaDescription<TBase>>();
    }

    void setSampleRate (float rate)
    {
        reciprocalSampleRate = 1 / rate;
        sampleRate = rate;
        maxFc = std::min (rate / 2.0f, 20000.0f);
    }

    // must be called after setSampleRate
    void init()
    {
        for (auto& dc : dcOutFilters)
            dc.setCutoffFreq (dcOutCutoff / sampleRate);
        // set random detune, += 5 cent;

        for (auto& f : fineTuneVocts)
            f = (rand01() * 2.0f - 1.0f) * 5.0f / (12.0f * 100.0f);
    }

    enum ParamIds
    {
        RATIO_PARAM,
        SEMITONE_PARAM,
        OCTAVE_PARAM,
        DEPTH_PARAM,
        FEEDBACK_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        VOCT_INPUT,
        DEPTH_CV_INPUT,
        FEEDBACK_CV_INPUT,
        FM_INPUT,
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

    float reciprocalSampleRate = 1;
    float sampleRate = 1;
    float maxFc = 1;
    std::array<float, PORT_MAX_CHANNELS> lastOuts{ 0.0f };

    std::array<float, PORT_MAX_CHANNELS> phases{ 0.0f };
    std::array<float, PORT_MAX_CHANNELS> fineTuneVocts{ 0.0f };

    static constexpr int oversampleCount = 4;
    static constexpr int oversampleQuality = 4;

    std::array<sspo::Decimator<oversampleCount, oversampleQuality, float>, PORT_MAX_CHANNELS> decimators;
    std::array<std::array<float, oversampleCount>, PORT_MAX_CHANNELS> oversampleBuffers;
    std::array<dsp::RCFilter, PORT_MAX_CHANNELS> dcOutFilters;
    static constexpr float dcOutCutoff = 5.5f;

    void step() override;
};

template <class TBase>
inline void HulaComp<TBase>::step()
{
    auto channels = std::max (1, TBase::inputs[VOCT_INPUT].getChannels());

    for (auto c = 0; c < channels; ++c)
    {
        //calculate frequency
        auto voct = TBase::inputs[VOCT_INPUT].getPolyVoltage (c) + fineTuneVocts[c];
        voct += static_cast<int> (TBase::params[OCTAVE_PARAM].getValue());
        voct += static_cast<int> (TBase::params[SEMITONE_PARAM].getValue()) * (1.0f / 12.0f);
        auto freq = dsp::FREQ_C4 * lookup.pow2 (voct);
        freq *= static_cast<int> (TBase::params[RATIO_PARAM].getValue());
        auto phaseInc = freq * reciprocalSampleRate / oversampleCount;

        //phase offset as fm is implemented as phase modulation
        auto phaseOffset = TBase::params[FEEDBACK_PARAM].getValue() * 0.25 * (lastOuts[c]);
        if (TBase::inputs[FEEDBACK_CV_INPUT].isConnected())
        {
            phaseOffset *= std::abs (TBase::inputs[FEEDBACK_CV_INPUT].getPolyVoltage (c) * 0.1f);
        }
        auto fmIn = TBase::inputs[FM_INPUT].getPolyVoltage (c) * 0.2; // scale from +-5 to +=1

        if (TBase::inputs[DEPTH_CV_INPUT].isConnected())
        {
            fmIn *= std::abs (TBase::inputs[DEPTH_CV_INPUT].getPolyVoltage (c) * 0.1f);
        }

        phaseOffset += TBase::params[DEPTH_PARAM].getValue() * (fmIn);

        phaseOffset = clamp (phaseOffset, -3.0f, 3.0f);

        for (auto i = 0; i < oversampleCount; ++i)
        {
            //progress
            phases[c] += phaseInc;
            while (phases[c] > 1.0f)
                phases[c] -= 1.0f;
            oversampleBuffers[c][i] = lookup.hulaSin ((phases[c] + phaseOffset) * k_2pi);
        }
        dcOutFilters[c].process (decimators[c].process (oversampleBuffers[c].data()));
        lastOuts[c] = dcOutFilters[c].highpass();
        TBase::outputs[MAIN_OUTPUT].setVoltage (lastOuts[c] * 5.0f, c);
    }
    TBase::outputs[MAIN_OUTPUT].setChannels (channels);
}

template <class TBase>
int HulaDescription<TBase>::getNumParams()
{
    return HulaComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config HulaDescription<TBase>::getParam (int i)
{
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Code type", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        //TODO
        case HulaComp<TBase>::RATIO_PARAM:
            ret = { 1.0f, 10.0f, 1.0f, "Ratio", " ", 0, 1, 0.0f };
            break;
        case HulaComp<TBase>::SEMITONE_PARAM:
            ret = { 0.0f, 12.0f, 0.0f, "Semitone", " ", 0, 1, 0.0f };
            break;
        case HulaComp<TBase>::OCTAVE_PARAM:
            ret = { -5.0f, 5.0f, 0.0f, "Octave", " ", 0, 1, 0.0f };
            break;
        case HulaComp<TBase>::DEPTH_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Depth", " ", 0, 1, 0.0f };
            break;
        case HulaComp<TBase>::FEEDBACK_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Feedback", " ", 0, 1, 0.0f };
            break;

        default:
            assert (false);
    }
    return ret;
}