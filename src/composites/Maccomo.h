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
#include "SynthFilter.h"
#include "AudioMath.h"
#include <memory>
#include <vector>
#include <time.h>

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
class MaccomoDescription : public IComposite
{
public:
    Config getParam (int i) override;
    int getNumParams() override;
};

/**
 * Complete Maccomo composite
 *
 * If TBase is WidgetComposite, this class is used as the implementation part of the KSDelay module.
 * If TBase is TestComposite, this class may stand alone for unit tests.
 */

template <class TBase>
class MaccomoComp : public TBase
{
public:
    MaccomoComp (Module* module) : TBase (module)
    {
    }

    MaccomoComp() : TBase()
    {
    }

    virtual ~MaccomoComp()
    {
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<MaccomoDescription<TBase>>();
    }

    void setSampleRate (float rate)
    {
        sampleRate = rate;
        sampleTime = 1.0f / rate;
        maxFreq = std::min (rate / 2.0f, 20000.0f);
    }

    // must be called after setSampleRate
    void init()
    {
        currentTypes.resize (maxChannels);
        for (auto& ct : currentTypes)
            ct = 0;

        filters.resize (maxChannels);
        for (auto& f : filters)
        {
            f.setUseNonLinearProcessing (true);
            f.setType (sspo::MoogLadderFilter::types()[0]);
        }

        sspo::AudioMath::defaultGenerator.seed (time (NULL));
    }

    enum ParamIds
    {
        FREQUENCY_PARAM,
        FREQUENCY_CV_ATTENUVERTER_PARAM,
        RESONANCE_CV_ATTENUVERTER_PARAM,
        RESONANCE_PARAM,
        DRIVE_CV_ATTENUVERTER_PARAM,
        DRIVE_PARAM,
        MODE_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        VOCT_INPUT,
        RESONANCE_CV_INPUT,
        DRIVE_CV_INPUT,
        MODE_CV_INPUT,
        MAIN_INPUT,
        FREQ_CV_INPUT,
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

    static constexpr float minFreq = 0.0f;
    float maxFreq = 20000.0f;
    static constexpr float maxRes = 10.0f;
    static constexpr float maxDrive = 2.0f;
    static constexpr int maxChannels = 16;

    static constexpr int typeCount = 6;
    std::vector<int> currentTypes;

    std::vector<sspo::MoogLadderFilter> filters;

    void step() override;

    float sampleRate = 1.0f;
    float sampleTime = 1.0f;
};

template <class TBase>
inline void MaccomoComp<TBase>::step()
{
    auto channels = std::max (TBase::inputs[MAIN_INPUT].getChannels(),
                              TBase::inputs[VOCT_INPUT].getChannels());
    channels = std::max (channels, 1);
    auto freqParam = TBase::params[FREQUENCY_PARAM].getValue();
    auto resParam = TBase::params[RESONANCE_PARAM].getValue();
    auto driveParam = TBase::params[DRIVE_PARAM].getValue();
    auto modeParam = static_cast<int> (TBase::params[MODE_PARAM].getValue());
    auto freqAttenuverterParam = TBase::params[FREQUENCY_CV_ATTENUVERTER_PARAM].getValue();
    auto resAttenuverterParam = TBase::params[RESONANCE_CV_ATTENUVERTER_PARAM].getValue();
    auto driveAttenuverterParam = TBase::params[DRIVE_CV_ATTENUVERTER_PARAM].getValue();

    freqParam = freqParam * 10.0f - 5.0f;

    for (auto i = 0; i < channels; ++i)
    {
        auto in = TBase::inputs[MAIN_INPUT].getVoltage (i);
        // Add -120dB noise to bootstrap self-oscillation
        in += 1e-6f * (2.f * sspo::AudioMath::rand01() - 1.f);

        auto frequency = freqParam;
        if (TBase::inputs[VOCT_INPUT].isConnected())
            frequency += TBase::inputs[VOCT_INPUT].getPolyVoltage (i);
        if (TBase::inputs[FREQ_CV_INPUT].isConnected())
        {
            frequency += (TBase::inputs[FREQ_CV_INPUT].getPolyVoltage (i)
                          * freqAttenuverterParam);
        }
        frequency = dsp::FREQ_C4 * std::pow (2.0f, frequency);

        frequency = clamp (frequency, 0.0f, maxFreq);

        auto resonance = resParam;
        resonance += (TBase::inputs[RESONANCE_CV_INPUT].getPolyVoltage (i) / 5.0f)
                     * resAttenuverterParam * maxRes;
        resonance = clamp (resonance, 0.0f, maxRes);

        auto drive = driveParam;
        drive += (TBase::inputs[DRIVE_CV_INPUT].getPolyVoltage (i) / 5.0f)
                 * driveAttenuverterParam * maxDrive;
        drive = clamp (drive, 0.0f, maxDrive);

        if (currentTypes[i] != modeParam + int (TBase::inputs[MODE_CV_INPUT].getPolyVoltage (i)))
        {
            currentTypes[i] = clamp (modeParam + int (TBase::inputs[MODE_CV_INPUT].getPolyVoltage (i)), 0, typeCount - 1);
            filters[i].setType (sspo::MoogLadderFilter::types()[currentTypes[i]]);
        }

        filters[i].setParameters (frequency, resonance, drive, 0, sampleRate);

        auto out = filters[i].process (in / 10.0f) * 10.0f;
        out = std::isfinite (out) ? out : 0;
        TBase::outputs[MAIN_OUTPUT].setVoltage (out, i);
    }
    TBase::outputs[MAIN_OUTPUT].setChannels (channels);
}

template <class TBase>
int MaccomoDescription<TBase>::getNumParams()
{
    return MaccomoComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config MaccomoDescription<TBase>::getParam (int i)
{
    auto freqBase = static_cast<float> (std::pow (2, 10.0f));
    auto freqMul = static_cast<float> (dsp::FREQ_C4 / std::pow (2, 5.f));
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Code type", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        case MaccomoComp<TBase>::FREQUENCY_PARAM:
            ret = { 0.0f, 1.125f, 0.5f, "Frequency", " Hz", freqBase, freqMul };
            break;
        case MaccomoComp<TBase>::FREQUENCY_CV_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "Frequency CV", " ", 0.0f, 1.0f, 0.0f };
            break;
        case MaccomoComp<TBase>::RESONANCE_CV_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "Resonance CV", " ", 0.0f, 1.0f, 0.0f };
            break;
        case MaccomoComp<TBase>::RESONANCE_PARAM:
            ret = { 0.0f, MaccomoComp<TBase>::maxRes, 0.0f, "Resonance", " ", 0.0f, 1.0f, 0.0f };
            break;
        case MaccomoComp<TBase>::DRIVE_CV_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "Drive CV", " ", 0.0f, 1.0f, 0.0f };
            break;
        case MaccomoComp<TBase>::DRIVE_PARAM:
            ret = { 0.0f, MaccomoComp<TBase>::maxDrive, 0.6f, "Drive", " ", 0.0f, 1.0f, 0.0f };
            break;
        case MaccomoComp<TBase>::MODE_PARAM:
            ret = { 0.0f, MaccomoComp<TBase>::typeCount - 1, 0.0f, "Type", " ", 0.0f, 1.0f, 0.0f };
            break;
        default:
            assert (false);
    }
    return ret;
}