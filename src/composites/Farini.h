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
#include "WaveShaper.h"
#include <memory>
#include <vector>
#include <array>
#include "Adsr.h"

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
        for (auto& d : dividers)
            d.setDivisor (divisorRate);
        upsamplerL.setQuality (upSampleQuality);
        decimatorL.setQuality (upSampleQuality);
        upsamplerR.setQuality (upSampleQuality);
        decimatorR.setQuality (upSampleQuality);
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
        SUSTAIN_CV_PARAM,
        RELEASE_PARAM,
        RELEASE_CV_PARAM,
        USE_NLD_PARAM,
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

    static constexpr auto divisorRate = 20U;
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
    sspo::Upsampler<maxUpSampleRate, maxUpSampleQuality, float_4> upsamplerL;
    sspo::Decimator<maxUpSampleRate, maxUpSampleQuality, float_4> decimatorL;
    float_4 oversampleBufferL[maxUpSampleRate];
    sspo::Upsampler<maxUpSampleRate, maxUpSampleQuality, float_4> upsamplerR;
    sspo::Decimator<maxUpSampleRate, maxUpSampleQuality, float_4> decimatorR;
    float_4 oversampleBufferR[maxUpSampleRate];
    std::array<sspo::BiQuad<float_4>, SIMD_MAX_CHANNELS> dcOutFilters;
    std::array<sspo::Adsr_4, SIMD_MAX_CHANNELS> adsrs;
    std::array<ClockDivider, SIMD_MAX_CHANNELS> dividers;
    std::array<float_4, SIMD_MAX_CHANNELS> lastEocGates{ 0.0f };
};

template <class TBase>
inline void FariniComp<TBase>::step()
{
    auto channels = TBase::inputs[GATE_INPUT].getChannels();
    channels = std::max (channels, 1);

    //read parameters as these are constant across all poly channels
    int mode = TBase::params[MODE_PARAM].getValue();
    bool retrig = TBase::params[RETRIG_PARAM].getValue();
    bool resetOnTrigger = TBase::params[RESET_PARAM].getValue();
    bool cycle = TBase::params[CYCLE_PARAM].getValue();
    bool useNLD = TBase::params[USE_NLD_PARAM].getValue();
    float_4 attack = TBase::params[ATTACK_PARAM].getValue();
    float_4 decay = TBase::params[DECAY_PARAM].getValue();
    float_4 sustain = TBase::params[SUSTAIN_PARAM].getValue();
    float_4 release = TBase::params[RELEASE_PARAM].getValue();

    float_4 attackCv = TBase::params[ATTACK_CV_PARAM].getValue();
    float_4 decayCV = TBase::params[DECAY_CV_PARAM].getValue();
    float_4 sustainCV = TBase::params[SUSTAIN_CV_PARAM].getValue();
    float_4 releaseCV = TBase::params[RELEASE_CV_PARAM].getValue();

    //loop over poly channels, using float_4. so 4 channels
    for (auto c = 0; c < channels; c += 4)
    {
        // Read inputs

        attack += (TBase::inputs[ATTACK_INPUT].template getPolyVoltageSimd<float_4> (c)
                   * 0.1f * TBase::params[ATTACK_CV_PARAM].getValue());
        attack = simd::fmax (0.0f, attack);
        decay += (TBase::inputs[DECAY_INPUT].template getPolyVoltageSimd<float_4> (c)
                  * 0.1f * TBase::params[DECAY_CV_PARAM].getValue());
        decay = simd::fmax (0.0f, decay);
        sustain += (TBase::inputs[SUSTAIN_INPUT].template getPolyVoltageSimd<float_4> (c)
                    * 0.1f * TBase::params[SUSTAIN_CV_PARAM].getValue());
        sustain = simd::fmax (0.0f, sustain);
        release += (TBase::inputs[RELEASE_INPUT].template getPolyVoltageSimd<float_4> (c)
                    * 0.1f * TBase::params[RELEASE_CV_PARAM].getValue());
        release = simd::fmax (0.0f, release);

        auto left = TBase::inputs[LEFT_INPUT].template getPolyVoltageSimd<float_4> (c);
        auto right = TBase::inputs[RIGHT_INPUT].template getPolyVoltageSimd<float_4> (c);
        auto triggers = TBase::inputs[TRIGGER_INPUT].template getPolyVoltageSimd<float_4> (c);

        auto gates = TBase::inputs[GATE_INPUT].template getPolyVoltageSimd<float_4> (c);
        gates = cycle ? lastEocGates[c / 4] : gates;
        gates = simd::ifelse (gates > 1.0f, 1.0f, 0.0f);

        if (dividers[c / 4].process())
        {
            // set parameters as these are slow to set
            adsrs[c / 4].setResetOnTrigger (resetOnTrigger);
            adsrs[c / 4].setParameters (attack, decay, sustain, release, sampleRate);
        }

        auto levels = adsrs[c / 4].step (gates);
        auto stages = adsrs[c / 4].getCurrentStages();

        auto attackGates = float_4 (stages[0] == sspo::Adsr_4::ATTACK_STAGE ? 10.0f : 0.0f,
                                    stages[1] == sspo::Adsr_4::ATTACK_STAGE ? 10.0f : 0.0f,
                                    stages[2] == sspo::Adsr_4::ATTACK_STAGE ? 10.0f : 0.0f,
                                    stages[3] == sspo::Adsr_4::ATTACK_STAGE ? 10.0f : 0.0f);

        auto decayGates = float_4 (stages[0] == sspo::Adsr_4::DECAY_STAGE ? 10.0f : 0.0f,
                                   stages[1] == sspo::Adsr_4::DECAY_STAGE ? 10.0f : 0.0f,
                                   stages[2] == sspo::Adsr_4::DECAY_STAGE ? 10.0f : 0.0f,
                                   stages[3] == sspo::Adsr_4::DECAY_STAGE ? 10.0f : 0.0f);

        auto sustainGates = float_4 (stages[0] == sspo::Adsr_4::SUSTAIN_STAGE ? 10.0f : 0.0f,
                                     stages[1] == sspo::Adsr_4::SUSTAIN_STAGE ? 10.0f : 0.0f,
                                     stages[2] == sspo::Adsr_4::SUSTAIN_STAGE ? 10.0f : 0.0f,
                                     stages[3] == sspo::Adsr_4::SUSTAIN_STAGE ? 10.0f : 0.0f);

        auto releaseGates = float_4 (stages[0] == sspo::Adsr_4::RELEASE_STAGE ? 10.0f : 0.0f,
                                     stages[1] == sspo::Adsr_4::RELEASE_STAGE ? 10.0f : 0.0f,
                                     stages[2] == sspo::Adsr_4::RELEASE_STAGE ? 10.0f : 0.0f,
                                     stages[3] == sspo::Adsr_4::RELEASE_STAGE ? 10.0f : 0.0f);

        auto eocGates = float_4 (stages[0] == sspo::Adsr_4::EOC_STAGE ? 10.0f : 0.0f,
                                 stages[1] == sspo::Adsr_4::EOC_STAGE ? 10.0f : 0.0f,
                                 stages[2] == sspo::Adsr_4::EOC_STAGE ? 10.0f : 0.0f,
                                 stages[3] == sspo::Adsr_4::EOC_STAGE ? 10.0f : 0.0f);

        //process audio

        auto leftOut = left * levels;
        auto rightOut = right * levels;

        //only oversample if needed
        // set the upsample rate if NLD used, else 0
        upSampleRate = useNLD ? 3 : 0;
        upsamplerL.setOverSample (upSampleRate);

        if (upSampleRate > 1)
        {
            upsamplerL.process (leftOut * 0.1f, oversampleBufferL);
            for (auto i = 0; i < upSampleRate; ++i)
                oversampleBufferL[i] = WaveShaper::nld.process (oversampleBufferL[i], 7); //atan 5
            decimatorL.setOverSample (upSampleRate);
            leftOut = decimatorL.process (oversampleBufferL) * 10.0f;

            upsamplerR.process (rightOut * 0.1f, oversampleBufferR);
            for (auto i = 0; i < upSampleRate; ++i)
                oversampleBufferR[i] = WaveShaper::nld.process (oversampleBufferR[i], 7); //atan 5
            decimatorR.setOverSample (upSampleRate);
            rightOut = decimatorR.process (oversampleBufferR) * 10.0f;
        }

        //        float_4 out = dcOutFilters[c / 4].process (in);

        //simd'ed out = std::isfinite (out) ? out : 0;
        //        out = rack::simd::ifelse ((movemask (out == out) != 0xF), float_4 (0.0f), out);

        TBase::outputs[ENV_OUTPUT].setVoltageSimd (levels * 10.0f, c);
        TBase::outputs[ATTACK_OUTPUT].setVoltageSimd (attackGates, c);
        TBase::outputs[DECAY_OUTPUT].setVoltageSimd (decayGates, c);
        TBase::outputs[SUSTAIN_OUTPUT].setVoltageSimd (sustainGates, c);
        TBase::outputs[RELEASE_OUTPUT].setVoltageSimd (releaseGates, c);
        TBase::outputs[EOC_OUTPUT].setVoltageSimd (eocGates, c);
        TBase::outputs[LEFT_OUTPUT].setVoltageSimd (leftOut, c);
        TBase::outputs[RIGHT_OUTPUT].setVoltageSimd (rightOut, c);

        lastEocGates[c / 4] = eocGates;
    }
    TBase::outputs[ENV_OUTPUT].setChannels (channels);
    TBase::outputs[ATTACK_OUTPUT].setChannels (channels);
    TBase::outputs[DECAY_OUTPUT].setChannels (channels);
    TBase::outputs[SUSTAIN_OUTPUT].setChannels (channels);
    TBase::outputs[RELEASE_OUTPUT].setChannels (channels);
    TBase::outputs[EOC_OUTPUT].setChannels (channels);

    TBase::outputs[LEFT_OUTPUT].setChannels (TBase::inputs[LEFT_INPUT].getChannels());
    TBase::outputs[RIGHT_OUTPUT].setChannels (TBase::inputs[RIGHT_INPUT].getChannels());
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

        case FariniComp<TBase>::SUSTAIN_CV_PARAM:
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