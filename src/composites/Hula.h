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

#include <array>

#include "IComposite.h"
#include "LookupTable.h"
#include "AudioMath.h"
#include "dsp/UtilityFilters.h"

#include "simd/functions.hpp"
#include "simd/sse_mathfun.h"
#include "simd/sse_mathfun_extension.h"

#include <memory>

namespace rack
{
    namespace engine
    {
        struct Module;
    }
} // namespace rack
using Module = ::rack::engine::Module;
using namespace rack;
using float_4 = ::rack::simd::float_4;

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
 * If TBase is WidgetComposite, this class is used as the implementation part of the Hula module.
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

    void setSampleRate (float rate);

    // must be called after setSampleRate
    void init();

    enum ParamIds
    {
        RATIO_PARAM,
        SEMITONE_PARAM,
        OCTAVE_PARAM,
        DEPTH_PARAM,
        FEEDBACK_PARAM,
        UNISON_PARAM,
        OVERSAMPLE_PARAM,
        DEFAULT_TUNING_PARAM,
        DC_OFFSET_PARAM,
        SCALE_PARAM,
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

    constexpr static int SIMD_CHANNELS = 4;
    float reciprocalSampleRate = 1;
    float sampleRate = 1;
    std::array<float_4, SIMD_CHANNELS> lastOuts;
    std::array<float_4, SIMD_CHANNELS> phases;
    std::array<float_4, SIMD_CHANNELS> fineTuneVocts;

    static constexpr int maxOversampleCount = 16;
    static constexpr int oversampleQuality = 1;

    std::array<sspo::Decimator<maxOversampleCount, oversampleQuality, float_4>, SIMD_CHANNELS> decimators;
    std::array<std::array<float_4, maxOversampleCount>, SIMD_CHANNELS> oversampleBuffers;
    std::array<sspo::BiQuad<float_4>, SIMD_CHANNELS> dcOutFilters;
    std::array<sspo::BiQuad<float_4>, SIMD_CHANNELS> lpFilters;

    std::array<sspo::BiQuad<float_4>, SIMD_CHANNELS> depthFilters;
    std::array<sspo::BiQuad<float_4>, SIMD_CHANNELS> feedbackFilters;

    static constexpr float dcOutCutoff = 5.5f;

    void step() override;
};

template <class TBase>
void HulaComp<TBase>::setSampleRate (float rate)
{
    reciprocalSampleRate = 1 / rate;
    sampleRate = rate;

    for (auto& l : lpFilters)
        l.setButterworthLp2 (rate, std::min (10e3f, rate * 0.25f));

    for (auto& dc : dcOutFilters)
        dc.setButterworthHp2 (sampleRate, dcOutCutoff);

    /// filter the changes in depth and feedback by fs/40
    for (auto& d : depthFilters)
        d.setButterworthLp2 (1000.0f, 25.0f);

    for (auto& f : feedbackFilters)
        f.setButterworthLp2 (1000.0f, 25.0f);
}

template <class TBase>
void HulaComp<TBase>::init()
{
    // set random detune, += 3 cent;
    auto detuneLimit = 2.0f; //cents
    for (auto& f : fineTuneVocts)
        f = float_4 ((rand01() * 2.0f - 1.0f) * detuneLimit / (12.0f * 100.0f),
                     (rand01() * 2.0f - 1.0f) * detuneLimit / (12.0f * 100.0f),
                     (rand01() * 2.0f - 1.0f) * detuneLimit / (12.0f * 100.0f),
                     (rand01() * 2.0f - 1.0f) * detuneLimit / (12.0f * 100.0f));

    for (auto& l : lastOuts)
        l = float_4 (0);

    for (auto& p : phases)
        p = float_4 (rand01(), rand01(), rand01(), rand01());

    for (auto& d : decimators)
        d.setQuality(1);

    for (auto& os : oversampleBuffers)
        for (auto& o : os)
            o = float_4::zero();
}

template <class TBase>
inline void HulaComp<TBase>::step()
{
    auto channels = std::max (1, TBase::inputs[VOCT_INPUT].getChannels());

    //we trade unison for polyphonic channels, so if we have unison
    //this becomes the channel count

    auto isUnison = TBase::params[UNISON_PARAM].getValue() > 1.5f;

    channels = simd::ifelse (isUnison,
                             TBase::params[UNISON_PARAM].getValue(),
                             channels);

    int oversampleCount = std::max (TBase::params[OVERSAMPLE_PARAM].getValue(), 1.0f);

    for (auto c = 0; c < channels; c += 4)
    {
        //calculate frequency
        float_4 voct = TBase::inputs[VOCT_INPUT].template getPolyVoltageSimd<float_4> (c) + fineTuneVocts[c / 4];
        voct += simd::floor (TBase::params[OCTAVE_PARAM].getValue());
        voct += simd::floor (TBase::params[SEMITONE_PARAM].getValue()) * (1.0f / 12.0f);
        float_4 freq = lookup.pow2 (voct) * TBase::params[DEFAULT_TUNING_PARAM].getValue();
        freq *= lookup.pow2 (TBase::params[RATIO_PARAM].getValue());

        float_4 phaseInc = freq * reciprocalSampleRate / oversampleCount;

        //phase offset as fm is implemented as phase modulation
        float_4 phaseOffset = TBase::params[FEEDBACK_PARAM].getValue() * 0.053f * (lastOuts[c / 4]);
        if (TBase::inputs[FEEDBACK_CV_INPUT].isConnected())
        {
            phaseOffset *= feedbackFilters[c / 4].process (simd::abs (TBase::inputs[FEEDBACK_CV_INPUT].template getPolyVoltageSimd<float_4> (c) * 0.1f));
        }
        float_4 fmIn = TBase::inputs[FM_INPUT].template getPolyVoltageSimd<float_4> (c) * 0.2f; // scale from +-5 to +=1

        if (TBase::inputs[DEPTH_CV_INPUT].isConnected())
        {
            fmIn *= depthFilters[c / 4].process (simd::abs (TBase::inputs[DEPTH_CV_INPUT].template getPolyVoltageSimd<float_4> (c) * 0.1f));
        }

        phaseOffset += TBase::params[DEPTH_PARAM].getValue() * fmIn;

        decimators[c / 4].setOverSample (oversampleCount);

        float_4 processed = float_4::zero();

        if (oversampleCount > 1)
        {
            for (auto i = 0; i < oversampleCount; ++i)
            {
                //generate oversampled signal
                phases[c / 4] += phaseInc;
                phases[c / 4] = simd::ifelse (phases[c / 4] > float_4 (1.0f),
                                              phases[c / 4] - simd::trunc (phases[c / 4]),
                                              phases[c / 4]);
                oversampleBuffers[c / 4][i] = lookup.hulaSin4 ((phases[c / 4] + phaseOffset) * k_2pi);
            }

            processed = decimators[c / 4].process (oversampleBuffers[c / 4].data());
        }
        else
        {
            phases[c / 4] += phaseInc;
            phases[c / 4] = simd::ifelse (phases[c / 4] > float_4 (1.0f),
                                          phases[c / 4] - simd::trunc (phases[c / 4]),
                                          phases[c / 4]);
            processed = lookup.hulaSin4 ((phases[c / 4] + phaseOffset) * k_2pi);
        }

        //only dc block for audio
        lastOuts[c / 4] = processed * 5.0f;
        processed = lastOuts[c / 4] * TBase::params[SCALE_PARAM].getValue()
                    + TBase::params[DC_OFFSET_PARAM].getValue();
        TBase::outputs[MAIN_OUTPUT].setVoltageSimd (lpFilters[c / 4].process (processed), c);
    }

    TBase::outputs[MAIN_OUTPUT].setChannels (channels);

    // sum channels if unison
    if (isUnison)
    {
        TBase::outputs[MAIN_OUTPUT].setVoltage (TBase::outputs[MAIN_OUTPUT].getVoltageSum() / simd::sqrt (channels));
        TBase::outputs[MAIN_OUTPUT].setChannels (1);
    }
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
        case HulaComp<TBase>::RATIO_PARAM:
            ret = { -4, 4, 0.0f, "Ratio", " ", 2.0f, 1, 0.0f };
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
        case HulaComp<TBase>::UNISON_PARAM:
            ret = { 1.0f, 16.0f, 1.0f, "Unison", " ", 0, 1, 0.0f };
            break;
        case HulaComp<TBase>::OVERSAMPLE_PARAM:
            ret = { 1.0f, 8.0f, 4.0f, "Over Sample Rate", " ", 0, 1, 0.0f };
            break;
        case HulaComp<TBase>::DEFAULT_TUNING_PARAM:
            ret = { 0.00001f, 10000.0f, dsp::FREQ_C4, "Over Sample Rate", " ", 0, 1, 0.0f };
            break;
        case HulaComp<TBase>::DC_OFFSET_PARAM:
            ret = { -10.0f, 10.0f, 0.0f, "Output DC Offset", " ", 0, 1, 0.0f };
            break;
        case HulaComp<TBase>::SCALE_PARAM:
            ret = { -2.0f, 2.0f, 1.0f, "Scale", " ", 0, 1, 0.0f };
            break;
        default:
            assert (false);
    }
    return ret;
}
