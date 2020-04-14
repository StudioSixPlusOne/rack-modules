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

#include "plugin.hpp"
#include "CircularBuffer.h"


struct KSDelay : Module 
{

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

	static constexpr int maxChannels = 16;
	constexpr static float maxCutoff = 20000.0f;

	std::vector<CircularBuffer<float> > buffers;
	std::vector<dsp::BiquadFilter>  lowpassFilters;
	std::vector<float> lastWets;
	std::vector<float> delayTimes; 
	unsigned int frame = 0;
	

	KSDelay() 
	{
		buffers.resize (maxChannels);
		for (auto& b : buffers)
			b.reset (4096);

		lowpassFilters.resize (maxChannels);

		lastWets.resize (maxChannels);
		for (auto& lw : lastWets)
			lw = 0;

		delayTimes.resize (maxChannels);
		for (auto& d :delayTimes)
			d = 0;

		config (NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
		configParam (OCTAVE_PARAM, -4.0f, 4.0f, 0.0f, "Tune", " octave");
		configParam (TUNE_PARAM, -7.0f, 7.0f, 0.0f, "Tune", " semitones");
		configParam (FEEDBACK_PARAM, 0.8f, 1.0f, 0.99f, "Feedback", "%", 0, 100);
		configParam (FILTER_PARAM, 0.0f, 1.125f, 1.125f, "Frequency", " Hz", std::pow (2, 10.f), dsp::FREQ_C4 / std::pow (2, 5.f), -8.1758);
		configParam (MIX_PARAM, 0.0f, 1.0f, 1.0f, "Mix", "%", 0, 100);
	}

	~KSDelay() {
	}

	void process(const ProcessArgs& args) override 
	{
		auto channels = inputs[IN_INPUT].getChannels();
		auto octaveParam = params[OCTAVE_PARAM].getValue();
		auto tuneParam = params[TUNE_PARAM].getValue();
		auto feedbackParam = params[FEEDBACK_PARAM].getValue();
		auto filterParam = (paramQuantities[FILTER_PARAM])->getDisplayValue();
		auto mixParam = params[MIX_PARAM].getValue();

		frame++;
		frame &= 7;

		for (auto i = 0; i < channels; ++i)
		{
			// Get input to delay block
			auto in = inputs[IN_INPUT].getVoltage (i);
			auto feedback = feedbackParam + inputs[FEEDBACK_INPUT].getPolyVoltage (i) / 10.0f;
			feedback = clamp (feedback, 0.0f, 1.0f);
			
			if (frame == 1)
			{
				delayTimes[i] =  1.0f / (dsp::FREQ_C4 * std::pow(2.0f, inputs[VOCT].getPolyVoltage (i) + octaveParam + tuneParam / 12.0f));

				auto color = filterParam;
				if (inputs[FILTER_INPUT].isConnected())
					color += std::pow (2, inputs[FILTER_INPUT].getPolyVoltage (i)) * dsp::FREQ_C4;	
				color = clamp (color, 1.0f, maxCutoff);
				lowpassFilters[i].setParameters (rack::dsp::BiquadFilter::LOWPASS, color / args.sampleRate, 0.707f, 1.0f);
			}

			auto index = delayTimes[i] * args.sampleRate - 1;
			auto wet = buffers[i].readBuffer (index);
			auto dry = in + lastWets[i] * feedback;
			buffers[i].writeBuffer (dry);

			wet = std::abs (wet) > 17.0f ? 0 : wet;
			wet = lowpassFilters[i].process (wet);
			
			lastWets[i] = wet;

			auto mix = mixParam + inputs[MIX_INPUT].getPolyVoltage (i) / 10.0f;
			mix = clamp (mix, 0.0f, 1.0f);
			float out = crossfade (in, wet, mix);
			
			outputs[OUT_OUTPUT].setVoltage (out, i);
		}
		outputs[OUT_OUTPUT].setChannels (channels);
	}
};

/*****************************************************
User Interface
*****************************************************/

struct RoundLargeBlackSnapKnob : RoundLargeBlackKnob 
{
	RoundLargeBlackSnapKnob() 
	{
		snap = true;
	}
};

struct KSDelayWidget : ModuleWidget 
{
	KSDelayWidget(KSDelay* module) 
	{
		setModule (module);
		setPanel (APP->window->loadSvg (asset::plugin (pluginInstance, "res/KSDelay.svg")));

		addChild (createWidget<ScrewSilver> (Vec (15, 0)));
		addChild (createWidget<ScrewSilver> (Vec (box.size.x - 30, 0)));
		addChild (createWidget<ScrewSilver> (Vec (0, 365)));
		addChild (createWidget<ScrewSilver> (Vec (box.size.x - 15, 365)));

		addParam (createParam<RoundLargeBlackSnapKnob> (Vec (67, 57), module, KSDelay::OCTAVE_PARAM));
		addParam (createParam<RoundSmallBlackKnob> (Vec (40, 80), module, KSDelay::TUNE_PARAM));
		addParam (createParam<RoundLargeBlackKnob> (Vec (67, 123), module, KSDelay::FEEDBACK_PARAM));
		addParam (createParam<RoundLargeBlackKnob> (Vec (67, 193), module, KSDelay::FILTER_PARAM));
		addParam (createParam<RoundLargeBlackKnob> (Vec (67, 257), module, KSDelay::MIX_PARAM));

		addInput (createInput<PJ301MPort> (Vec (14, 63), module, KSDelay::VOCT));
		addInput (createInput<PJ301MPort> (Vec (14, 129), module, KSDelay::FEEDBACK_INPUT));
		addInput (createInput<PJ301MPort> (Vec (14, 196), module, KSDelay::FILTER_INPUT));
		addInput (createInput<PJ301MPort> (Vec (14, 263), module, KSDelay::MIX_INPUT));
		addInput (createInput<PJ301MPort> (Vec (14, 320), module, KSDelay::IN_INPUT));
		addOutput (createOutput<PJ301MPort> (Vec (73, 320), module, KSDelay::OUT_OUTPUT));
	}
};


Model* modelKSDelay = createModel<KSDelay, KSDelayWidget> ("KSDelay");
