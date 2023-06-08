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
#include "CircularBuffer.h"
#include "HardLimiter.h"
#include "LookupTable.h"
//#include "resampler.hpp"

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
class CombFilterDescription : public IComposite
{
public:
    Config getParam (int i) override;
    int getNumParams() override;
};

/**
 * Complete CombFilter composite
 *
 * If TBase is WidgetComposite, this class is used as the implementation part of the KSDelay module.
 * If TBase is TestComposite, this class may stand alone for unit tests.
 */

template <class TBase>
class CombFilterComp : public TBase
{
public:
    CombFilterComp (Module* module) : TBase (module)
    {
    }

    CombFilterComp() : TBase()
    {
    }

    virtual ~CombFilterComp()
    {
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<CombFilterDescription<TBase>>();
    }

    enum ParamIds
    {
        FREQUENCY_PARAM,
        FREQUENCY_CV_ATTENUVERTER_PARAM,
        COMB_CV_ATTENUVERTER_PARAM,
        COMB_PARAM,
        FEEDBACK_CV_ATTENUVERTER_PARAM,
        FEEDBACK_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        VOCT_INPUT,
        FREQ_CV_INPUT,
        COMB_CV_INPUT,
        FEEDBACK_CV_INPUT,
        MAIN_INPUT,
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

    static constexpr float dcOutCutoff = 4.0f;
    float maxFreq = 20000;
    float sampleRate = 1;
    float samplePeriod = 1;

    std::vector<sspo::CircularBuffer<float>> buffers;
    std::vector<sspo::Compressor> limiters;
    std::vector<dsp::RCFilter> dcOutFilters;

    void setSampleRate (float rate)
    {
        sampleRate = rate;
        samplePeriod = 1.0f / sampleRate;

        maxFreq = std::min (20000.0f, sampleRate / 2.0f);

        for (auto& d : dcOutFilters)
            d.setCutoffFreq (dcOutCutoff / sampleRate);

        for (auto& l : limiters)
            l.setSampleRate (sampleRate);
    }

    // must be called after setSampleRate
    void init()
    {
        buffers.resize (PORT_MAX_CHANNELS);
        for (auto& b : buffers)
            b.reset (4096);

        dcOutFilters.resize (PORT_MAX_CHANNELS);
        for (auto& d : dcOutFilters)
            d.setCutoffFreq (dcOutCutoff / sampleRate);

        limiters.resize (PORT_MAX_CHANNELS);
        for (auto& l : limiters)
        {
            l.setSampleRate (sampleRate);
            l.setTimes (0.001, 0.120f);
            l.threshold = -0.3f;
            l.ratio = 10.5f;
        }
    }

    void step() override;
};

template <class TBase>
inline void CombFilterComp<TBase>::step()
{
    auto channels = std::max (1, TBase::inputs[MAIN_INPUT].getChannels());
    auto freqParam = TBase::params[FREQUENCY_PARAM].getValue();
    auto freqAttenuverterParam = TBase::params[FREQUENCY_CV_ATTENUVERTER_PARAM].getValue();
    auto combParam = TBase::params[COMB_PARAM].getValue();
    auto combAttenuverterParam = TBase::params[COMB_CV_ATTENUVERTER_PARAM].getValue();
    auto feedbackParam = TBase::params[FEEDBACK_PARAM].getValue();
    auto feedbackAttenuverterParam = TBase::params[FEEDBACK_CV_ATTENUVERTER_PARAM].getValue();

    for (auto c = 0; c < channels; ++c)
    {
        auto in = TBase::inputs[MAIN_INPUT].getPolyVoltage (c) / 5.0f;

        auto frequency = freqParam;
        frequency += TBase::inputs[VOCT_INPUT].getPolyVoltage (c);
        frequency += (TBase::inputs[FREQ_CV_INPUT].getPolyVoltage (c) * freqAttenuverterParam);
        frequency = dsp::FREQ_C4 * simd::pow (2.0f, frequency);
        frequency = clamp (frequency, 0.1f, maxFreq);

        auto feedback = feedbackParam + feedbackAttenuverterParam * (TBase::inputs[FEEDBACK_CV_INPUT].getPolyVoltage (c) / 5.0f);
        feedback = clamp (feedback, -0.9f, 0.9f);

        auto comb = combParam + combAttenuverterParam * (TBase::inputs[COMB_CV_INPUT].getPolyVoltage (c) / 5.0f);
        comb = clamp (comb, -1.0f, 1.0f);

        auto delayTime = 1.0f / frequency;
        auto index = delayTime * sampleRate;

        in += buffers[c].readBuffer (index) * comb * feedback;

        auto out = in + buffers[c].readBuffer (index) * comb;
        buffers[c].writeBuffer (in);

        dcOutFilters[c].process (out);
        out = dcOutFilters[c].highpass();

        out = limiters[c].process (out);

        TBase::outputs[MAIN_OUTPUT].setVoltage (out * 5.0f, c);
    }
    TBase::outputs[MAIN_OUTPUT].setChannels (channels);
}

template <class TBase>
int CombFilterDescription<TBase>::getNumParams()
{
    return CombFilterComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config CombFilterDescription<TBase>::getParam (int i)
{
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Code type", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        case CombFilterComp<TBase>::FREQUENCY_PARAM:
            ret = { -4.0f, 4.0f, 0.0f, "Frequency", " ", 0, 1, 0.0f };
            break;
        case CombFilterComp<TBase>::FREQUENCY_CV_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "Frequency Attenuverter", " ", 0, 1, 0.0f };
            break;
        case CombFilterComp<TBase>::COMB_CV_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "Comb Attenuverter", " ", 0, 1, 0.0f };
            break;
        case CombFilterComp<TBase>::COMB_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "Comb", " ", 0, 1, 0.0f };
            break;
        case CombFilterComp<TBase>::FEEDBACK_CV_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "Feedback Attenuverter", " ", 0, 1, 0.0f };
            break;
        case CombFilterComp<TBase>::FEEDBACK_PARAM:
            ret = { 0.0f, 1.1f, 0.f, "Feedback", " ", 0, 1, 0.0f };
            break;
        default:
            assert (false);
    }
    return ret;
}
