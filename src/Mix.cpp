#include "plugin.hpp"
using simd::float_4;


struct Mix : Module 
{
	enum ParamIds 
	{
		ATTENUVERTER_PARAM,
		NUM_PARAMS
	};
	enum InputIds 
	{
		ONE_INPUT,
		TWO_INPUT,
		THREE_INPUT,
		FOUR_INPUT,
		FIVE_INPUT,
		SIX_INPUT,
		SEVEN_INPUT,
		EIGHT_INPUT,
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

	Mix() 
	{
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ATTENUVERTER_PARAM, -1.0f, 1.0f, 1.0f, "");
	}

	int maxInputChannels()
	{
		auto ret = 0;
		for (auto i = 0; i < NUM_INPUTS; ++i)
		{
			if (inputs[i].getChannels() > ret)
				ret = inputs[i].getChannels(); 
		}
		return ret;
	}

	void process(const ProcessArgs& args) override 
	{
		auto channels = maxInputChannels();

		for (auto c = 0; c < channels; c += 4)
		{
			float_4 out {};
			for (auto i = 0; i < NUM_INPUTS; ++i)
				out += inputs[i].getPolyVoltageSimd<float_4> (c);
				
			out *= params[ATTENUVERTER_PARAM].getValue();

			//set output
			out.store(outputs[MAIN_OUTPUT].getVoltages(c));
		}

		outputs[MAIN_OUTPUT].setChannels (channels);
	}
};


struct MixWidget : ModuleWidget {
	MixWidget(Mix* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Mix.svg")));


		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.619, 93.398)), module, Mix::ATTENUVERTER_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.624, 14.362)), module, Mix::ONE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.632, 23.549)), module, Mix::TWO_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.614, 32.702)), module, Mix::THREE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.635, 41.903)), module, Mix::FOUR_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.615, 51.119)), module, Mix::FIVE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.617, 60.302)), module, Mix::SIX_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.587, 69.502)), module, Mix::SEVEN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.603, 78.695)), module, Mix::EIGHT_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.619, 108.479)), module, Mix::MAIN_OUTPUT));
	}
};


Model* modelMix = createModel<Mix, MixWidget>("Mix");