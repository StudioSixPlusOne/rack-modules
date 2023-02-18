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
#include "../dsp/WaveShaper.h"
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
class MixDescription : public IComposite
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
class MixComp : public TBase
{
public:
    MixComp (Module* module) : TBase (module)
    {
    }

    MixComp() : TBase()
    {
    }

    virtual ~MixComp()
    {
    }

    /** Implement IComposite
    */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<MixDescription<TBase>>();
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
        upsampler.setQuality (upSampleQuality);
        decimator.setQuality (upSampleQuality);
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
        ONE_PARAM,
        TWO_PARAM,
        THREE_PARAM,
        FOUR_PARAM,
        FIVE_PARAM,
        MAIN_PARAM,
        NLD_PARAM,
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
        MAIN_OUTPUT,
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
inline void MixComp<TBase>::step()
{
    auto channels = std::max ({ TBase::inputs[ONE_INPUT].getChannels(),
                                TBase::inputs[TWO_INPUT].getChannels(),
                                TBase::inputs[THREE_INPUT].getChannels(),
                                TBase::inputs[FOUR_INPUT].getChannels(),
                                TBase::inputs[FIVE_INPUT].getChannels() });

    //read parameters as these are constant across all poly channels

    //loop over poly channels, using float_4. so 4 channels
    for (auto c = 0; c < channels; c += 4)
    {
        auto in = TBase::inputs[ONE_INPUT].template getPolyVoltageSimd<float_4> (c)
                  * TBase::params[ONE_PARAM].getValue();
        in += TBase::inputs[TWO_INPUT].template getPolyVoltageSimd<float_4> (c)
              * TBase::params[TWO_PARAM].getValue();
        in += TBase::inputs[THREE_INPUT].template getPolyVoltageSimd<float_4> (c)
              * TBase::params[THREE_PARAM].getValue();
        in += TBase::inputs[FOUR_INPUT].template getPolyVoltageSimd<float_4> (c)
              * TBase::params[FOUR_PARAM].getValue();
        in += TBase::inputs[FIVE_INPUT].template getPolyVoltageSimd<float_4> (c)
              * TBase::params[FIVE_PARAM].getValue();

        if (divider.process())
        {
            // slower response stuff here
        }

        //process audio
        //only oversample if needed

        // set the upsample rate if NLD used, else 0
        upSampleRate = (TBase::params[NLD_PARAM].getValue() > 0.0f) ? 3 : 0;
        upsampler.setOverSample (upSampleRate);
        if (upSampleRate > 1)
        {
            upsampler.process (in, oversampleBuffer);
            for (auto i = 0; i < upSampleRate; ++i)
            {
                oversampleBuffer[i] = WaveShaper::nld.process (in * 0.1f,
                                                               TBase::params[NLD_PARAM].getValue())
                                      * TBase::params[MAIN_PARAM].getValue();
            }
            decimator.setOverSample (upSampleRate);
            in = decimator.process (oversampleBuffer) * 10.0f;
        }
        else
        {
            in *= TBase::params[MAIN_PARAM].getValue();
        }

        float_4 out = in; //dcOutFilters[c / 4].process (in);

        //simd'ed out = std::isfinite (out) ? out : 0;
        out = rack::simd::ifelse ((movemask (out == out) != 0xF), float_4 (0.0f), out);

        TBase::outputs[MAIN_OUTPUT].setVoltageSimd (out, c);
    }
    TBase::outputs[MAIN_OUTPUT].setChannels (channels);
}

template <class TBase>
int MixDescription<TBase>::getNumParams()
{
    return MixComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config MixDescription<TBase>::getParam (int i)
{
    auto freqBase = static_cast<float> (std::pow (2, 10.0f));
    auto freqMul = static_cast<float> (dsp::FREQ_C4 / std::pow (2, 5.f));
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Name", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        case MixComp<TBase>::ONE_PARAM:
            ret = { 0.0f, 2.0f, 0.5f, "ONE", " ", 0.0f, 1.0f, 0.0f };
            break;

        case MixComp<TBase>::TWO_PARAM:
            ret = { 0.0f, 2.0f, 0.5f, "TWO", " ", 0.0f, 1.0f, 0.0f };
            break;

        case MixComp<TBase>::THREE_PARAM:
            ret = { 0.0f, 2.0f, 0.5f, "THREE", " ", 0.0f, 1.0f, 0.0f };
            break;

        case MixComp<TBase>::FOUR_PARAM:
            ret = { 0.0f, 2.0f, 0.5f, "FOUR", " ", 0.0f, 1.0f, 0.0f };
            break;

        case MixComp<TBase>::FIVE_PARAM:
            ret = { 0.0f, 2.0f, 0.5f, "FIVE", " ", 0.0f, 1.0f, 0.0f };
            break;

        case MixComp<TBase>::MAIN_PARAM:
            ret = { 0.0f, 2.0f, 0.5f, "MAIN", " ", 0.0f, 1.0f, 0.0f };
            break;

        case MixComp<TBase>::NLD_PARAM:
            ret = { 0.0f, static_cast<float> (sspo::AudioMath::WaveShaper::nld.size() - 1), 0.0f, "NLD TYPE", " ", 0.0f, 1.0f, 0.0f };
            break;

        default:
            assert (false);
    }
    return ret;
}