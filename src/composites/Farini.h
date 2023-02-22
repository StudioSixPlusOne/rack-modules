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
class FariniDescription : public IComposite
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
class FariniComp : public TBase
{
public:
    FariniComp (Module* module) : TBase (module)
    {
    }

    FariniComp() : TBase()
    {
    }

    virtual ~FariniComp()
    {
    }

    /** Implement IComposite
    */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<FariniDescription<TBase>>();
    }

    void setSampleRate (float rate)
    {
        sampleRate = rate;
        sampleTime = 1.0f / rate;
        maxFreq = std::min (rate / 2.0f, 20000.0f);

        // set samplerate on any dsp objects
        for (auto& dc : dcOutFilters)
            dc.setButterworthHp2 (sampleRate, dcInFilterCutoff);
    }

    // must be called after setSampleRate
    void init()
    {
        //resize arrays
        //initialise dsp object

        sspo::AudioMath::defaultGenerator.seed (time (NULL));
        dcOutFilters.resize (SIMD_MAX_CHANNELS);
        divider.setDivisor (divisorRate);
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
        MODE_PARAM,
        RETRIG_PARAM,
        RESET_PARAM,
        CYCLE_PARAM,
        ATTACK_PARAM,
        ATTACK_CV_PARAM,
        DECAY_PARAM,
        DECAY_CV_PARAM,
        SUSTAIN_PARAM,
        SUSTAIN__CV_PARAM,
        RELEASE_PARAM,
        RELEASE_CV_PARAM,
        NUM_PARAMS
    };
    enum InputId
    {
        ATTACK_INPUT,
        DECAY_INPUT,
        SUSTAIN_INPUT,
        RELEASE_INPUT,
        LEFT_INPUT,
        RIGHT_INPUT,
        TRIGGER_INPUT,
        GATE_INPUT,
        NUM_INPUTS
    };
    enum OutputId
    {
        ATTACK_OUTPUT,
        DECAY_OUTPUT,
        SUSTAIN_OUTPUT,
        RELEASE_OUTPUT,
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
        EOC_OUTPUT,
        ENV_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightId
    {
        NUM_LIGHTS
    };

    static constexpr auto divisorRate = 4U;
    constexpr static float dcInFilterCutoff = 5.5f;
    static constexpr float minFreq = 0.0f;
    float maxFreq = 20000.0f;
    static constexpr int SIMD_MAX_CHANNELS = 4;
    float sampleRate = 1.0f;
    float sampleTime = 1.0f;
    static constexpr auto semitoneVoltage = 1.0 / 12.0f;
    static constexpr int maxUpSampleRate = 12;
    static constexpr int maxUpSampleQuality = 12;
    int upSampleRate = 1;
    int upSampleQuality = 1;
    sspo::Upsampler<maxUpSampleRate, maxUpSampleQuality, float_4> upsampler;
    sspo::Decimator<maxUpSampleRate, maxUpSampleQuality, float_4> decimator;
    float_4 oversampleBuffer[maxUpSampleRate];
    std::vector<sspo::BiQuad<float_4>> dcOutFilters;
    ClockDivider divider;
};

template <class TBase>
inline void FariniComp<TBase>::step()
{
    auto channels = TBase::inputs[TRIGGER_INPUT].getChannels();
    channels = std::max (channels, 1);

    //read parameters as these are constant across all poly channels

    //loop over poly channels, using float_4. so 4 channels
    for (auto c = 0; c < channels; c += 4)
    {
        // Read inputs
        //        if (TBase::inputs[VCA_CV_INPUT].isConnected())
        //        {
        //            vcaGain = vcaGain + (TBase::inputs[VCA_CV_INPUT].template getPolyVoltageSimd<float_4> (c) * 0.1f * TBase::params[VCA_CV_ATTENUVERTER_PARAM].getValue());
        //        }
        //
        auto in = TBase::inputs[LEFT_INPUT].template getPolyVoltageSimd<float_4> (c);

        if (divider.process())
        {
            // slower response stuff here
        }

        //process audio
        //only oversample if needed

        if (upSampleRate > 1)
        {
            upsampler.process (in, oversampleBuffer);
            for (auto i = 0; i < upSampleRate; ++i)
                oversampleBuffer[i] = 0.0f; //add processing
            in = decimator.process (oversampleBuffer);
        }
        else
        {
            in = 0.0f; //add processing
        }

        float_4 out = dcOutFilters[c / 4].process (in);

        //simd'ed out = std::isfinite (out) ? out : 0;
        out = rack::simd::ifelse ((movemask (out == out) != 0xF), float_4 (0.0f), out);

        TBase::outputs[LEFT_OUTPUT].setVoltageSimd (out, c);
    }
    TBase::outputs[LEFT_OUTPUT].setChannels (channels);
}

template <class TBase>
int FariniDescription<TBase>::getNumParams()
{
    return FariniComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config FariniDescription<TBase>::getParam (int i)
{
    auto freqBase = static_cast<float> (std::pow (2, 10.0f));
    auto freqMul = static_cast<float> (dsp::FREQ_C4 / std::pow (2, 5.f));
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Name", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        case FariniComp<TBase>::MODE_PARAM:
            ret = { 0.0f, 2.0f, 0.0f, "MODE", " ", 0.0f, 1.0f, 0.0f };
            break;

        case FariniComp<TBase>::RETRIG_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "RETRIG", " ", 0.0f, 1.0f, 0.0f };
            break;

        case FariniComp<TBase>::RESET_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "RESET", " ", 0.0f, 1.0f, 0.0f };
            break;

        case FariniComp<TBase>::CYCLE_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "CYCLE", " ", 0.0f, 1.0f, 0.0f };
            break;

        case FariniComp<TBase>::ATTACK_PARAM:
            ret = { 0.0f, 1.0f, 0.5f, "ATTACK", " ", 0.0f, 1.0f, 0.0f };
            break;

        case FariniComp<TBase>::ATTACK_CV_PARAM:
            ret = { 0.0f, 1.0f, 0.5f, "ATTACK_CV", " ", 0.0f, 1.0f, 0.0f };
            break;

        case FariniComp<TBase>::DECAY_PARAM:
            ret = { 0.0f, 1.0f, 0.5f, "DECAY", " ", 0.0f, 1.0f, 0.0f };
            break;

        case FariniComp<TBase>::DECAY_CV_PARAM:
            ret = { 0.0f, 1.0f, 0.5f, "DECAY_CV", " ", 0.0f, 1.0f, 0.0f };
            break;

        case FariniComp<TBase>::SUSTAIN_PARAM:
            ret = { 0.0f, 1.0f, 0.5f, "SUSTAIN", " ", 0.0f, 1.0f, 0.0f };
            break;

        case FariniComp<TBase>::SUSTAIN__CV_PARAM:
            ret = { 0.0f, 1.0f, 0.5f, "SUSTAIN__CV", " ", 0.0f, 1.0f, 0.0f };
            break;

        case FariniComp<TBase>::RELEASE_PARAM:
            ret = { 0.0f, 1.0f, 0.5f, "RELEASE", " ", 0.0f, 1.0f, 0.0f };
            break;

        case FariniComp<TBase>::RELEASE_CV_PARAM:
            ret = { 0.0f, 1.0f, 0.5f, "RELEASE_CV", " ", 0.0f, 1.0f, 0.0f };
            break;

        default:
            assert (false);
    }
    return ret;
}