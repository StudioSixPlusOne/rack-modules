/*
 * Copyright (c) 2020, 2023 Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>
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
#include "../dsp/SynthFilterII.h"
#include "../dsp/AudioMath.h"
#include "../dsp/UtilityFilters.h"
#include "../dsp/WaveShaper.h"
#include <memory>
#include <vector>
//#include <time.h>

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
class BascomDescription : public IComposite
{
public:
    Config getParam (int i) override;
    int getNumParams() override;
};

/**
 * Complete Bascom composite
 *
 * If TBase is WidgetComposite, this class is used as the implementation part of the KSDelay module.
 * If TBase is TestComposite, this class may stand alone for unit tests.
 */

template <class TBase>
class BascomComp : public TBase
{
public:
    BascomComp (Module* module) : TBase (module)
    {
    }

    BascomComp() : TBase()
    {
    }

    virtual ~BascomComp()
    {
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<BascomDescription<TBase>>();
    }

    void setSampleRate (float rate)
    {
        sampleRate = rate;
        sampleTime = 1.0f / rate;
        maxFreq = std::min (rate / 2.0f, 20000.0f);
        for (auto& f : filters)
        {
            f.setSampleRate (sampleRate);
        }
    }

    // must be called after setSampleRate
    void init()
    {
        filters.resize (SIMD_MAX_CHANNELS);
        for (auto& f : filters)
        {
            f.setCoeffs (0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
            f.setAux (0.5f);
        }

        sspo::AudioMath::defaultGenerator.seed (time (NULL));
        divider.setDivisor (1);

        dcOutFilters.resize (SIMD_MAX_CHANNELS);
        for (auto& dc : dcOutFilters)
            dc.setButterworthHp2 (sampleRate, dcInFilterCutoff);
    }

#include "BascomParamEnum.h"

    enum InputIds
    {
        VOCT_INPUT,
        RESONANCE_CV_INPUT,
        DRIVE_CV_INPUT,
        MAIN_INPUT,
        FREQ_CV_INPUT,
        VCA_CV_INPUT,
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

    constexpr static float dcInFilterCutoff = 5.5f;
    static constexpr float minFreq = 0.0f;
    float maxFreq = 20000.0f;
    static constexpr float maxRes = 10.0001f;
    static constexpr float maxDrive = 5.0f;
    static constexpr int SIMD_MAX_CHANNELS = 4;
    float sampleRate = 1.0f;
    float sampleTime = 1.0f;
    std::vector<sspo::synthFilterII::LadderFilter<float_4>> filters;
    ClockDivider divider;
    void step() override;
    static constexpr auto semitoneVoltage = 1.0 / 12.0f;
    static constexpr int maxUpSampleRate = 12;
    static constexpr int maxUpSampleQuality = 12;
    sspo::Upsampler<maxUpSampleRate, maxUpSampleQuality, float_4> upsampler;
    sspo::Decimator<maxUpSampleRate, maxUpSampleQuality, float_4> decimator;
    float_4 oversampleBuffer[maxUpSampleRate];
    std::vector<sspo::BiQuad<float_4>> dcOutFilters;
    WaveShaper::Nld nld;
};

template <class TBase>
inline void BascomComp<TBase>::step()
{
    auto channels = std::max (TBase::inputs[MAIN_INPUT].getChannels(),
                              TBase::inputs[VOCT_INPUT].getChannels());
    channels = std::max (channels, 1);
    auto freqParam = TBase::params[FREQUENCY_PARAM].getValue();
    auto resParam = TBase::params[RESONANCE_PARAM].getValue();
    auto driveParam = TBase::params[DRIVE_PARAM].getValue();
    auto freqAttenuverterParam = TBase::params[FREQUENCY_CV_ATTENUVERTER_PARAM].getValue();
    auto resAttenuverterParam = TBase::params[RESONANCE_CV_ATTENUVERTER_PARAM].getValue();
    auto driveAttenuverterParam = TBase::params[DRIVE_CV_ATTENUVERTER_PARAM].getValue();
    auto vcaParam = TBase::params[VCA_PARAM].getValue();

    auto noise = float_4 (1e-3f * (2.0f * sspo::AudioMath::rand01() - 1.0f));
    freqParam = freqParam * 10.0f - 5.0f;

    for (auto c = 0; c < channels; c += 4)
    {
        auto vcaGain = float_4(vcaParam);
        if (TBase::inputs[VCA_CV_INPUT].isConnected())
        {
            vcaGain = vcaGain + (TBase::inputs[VCA_CV_INPUT].template getPolyVoltageSimd<float_4> (c)
                        * 0.1f * TBase::params[VCA_CV_ATTENUVERTER_PARAM].getValue());
        }

        auto in = TBase::inputs[MAIN_INPUT].template getPolyVoltageSimd<float_4> (c);
        // Add -120dB noise to bootstrap self-oscillation
        in += noise;

        auto frequency = float_4 (freqParam);
        if (TBase::inputs[VOCT_INPUT].isConnected())
            frequency += float_4 (TBase::inputs[VOCT_INPUT].template getPolyVoltageSimd<float_4> (c));
        if (TBase::inputs[FREQ_CV_INPUT].isConnected())
        {
            frequency += (TBase::inputs[FREQ_CV_INPUT].template getPolyVoltageSimd<float_4> (c)
                          * freqAttenuverterParam);
        }

        auto frequency1 = frequency + TBase::params[FC_OFFSET_1_PARAM].getValue() * semitoneVoltage;
        auto frequency2 = frequency + TBase::params[FC_OFFSET_2_PARAM].getValue() * semitoneVoltage;
        auto frequency3 = frequency + TBase::params[FC_OFFSET_3_PARAM].getValue() * semitoneVoltage;
        auto frequency4 = frequency + TBase::params[FC_OFFSET_4_PARAM].getValue() * semitoneVoltage;

        frequency1 = dsp::FREQ_C4 * rack::simd::pow (2.0f, frequency1);
        frequency2 = dsp::FREQ_C4 * rack::simd::pow (2.0f, frequency2);
        frequency3 = dsp::FREQ_C4 * rack::simd::pow (2.0f, frequency3);
        frequency4 = dsp::FREQ_C4 * rack::simd::pow (2.0f, frequency4);

        frequency1 = rack::simd::clamp (frequency1, float_4::zero(), float_4 (maxFreq));
        frequency2 = rack::simd::clamp (frequency2, float_4::zero(), float_4 (maxFreq));
        frequency3 = rack::simd::clamp (frequency3, float_4::zero(), float_4 (maxFreq));
        frequency4 = rack::simd::clamp (frequency4, float_4::zero(), float_4 (maxFreq));

        auto resonance = float_4 (resParam);
        resonance += (float_4 (TBase::inputs[RESONANCE_CV_INPUT].template getPolyVoltageSimd<float_4> (c)) / 5.0f)
                     * resAttenuverterParam * maxRes;
        resonance = rack::simd::clamp (resonance, float_4 (0.5f), float_4 (maxRes));

        auto drive = float_4 (driveParam);
        drive += (TBase::inputs[DRIVE_CV_INPUT].template getPolyVoltageSimd<float_4> (c) / 5.0f)
                 * driveAttenuverterParam * maxDrive;
        drive = rack::simd::clamp (drive, float_4 (1.0f), float_4 (maxDrive));

        divider.setDivisor (TBase::params[PARAM_UPDATE_DIVIDER_PARAM].getValue());

        int upsampleRate = std::max (TBase::params[OVERSAMPLE_PARAM].getValue(), 1.1f);

        if (divider.process())
        {
            filters[c / 4].setNldTypes (TBase::params[INPUT_NLD_TYPE_PARAM].getValue(),
                                        TBase::params[RESONANCE_NLD_TYPE_PARAM].getValue(),
                                        TBase::params[STAGE_1_NLD_TYPE_PARAM].getValue(),
                                        TBase::params[STAGE_2_NLD_TYPE_PARAM].getValue(),
                                        TBase::params[STAGE_3_NLD_TYPE_PARAM].getValue(),
                                        TBase::params[STAGE_4_NLD_TYPE_PARAM].getValue());

            filters[c / 4].setSampleRate (sampleRate * upsampleRate);
            filters[c / 4].setFcQSat (frequency1,
                                      frequency2,
                                      frequency3,
                                      frequency4,
                                      resonance,
                                      drive);
            filters[c / 4].setCoeffs (TBase::params[COEFF_A_PARAM].getValue(),
                                      TBase::params[COEFF_B_PARAM].getValue(),
                                      TBase::params[COEFF_C_PARAM].getValue(),
                                      TBase::params[COEFF_D_PARAM].getValue(),
                                      TBase::params[COEFF_E_PARAM].getValue());

            upsampler.setQuality (TBase::params[DECIMATOR_FILTERS_PARAM].getValue());
            decimator.setQuality (TBase::params[DECIMATOR_FILTERS_PARAM].getValue());

            upsampler.setOverSample (upsampleRate);
            decimator.setOverSample (upsampleRate);

            filters[c / 4].setFeedbackPath (TBase::params[FEEDBACK_PATH_PARAM].getValue());
        }

        //only oversample if needed

        if (upsampleRate > 1)
        {
            upsampler.process (in, oversampleBuffer);
            for (auto i = 0; i < upsampleRate; ++i)
                oversampleBuffer[i] = filters[c / 4].process ((drive * oversampleBuffer[i]) / 10.0f) * 10.0f;
            in = decimator.process (oversampleBuffer) * upsampleRate;
        }
        else
        {
            in = filters[c / 4].process ((drive * in) / 10.0f) * 10.0f;
        }

        float_4 out = dcOutFilters[c / 4].process (in);
        //out = std::isfinite (out) ? out : 0;

        out *= vcaGain;

        TBase::outputs[MAIN_OUTPUT].setVoltageSimd (out, c);
    }
    TBase::outputs[MAIN_OUTPUT].setChannels (channels);
}

template <class TBase>
int BascomDescription<TBase>::getNumParams()
{
    return BascomComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config BascomDescription<TBase>::getParam (int i)
{
    WaveShaper::Nld nld;

    auto freqBase = static_cast<float> (std::pow (2, 10.0f));
    auto freqMul = static_cast<float> (dsp::FREQ_C4 / std::pow (2, 5.f));
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Code type", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        case BascomComp<TBase>::FREQUENCY_PARAM:
            ret = { 0.0f, 1.125f, 0.5f, "Frequency", " Hz", freqBase, freqMul };
            break;
        case BascomComp<TBase>::FREQUENCY_CV_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "Frequency CV", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::RESONANCE_CV_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "Resonance CV", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::RESONANCE_PARAM:
            ret = { 0.5f, BascomComp<TBase>::maxRes, 0.707f, "Resonance", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::DRIVE_CV_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "Drive CV", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::DRIVE_PARAM:
            ret = { 0.1f, BascomComp<TBase>::maxDrive, 1.0f, "Drive", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::OVERSAMPLE_PARAM:
            ret = { 1.0f, 12, 1.0f, "Oversample", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::DECIMATOR_FILTERS_PARAM:
            ret = { 1.0f, 12, 1.0f, "Decimator Filters", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::PARAM_UPDATE_DIVIDER_PARAM:
            ret = { 1.0f, 128, 1.0f, "Update Divider", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::COEFF_A_PARAM:
            ret = { -18.0f, 18.0f, 0.0f, "Mix Coeff A", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::COEFF_B_PARAM:
            ret = { -18.0f, 18.0f, 0.0f, "Mix Coeff B", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::COEFF_C_PARAM:
            ret = { -18.0f, 18.0f, 0.0f, "Mix Coeff C", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::COEFF_D_PARAM:
            ret = { -18.0f, 18.0f, 0.0f, "Mix Coeff D", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::COEFF_E_PARAM:
            ret = { -18.0f, 18.0f, 1.0f, "Mix Coeff E", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::FC_OFFSET_1_PARAM:
            ret = { -24.0f, 24.0f, 0.0f, "Fc Offset 1", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::FC_OFFSET_2_PARAM:
            ret = { -24.0f, 24.0f, 0.0f, "Fc Offset 2", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::FC_OFFSET_3_PARAM:
            ret = { -24.0f, 24.0f, 0.0f, "Fc Offset 3", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::FC_OFFSET_4_PARAM:
            ret = { -24.0f, 24.0f, 0.0f, "Fc Offset 4", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::INPUT_NLD_TYPE_PARAM:
            ret = { 0.0f, float (nld.size()), 0.0f, "Input NLD Type", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::RESONANCE_NLD_TYPE_PARAM:
            ret = { -0.0f, float (nld.size()), 0.0f, "Resonance NLD Type", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::STAGE_1_NLD_TYPE_PARAM:
            ret = { 0.0f, float (nld.size()), 0.0f, "NLD TYPE 1", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::STAGE_2_NLD_TYPE_PARAM:
            ret = { 0.0f, float (nld.size()), 0.0f, "NLD TYPE 2", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::STAGE_3_NLD_TYPE_PARAM:
            ret = { 0.0f, float (nld.size()), 0.0f, "NLD TYPE 3", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::STAGE_4_NLD_TYPE_PARAM:
            ret = { 0.0f, float (nld.size()), 0.0f, "NLD TYPE 4", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::VCA_CV_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "VCA IN", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::VCA_PARAM:
            ret = { 0.0f, 2.0, 0.5f, "VCA GAIN", " ", 0.0f, 1.0f, 0.0f };
            break;
        case BascomComp<TBase>::FEEDBACK_PATH_PARAM:
            ret = { 0.0f, 1.0, 0.0f, "Feedback Path", " ", 0.0f, 1.0f, 0.0f };
            break;

        default:
            assert (false);
    }
    return ret;
}