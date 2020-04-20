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

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;

template <class TBase>
class KSDelayDescription : public IComposite
{
public:
    Config getParam(int i) override;
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
    KSDelayComp(Module * module) : TBase(module)
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

    void setSampleRate(float rate)
    {
        reciprocalSampleRate = 1 / rate;
        sampleRate = rate;

        for (auto &dc : dcFilters)
			dc.setParameters( rack::dsp::BiquadFilter::HIGHPASS, dcFilterCutoff / rate, 0.141f, 1.0f);
    }

    // must be called after setSampleRate
    void init()
    {
		buffers.resize (maxChannels);
		for (auto& b : buffers)
			b.reset (4096);

		lowpassFilters.resize (maxChannels);

		dcFilters.resize (maxChannels);
		for (auto &dc : dcFilters)
			dc.setParameters( rack::dsp::BiquadFilter::HIGHPASS, dcFilterCutoff / sampleRate, 0.141f, 1.0f);

		lastWets.resize (maxChannels);
		for (auto& lw : lastWets)
			lw = 0;

		delayTimes.resize (maxChannels);
		for (auto& d :delayTimes)
			d = 0;
    }

    // Define all the enums here. This will let the tests and the widget access them.
	
    enum ParamIds 
	{
		OCTAVE_PARAM,
		TUNE_PARAM,
		FEEDBACK_PARAM,
		FILTER_PARAM,
		MIX_PARAM,
		NUM_PARAMS
	};

	enum InputIds 
	{
		VOCT,
		FILTER_INPUT,
		FEEDBACK_INPUT,
		MIX_INPUT,
		IN_INPUT,
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

    typedef float T;        // use floats for all signals

	constexpr static int maxChannels = 16;
	constexpr static float maxCutoff = 20000.0f;
	constexpr static float dcFilterCutoff = 5.5f;

	std::vector<CircularBuffer<float> > buffers;
	std::vector<rack::dsp::BiquadFilter>  lowpassFilters;
	std::vector<rack::dsp::BiquadFilter> dcFilters;
	std::vector<float> lastWets;
	std::vector<float> delayTimes; 
	unsigned int frame = 0;

private:


    float reciprocalSampleRate = 1.0f;
    float sampleRate = 1.0f;
};

template <class TBase>
inline void KSDelayComp<TBase>::step()
{
    auto channels = TBase::inputs[IN_INPUT].getChannels();
	auto octaveParam = TBase::params[OCTAVE_PARAM].getValue();
	auto tuneParam = TBase::params[TUNE_PARAM].getValue();
	auto feedbackParam = TBase::params[FEEDBACK_PARAM].getValue();
	auto filterParam = (TBase::paramQuantities[FILTER_PARAM])->getDisplayValue();
	auto mixParam = TBase::params[MIX_PARAM].getValue();

	frame++;
	frame &= 7;

	for (auto i = 0; i < channels; ++i)
	{
		// Get input to delay block
		auto in = TBase::inputs[IN_INPUT].getVoltage (i);
		in = dcFilters[i].process (in);
		auto feedback = feedbackParam + TBase::inputs[FEEDBACK_INPUT].getPolyVoltage (i) / 10.0f;
		feedback = clamp (feedback, 0.0f, 1.0f);
			
		delayTimes[i] =  1.0f / (dsp::FREQ_C4 * std::pow(2.0f, TBase::inputs[VOCT].getPolyVoltage (i) + octaveParam + tuneParam / 12.0f));

		auto color = filterParam;
		if (TBase::inputs[FILTER_INPUT].isConnected())
			color += std::pow (2.0f, TBase::inputs[FILTER_INPUT].getPolyVoltage (i)) * dsp::FREQ_C4;	
		color = clamp (color, 1.0f, std::min( maxCutoff, sampleRate * 0.5f));
		lowpassFilters[i].setParameters (rack::dsp::BiquadFilter::LOWPASS, color / sampleRate, 0.707f, 1.0f);
			
		auto index = delayTimes[i] * sampleRate - 1;
		auto wet = buffers[i].readBuffer (index);
		auto dry = in + lastWets[i] * feedback;
		buffers[i].writeBuffer (dry);

		wet = std::abs (wet) > 17.0f ? 0 : wet;
		wet = lowpassFilters[i].process (wet);
			
		lastWets[i] = wet;

		auto mix = mixParam + TBase::inputs[MIX_INPUT].getPolyVoltage (i) / 10.0f;
		mix = clamp (mix, 0.0f, 1.0f);
		float out = crossfade (in, wet, mix);
			
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
IComposite::Config KSDelayDescription<TBase>::getParam(int i)
{
   
    IComposite::Config ret = {0.0f, 1.0f, 0.0f, "Code type", "unit", 0.0f, 1.0f, 0.0f};
    switch (i) {
        case KSDelayComp<TBase>::OCTAVE_PARAM:      
            ret = {-4.0f, 4.0f, 0.0f, "Octave", " octave", 0.0f, 1.0f, 0.0f};
            break;
        case KSDelayComp<TBase>::TUNE_PARAM:      
            ret = {-7.0f, 7.0f, 0.0f, "Tune", " semitones", 0.0f, 1.0f, 0.0f};
            break;
        case KSDelayComp<TBase>::FEEDBACK_PARAM:      
            ret = {0.8f, 1.0f, 0.99f, "Feedback", "%", 0, 100, 0.0f};
            break;
        case KSDelayComp<TBase>::FILTER_PARAM:      
            ret = {0.0f, 1.125f, 1.125f, "Frequency", " Hz", std::pow (2, 10.f), dsp::FREQ_C4 / std::pow (2, 5.f), 0.0f};
            break;
        case KSDelayComp<TBase>::MIX_PARAM:      
            ret = {0.0f, 1.0f, 1.0f, "Mix", "%", 0, 100, 0.0f};
            break;
        default:
            assert(false);
    }
    return ret;

}

