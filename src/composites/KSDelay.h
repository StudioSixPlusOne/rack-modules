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
#include "LookupTable.h"
#include "CircularBuffer.h"
#include "HardLimiter.h"

#include <cstdlib>
#include <vector>

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
class KSDelayDescription : public IComposite
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
class KSDelayComp : public TBase
{
public:
    KSDelayComp (Module* module) : TBase (module)
    {
    }

    KSDelayComp() : TBase()
    {
    }

    virtual ~KSDelayComp()
    {
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<KSDelayDescription<TBase>>();
    }

    void setSampleRate (float rate)
    {
        reciprocalSampleRate = 1 / rate;
        sampleRate = rate;
        maxCutoff = std::min (rate / 2.0f, 20000.0f);

        for (auto& dc : dcInFilters)
            dc.setParameters (rack::dsp::BiquadFilter::HIGHPASS, dcInFilterCutoff / rate, 0.141f, 1.0f);

        for (auto& l : limiters)
            l.setSampleRate (rate);
    }

    // must be called after setSampleRate
    void init()
    {
        buffers.resize (maxChannels);
        for (auto& b : buffers)
            b.reset (4096);

        dcInFilters.resize (maxChannels);
        for (auto& dc : dcInFilters)
            dc.setParameters (rack::dsp::BiquadFilter::HIGHPASS, dcInFilterCutoff / sampleRate, 0.141f, 1.0f);

        dcOutFilters.resize (maxChannels);
        for (auto& dc : dcOutFilters)
            dc.setParameters (rack::dsp::BiquadFilter::HIGHPASS, dcOutFilterCutoff / sampleRate, 0.141f, 1.0f);

        lastWets.resize (maxChannels);
        for (auto& lw : lastWets)
            lw = 0;

        delayTimes.resize (maxChannels);
        for (auto& d : delayTimes)
            d = 0;

        limiters.resize (maxChannels);
        for (auto& l : limiters)
        {
            l.setTimes (0.00f, 0.0025f);
            l.setSampleRate (sampleRate);
            l.threshold = -0.50f;
        }

        oscphases.resize (maxChannels);
        for (auto& perChannel : oscphases)
        {
            perChannel.resize (maxOscCount);
            for (auto& phase : perChannel)
            {
                phase = static_cast<float> (std::rand()) / RAND_MAX;
            }
        }

        glide.resize (maxChannels);
        for (auto& g : glide)
            g.setRiseFall (0.01f, 0.01f);

        lastOut.resize (maxChannels);
        for (auto& lo : lastOut)
            lo = 0.000f;

        pitches.resize (maxChannels);
        for (auto& p : pitches)
            p = dsp::FREQ_C4;

        unisonTunings = { 0.0f, -0.01952356f, 0.01991221f, -0.06288439f, 0.06216538f, -0.11002313f, 0.10745242f };
    }

    //supersaw curves from "How to emulate the supersaw, Adam Szabo"
    // 0.0f <= x <= 1

    float unisonCentreLevel (const float x)
    {
        return -0.55366 * x + 0.99785;
    }

    float unisonSideLevel (const float x)
    {
        return -0.73764 * x * x + 1.2841 * x + 0.044372;
    }

    // Define all the enums here. This will let the tests and the widget access them.

    enum ParamIds
    {
        OCTAVE_PARAM,
        TUNE_PARAM,
        FEEDBACK_PARAM,
        UNISON_PARAM,
        UNISON_SPREAD_PARAM,
        UNISON_MIX_PARAM,
        STRETCH_PARAM,
        STRETCH_LOCK_PARAM,
        NUM_PARAMS
    };

    enum InputIds
    {
        VOCT,
        FEEDBACK_INPUT,
        IN_INPUT,
        UNISON_INPUT,
        UNISON_SPREAD_INPUT,
        UNISON_MIX_INPUT,
        STRETCH_INPUT,
        NUM_INPUTS
    };

    enum OutputIds
    {
        OUT_OUTPUT,
        NUM_OUTPUTS
    };

    enum LightIds
    {
        NUM_LIGHTS
    };

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

    typedef float T; // use floats for all signals
    float maxCutoff = 20000.0f;

    constexpr static int maxChannels = 16;
    constexpr static float dcInFilterCutoff = 5.5f;
    constexpr static float dcOutFilterCutoff = 10.0f;
    constexpr static int maxOscCount = 7;

    //Oscillator detunings for unisson from "How to emulate the super saw, Adam Szabo"
    //
    std::vector<float> unisonTunings;
    std::vector<float> unisonLevels;
    std::vector<sspo::CircularBuffer<float>> buffers;
    std::vector<rack::dsp::BiquadFilter> dcInFilters;
    std::vector<rack::dsp::BiquadFilter> dcOutFilters;
    std::vector<sspo::Compressor<float>> limiters;
    std::vector<float> lastWets;
    std::vector<float> delayTimes;
    std::vector<std::vector<float>> oscphases;
    std::vector<dsp::SlewLimiter> glide;
    std::vector<int> framesToSample;
    std::vector<float> lastOut;
    std::vector<float> pitches;

private:
    float reciprocalSampleRate = 1.0f;
    float sampleRate = 1.0f;
};

template <class TBase>
inline void KSDelayComp<TBase>::step()
{
    auto channels = std::max (TBase::inputs[IN_INPUT].getChannels(), TBase::inputs[VOCT].getChannels());
    auto octaveParam = TBase::params[OCTAVE_PARAM].getValue();
    auto tuneParam = TBase::params[TUNE_PARAM].getValue();
    auto feedbackParam = TBase::params[FEEDBACK_PARAM].getValue();
    auto unisonCount = TBase::params[UNISON_PARAM].getValue();
    auto unisonSpread = TBase::params[UNISON_SPREAD_PARAM].getValue();
    auto unisonMix = TBase::params[UNISON_MIX_PARAM].getValue();
    auto stretchParam = TBase::params[STRETCH_PARAM].getValue();

    auto glideParam = 0.05f;

    channels = std::max (channels, 1);

    for (auto i = 0; i < channels; ++i)
    {
        auto unisonSpreadCoefficient = lookup.unisonSpread (unisonSpread + std::abs (TBase::inputs[UNISON_SPREAD_INPUT].getPolyVoltage (i) / 10.0f));
        auto unisonSideLevelCoefficient = unisonSideLevel (unisonMix + std::abs (TBase::inputs[UNISON_MIX_INPUT].getPolyVoltage (i) / 10.0f));
        auto unisonCentreLevelCoefficient = unisonCentreLevel (unisonMix + std::abs (TBase::inputs[UNISON_MIX_INPUT].getPolyVoltage (i) / 10.0f));

        auto in = TBase::inputs[IN_INPUT].getPolyVoltage (i);

        in = dcInFilters[i].process (in);
        auto feedback = feedbackParam + TBase::inputs[FEEDBACK_INPUT].getPolyVoltage (i) / 10.0f;
        feedback = clamp (feedback, 0.0f, 0.5f);

        auto glideTime = glideParam;

        glide[i].setRiseFall (glideTime, glideTime);
        auto glideFreq = glide[i].process (10.0f, dsp::FREQ_C4 * lookup.pow2 (TBase::inputs[VOCT].getPolyVoltage (i) + octaveParam + tuneParam / 12.0f));
        glideFreq = clamp (glideFreq, 20.0f, maxCutoff);
        delayTimes[i] = 1.0f / glideFreq;

        auto index = delayTimes[i] * sampleRate - 1.5f;

        // update buffer
        auto wet = buffers[i].readBuffer (index);

        auto stretch = stretchParam;
        if (TBase::inputs[STRETCH_INPUT].isConnected())
        {
            stretch += TBase::inputs[STRETCH_INPUT].getPolyVoltage (i) / 10.0;
        }
        stretch = stretch * 0.0003f * glideFreq * glideFreq;

        auto nonStretchProbabilty = 1.0f / stretch;
        auto useStretch = (1.0f - nonStretchProbabilty) > sspo::AudioMath::rand01();

        auto dry = useStretch
                       ? in + wet
                       : in + lastWets[i] * feedback + 0.5f * wet;
        dry = 5.0f * limiters[i].process (dry / 5.0f);
        buffers[i].writeBuffer (dry);
        lastWets[i] = wet;

        // calc phases
        auto mixedOsc = 0.0f;
        auto unison = std::abs (std::min (static_cast<int> (unisonCount + TBase::inputs[UNISON_INPUT].getPolyVoltage (i)), 7));
        if (unisonCount == 1)
            unisonCentreLevelCoefficient = 1.0f;
        for (int osc = 0; osc < unison; ++osc)
        {
            oscphases[i][osc] += (unisonTunings[osc] * unisonSpreadCoefficient) / index;
            if (oscphases[i][osc] >= 1.0f)
                oscphases[i][osc] -= 1.0f;
            if (oscphases[i][osc] < 0.0f)
                oscphases[i][osc] += 1.0f;

            auto phaseoffset = index - oscphases[i][osc] * index;
            if (osc == 0)
                mixedOsc += buffers[i].readBuffer (phaseoffset) * unisonCentreLevelCoefficient;
            else
                mixedOsc += buffers[i].readBuffer (phaseoffset) * unisonSideLevelCoefficient;
        }
        wet = mixedOsc;

        auto mix = 1.0f;
        mix = clamp (mix, 0.0f, 1.0f);
        float out = crossfade (in, wet, mix);

        out = dcOutFilters[i].process (out);
        out = sspo::voltageSaturate (out);
        lastOut[i] = out;

        TBase::outputs[OUT_OUTPUT].setVoltage (out, i);
    }
    TBase::outputs[OUT_OUTPUT].setChannels (channels);
}

template <class TBase>
int KSDelayDescription<TBase>::getNumParams()
{
    return KSDelayComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config KSDelayDescription<TBase>::getParam (int i)
{
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Code type", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        case KSDelayComp<TBase>::OCTAVE_PARAM:
            ret = { -4.0f, 4.0f, 0.0f, "Octave", " octave", 0.0f, 1.0f, 0.0f };
            break;
        case KSDelayComp<TBase>::TUNE_PARAM:
            ret = { -7.0f, 7.0f, 0.0f, "Tune", " semitones", 0.0f, 1.0f, 0.0f };
            break;
        case KSDelayComp<TBase>::FEEDBACK_PARAM:
            ret = { 0.0f, 0.5f, 0.5f, "Feedback", "%", 0, 100, 0.0f };
            break;
        case KSDelayComp<TBase>::UNISON_PARAM:
            ret = { 1.0f, 7.0f, 1.0f, "Unison count", " ", 0, 1, 0.0f };
            break;
        case KSDelayComp<TBase>::UNISON_SPREAD_PARAM:
            ret = { 0.0f, 1.0f, 0.5f, "Unison Spread", " ", 0, 1, 0.0f };
            break;
        case KSDelayComp<TBase>::UNISON_MIX_PARAM:
            ret = { 0.0f, 1.0f, 1.0f, "Unison Mix", "  ", 0, 1, 0.0f };
            break;
        case KSDelayComp<TBase>::STRETCH_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Stretch", " ", 0, 1, 0.0f };
            break;
        case KSDelayComp<TBase>::STRETCH_LOCK_PARAM:
            ret = { 0.0f, 1.0f, 1.0f, "Stretch Lock", " ", 0, 1, 0.0f };
            break;
        default:
            assert (false);
    }
    return ret;
}
