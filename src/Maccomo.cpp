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
#include "SynthFilter.h"


struct Maccomo : Module 
{
	enum ParamIds 
	{
		FREQUENCY_PARAM,
		FREQUENCY_CV_ATTENUVERTER_PARAM,
		RESONANCE_CV_ATTENUVERTER_PARAM,
		RESONANCE_PARAM,
		DRIVE_CV_ATTENUVERTER_PARAM,
		DRIVE_PARAM,
		MODE_PARAM,
		NUM_PARAMS
	};

	enum InputIds 
	{
		VOCT_INPUT,
		RESONANCE_CV_INPUT,
		DRIVE_CV_INPUT,
		MODE_CV_INPUT,
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
	static constexpr float maxFreq = 20000.0f;
	static constexpr float maxRes = 10.0f;
	static constexpr float maxDrive = 2.0f;
	static constexpr int maxChannels = 16;
	
	int typeCount = 0;
	std::vector<int> currentTypes;

	std::vector<sspo::MoogLadderFilter> filters;

	Maccomo() 
	{
		typeCount = sspo::MoogLadderFilter::types().size();
		config (NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);

		configParam (FREQUENCY_PARAM, 0.0f, 1.125f, 0.5f, "Frequency", " Hz", std::pow (2, 10.0f), dsp::FREQ_C4 / std::pow (2, 5.f));

		configParam (FREQUENCY_CV_ATTENUVERTER_PARAM, -1.0f, 1.0f, 0.0f, "Frequency CV");
		configParam (RESONANCE_CV_ATTENUVERTER_PARAM, -1.0f, 1.0f, 0.0f, "Resonance CV");
		configParam (RESONANCE_PARAM, 0.0f, maxRes, 0.0f, "Resonance");
		configParam (DRIVE_CV_ATTENUVERTER_PARAM, -1.0f, 1.0f, 0.0f, "Drive CV");
		configParam (DRIVE_PARAM, 0.0f, maxDrive, 0.6f, "Drive");
		configParam (MODE_PARAM, 0.0f, typeCount - 1, 0.0f, "Type");

		currentTypes.resize (maxChannels);
		for (auto& ct : currentTypes)
			ct = 0;

		filters.resize (maxChannels);
		for (auto& f : filters)
		{
			f.setUseNonLinearProcessing (true);
			f.setType (sspo::MoogLadderFilter::types()[0]);
		}
	}

	void process(const ProcessArgs& args) override {

		auto channels = std::max (inputs[MAIN_INPUT].getChannels(), inputs[VOCT_INPUT].getChannels());
		auto freqParam = params[FREQUENCY_PARAM].getValue();
		auto resParam = params[RESONANCE_PARAM].getValue();
		auto driveParam = params[DRIVE_PARAM].getValue();
		auto modeParam = static_cast<int> (params[MODE_PARAM].getValue());
		auto freqAttenuverterParam = params[FREQUENCY_CV_ATTENUVERTER_PARAM].getValue();
		auto resAttenuverterParam = params[RESONANCE_CV_ATTENUVERTER_PARAM].getValue();
		auto driveAttenuverterParam = params[DRIVE_CV_ATTENUVERTER_PARAM].getValue();

		freqParam = freqParam * 10.0f - 5.0f;

		for (auto i = 0; i < channels; ++i)
		{
			auto in = inputs[MAIN_INPUT].getVoltage (i);
			// Add -120dB noise to bootstrap self-oscillation
			in += 1e-6f * (2.f * random::uniform() - 1.f);

			auto frequency = freqParam;
			if (inputs[VOCT_INPUT].isConnected())
				frequency += inputs[VOCT_INPUT].getPolyVoltage (i);
			if (inputs[FREQ_CV_INPUT].isConnected())
				frequency += (inputs[FREQ_CV_INPUT].getPolyVoltage (i) * freqAttenuverterParam); 
			frequency = dsp::FREQ_C4 * std::pow (2.0f, frequency);

			frequency = clamp(frequency, 0.0f, maxFreq);

			auto resonance = resParam;
			resonance += (inputs[RESONANCE_CV_INPUT].getPolyVoltage (i) / 5.0f) * resAttenuverterParam * maxRes;
			resonance = clamp (resonance, 0.0f, maxRes);

			auto drive = driveParam;
			drive += (inputs[DRIVE_CV_INPUT].getPolyVoltage (i) / 5.0f) * driveAttenuverterParam * maxDrive;
			drive = clamp (drive, 0.0f, maxDrive);
			
			if (currentTypes.at (i) != modeParam  + static_cast<int>(inputs[MODE_CV_INPUT].getPolyVoltage (i)))
			{
				currentTypes[i] = clamp (modeParam +  static_cast<int>(inputs[MODE_CV_INPUT].getPolyVoltage (i)), 0, typeCount -1);
				filters[i].setType(sspo::MoogLadderFilter::types() [currentTypes[i]]);
			}

			filters[i].setParameters (frequency
			, resonance
			, drive
			, 0
			, args.sampleRate);

			auto out = filters[i].process (in / 10.0f) * 10.0f;
			outputs[MAIN_OUTPUT].setVoltage (out, i);

		}
		outputs[MAIN_OUTPUT].setChannels (channels);
	}
};


/*****************************************************
User Interface
*****************************************************/

struct RoundSmallBlackSnapKnob : RoundSmallBlackKnob 
{
	RoundSmallBlackSnapKnob() 
	{
		snap = true;
	}
};

struct MaccomoWidget : ModuleWidget
 {
	MaccomoWidget(Maccomo* module) 
	{
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Maccomo.svg")));

		addChild (createWidget<ScrewSilver> (Vec(RACK_GRID_WIDTH, 0)));
		addChild (createWidget<ScrewSilver> (Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild (createWidget<ScrewSilver> (Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild (createWidget<ScrewSilver> (Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam (createParamCentered<RoundLargeBlackKnob> (mm2px (Vec (41.01, 25.14)), module, Maccomo::FREQUENCY_PARAM));
		addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (25.135, 29.10)), module, Maccomo::FREQUENCY_CV_ATTENUVERTER_PARAM));
		addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (25.135, 47.802)), module, Maccomo::RESONANCE_CV_ATTENUVERTER_PARAM));
		addParam (createParamCentered<RoundLargeBlackKnob> (mm2px (Vec(41.01, 47.802)), module, Maccomo::RESONANCE_PARAM));
		addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (25.135, 70.292)), module, Maccomo::DRIVE_CV_ATTENUVERTER_PARAM));
		addParam (createParamCentered<RoundLargeBlackKnob> (mm2px (Vec (41.01, 70.292)), module, Maccomo::DRIVE_PARAM));
		addParam (createParamCentered<RoundSmallBlackSnapKnob> (mm2px (Vec (41.01, 92.781)), module, Maccomo::MODE_PARAM));

		addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 21.344)), module, Maccomo::VOCT_INPUT));
		addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 47.802)), module, Maccomo::RESONANCE_CV_INPUT));
		addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 70.292)), module, Maccomo::DRIVE_CV_INPUT));
		addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 92.781)), module, Maccomo::MODE_CV_INPUT));
		addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 112.625)), module, Maccomo::MAIN_INPUT));
		addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 29.50)), module, Maccomo::FREQ_CV_INPUT));

		addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (41.01, 112.625)), module, Maccomo::MAIN_OUTPUT));
	}
};


Model* modelMaccomo = createModel<Maccomo, MaccomoWidget> ("Maccomo");