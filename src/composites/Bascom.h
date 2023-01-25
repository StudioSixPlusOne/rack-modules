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
#include <memory>
#include <vector>
#include <time.h>

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
        divider.setDivisor (128);
    }

    enum ParamIds
    {
        FREQUENCY_PARAM,
        FREQUENCY_CV_ATTENUVERTER_PARAM,
        RESONANCE_CV_ATTENUVERTER_PARAM,
        RESONANCE_PARAM,
        DRIVE_CV_ATTENUVERTER_PARAM,
        DRIVE_PARAM,
        OVERSAMPLE_PARAM,
        DECIMATOR_FILTERS_PARAM,
        PARAM_UPDATE_DIVIDER_PARAM,
        COEFF_A_PARAM,
        COEFF_B_PARAM,
        COEFF_C_PARAM,
        COEFF_D_PARAM,
        COEFF_E_PARAM,
        FC_OFFSET_1_PARAM,
        FC_OFFSET_2_PARAM,
        FC_OFFSET_3_PARAM,
        FC_OFFSET_4_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        VOCT_INPUT,
        RESONANCE_CV_INPUT,
        DRIVE_CV_INPUT,
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
    static constexpr float maxRes = 7.0f;
    static constexpr float maxDrive = 30.0f;
    static constexpr int SIMD_MAX_CHANNELS = 4;
    static constexpr int typeCount = 6;
    int currentType = 1;
    float sampleRate = 1.0f;
    float sampleTime = 1.0f;
    std::vector<sspo::synthFilterII::LadderFilter<float_4>> filters;
    ClockDivider divider;
    void step() override;
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
    auto modeParam = static_cast<int> (TBase::params[MODE_PARAM].getValue());
    auto freqAttenuverterParam = TBase::params[FREQUENCY_CV_ATTENUVERTER_PARAM].getValue();
    auto resAttenuverterParam = TBase::params[RESONANCE_CV_ATTENUVERTER_PARAM].getValue();
    auto driveAttenuverterParam = TBase::params[DRIVE_CV_ATTENUVERTER_PARAM].getValue();

    auto noise = float_4 (1e-6f * (2.0f * sspo::AudioMath::rand01() - 1.0f));
    freqParam = freqParam * 10.0f - 5.0f;

    for (auto c = 0; c < channels; c += 4)
    {
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
        frequency = dsp::FREQ_C4 * rack::simd::pow (2.0f, frequency);

        frequency = rack::simd::clamp (frequency, float_4 (0.0f), float_4 (maxFreq));

        auto resonance = float_4 (resParam);
        resonance += (float_4 (TBase::inputs[RESONANCE_CV_INPUT].template getPolyVoltageSimd<float_4> (c)) / 5.0f)
                     * resAttenuverterParam * maxRes;
        resonance = rack::simd::clamp (resonance, float_4 (0.5f), float_4 (maxRes));

        auto drive = float_4 (driveParam);
        drive += (TBase::inputs[DRIVE_CV_INPUT].template getPolyVoltageSimd<float_4> (c) / 5.0f)
                 * driveAttenuverterParam * maxDrive;
        drive = rack::simd::clamp (drive, float_4 (1.0f), float_4 (maxDrive));

        if (divider.process())
        {
            filters[c / 4].setFcQSat (frequency, resonance, drive);
        }

        auto out = filters[c / 4].process ((drive * in) / 10.0f) * 10.0f;
        //out = std::isfinite (out) ? out : 0;

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
            ret = { 1.0f, BascomComp<TBase>::maxDrive, 1.0f, "Drive", " ", 0.0f, 1.0f, 0.0f };
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
            ret = { -18.0f, 18.0f, 0.0f, "Mix Coeff c", " ", 0.0f, 1.0f, 0.0f };
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
        default:
            assert (false);
    }
    return ret;
}