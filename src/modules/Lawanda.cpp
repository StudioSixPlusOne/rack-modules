#include "plugin.hpp"
#include "widgets.h"



struct Lawanda : Module {
	enum ParamIds {
		LENGTH_L_PARAM,
		LENGTH_PARAM,
		LENGTH_R_PARAM,
		STIFFNESS_N1_L_PARAM,
		STIFFNESS_N1_PARAM,
		STIFFNESS_N1_R_PARAM,
		STIFFNESS_N2_L_PARAM,
		STIFFNESS_N2_PARAM,
		STIFFNESS_N2_R_PARAM,
		IN_POS_L_PARAM,
		IN_POS_PARAM,
		IN_POS_R_PARAM,
		OUT_POS_L_PARAM,
		OUT_POS_PARAM,
		OUT_POS_R_PARAM,
		UNISON_COUNT_L_PARAM,
		UNISON_COUNT_PARAM,
		UNISON_COUNT_R_PARAM,
		UNISON_SPREAD_L_PARAM,
		UNISON_SPREAD_PARAM,
		UNISON_SPREAD_R_PARAM,
		UNISON_MIX_L_PARAM,
		UNISON_MIX_PARAM,
		UNISON_MIX_R_PARAM,
		FREEZE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		LENGTH_L_INPUT,
		LENGTH_R_INPUT,
		STIFFNESS_N1_L_INPUT,
		STIFFNESS_N1_R_INPUT,
		STIFFNESS_N2_L_INPUT,
		STIFFNESS_N2_R_INPUT,
		IN_POS_L_INPUT,
		IN_POS_R_INPUT,
		OUT_POS_L_INPUT,
		OUT_POS_R_INPUT,
		UNISON_COUNT_L_INPUT,
		UNISON_COUNT_R_INPUT,
		UNISON_SPREAD_L_INPUT,
		UNISON_SPREAD_R_INPUT,
		UNISON_MIX_L_INPUT,
		UNISON_MIX_R_INPUT,
		L_INPUT,
		R_INPUT,
		FREEZE_INPUT,
		SYNC_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		L_OUTPUT,
		R_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		FREEZE_LIGHT,
		NUM_LIGHTS
	};

	Lawanda() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(LENGTH_L_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LENGTH_PARAM, 0.f, 1.f, 0.f, "");
		configParam(LENGTH_R_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STIFFNESS_N1_L_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STIFFNESS_N1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STIFFNESS_N1_R_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STIFFNESS_N2_L_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STIFFNESS_N2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(STIFFNESS_N2_R_PARAM, 0.f, 1.f, 0.f, "");
		configParam(IN_POS_L_PARAM, 0.f, 1.f, 0.f, "");
		configParam(IN_POS_PARAM, 0.f, 1.f, 0.f, "");
		configParam(IN_POS_R_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OUT_POS_L_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OUT_POS_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OUT_POS_R_PARAM, 0.f, 1.f, 0.f, "");
		configParam(UNISON_COUNT_L_PARAM, 0.f, 1.f, 0.f, "");
		configParam(UNISON_COUNT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(UNISON_COUNT_R_PARAM, 0.f, 1.f, 0.f, "");
		configParam(UNISON_SPREAD_L_PARAM, 0.f, 1.f, 0.f, "");
		configParam(UNISON_SPREAD_PARAM, 0.f, 1.f, 0.f, "");
		configParam(UNISON_SPREAD_R_PARAM, 0.f, 1.f, 0.f, "");
		configParam(UNISON_MIX_L_PARAM, 0.f, 1.f, 0.f, "");
		configParam(UNISON_MIX_PARAM, 0.f, 1.f, 0.f, "");
		configParam(UNISON_MIX_R_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FREEZE_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct LawandaWidget : ModuleWidget {
	LawandaWidget(Lawanda* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Lawanda.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(45.91, 19.277)), module, Lawanda::LENGTH_L_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(58.128, 19.277)), module, Lawanda::LENGTH_PARAM));
		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(70.346, 19.277)), module, Lawanda::LENGTH_R_PARAM));
		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(45.889, 29.314)), module, Lawanda::STIFFNESS_N1_L_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(58.107, 29.314)), module, Lawanda::STIFFNESS_N1_PARAM));
		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(70.325, 29.314)), module, Lawanda::STIFFNESS_N1_R_PARAM));
		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(45.889, 39.332)), module, Lawanda::STIFFNESS_N2_L_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(58.107, 39.332)), module, Lawanda::STIFFNESS_N2_PARAM));
		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(70.325, 39.332)), module, Lawanda::STIFFNESS_N2_R_PARAM));
		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(46.648, 53.092)), module, Lawanda::IN_POS_L_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(58.866, 53.092)), module, Lawanda::IN_POS_PARAM));
		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(71.084, 53.092)), module, Lawanda::IN_POS_R_PARAM));
		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(46.648, 63.129)), module, Lawanda::OUT_POS_L_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(58.866, 63.129)), module, Lawanda::OUT_POS_PARAM));
		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(71.084, 63.129)), module, Lawanda::OUT_POS_R_PARAM));
		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(45.889, 77.399)), module, Lawanda::UNISON_COUNT_L_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(58.107, 77.399)), module, Lawanda::UNISON_COUNT_PARAM));
		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(70.325, 77.399)), module, Lawanda::UNISON_COUNT_R_PARAM));
		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(45.889, 87.436)), module, Lawanda::UNISON_SPREAD_L_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(58.107, 87.436)), module, Lawanda::UNISON_SPREAD_PARAM));
		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(70.325, 87.436)), module, Lawanda::UNISON_SPREAD_R_PARAM));
		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(45.889, 97.473)), module, Lawanda::UNISON_MIX_L_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(58.107, 97.473)), module, Lawanda::UNISON_MIX_PARAM));
		addParam(createParamCentered<sspo::SmallKnob>(mm2px(Vec(70.325, 97.473)), module, Lawanda::UNISON_MIX_R_PARAM));
		addParam(createParamCentered<LEDButton>(mm2px(Vec(46.76, 112.422)), module, Lawanda::FREEZE_PARAM));

		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(33.692, 19.277)), module, Lawanda::LENGTH_L_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(82.565, 19.277)), module, Lawanda::LENGTH_R_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(33.671, 29.314)), module, Lawanda::STIFFNESS_N1_L_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(82.543, 29.314)), module, Lawanda::STIFFNESS_N1_R_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(33.671, 39.351)), module, Lawanda::STIFFNESS_N2_L_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(82.543, 39.351)), module, Lawanda::STIFFNESS_N2_R_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(34.43, 53.092)), module, Lawanda::IN_POS_L_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(83.302, 53.092)), module, Lawanda::IN_POS_R_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(34.453, 63.024)), module, Lawanda::OUT_POS_L_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(83.326, 63.024)), module, Lawanda::OUT_POS_R_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(33.671, 77.399)), module, Lawanda::UNISON_COUNT_L_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(82.543, 77.399)), module, Lawanda::UNISON_COUNT_R_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(33.671, 87.436)), module, Lawanda::UNISON_SPREAD_L_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(82.543, 87.436)), module, Lawanda::UNISON_SPREAD_R_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(33.671, 97.473)), module, Lawanda::UNISON_MIX_L_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(82.543, 97.473)), module, Lawanda::UNISON_MIX_R_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(11.305, 112.422)), module, Lawanda::L_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(21.889, 112.422)), module, Lawanda::R_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(36.176, 112.422)), module, Lawanda::FREEZE_INPUT));
		addInput(createInputCentered<sspo::PJ301MPort>(mm2px(Vec(62.106, 112.422)), module, Lawanda::SYNC_INPUT));

		addOutput(createOutputCentered<sspo::PJ301MPort>(mm2px(Vec(79.039, 112.422)), module, Lawanda::L_OUTPUT));
		addOutput(createOutputCentered<sspo::PJ301MPort>(mm2px(Vec(89.622, 112.422)), module, Lawanda::R_OUTPUT));

		addChild(createLightCentered<LargeLight<GreenLight>>(mm2px(Vec(46.76, 112.422)), module, Lawanda::FREEZE_LIGHT));
	}
};


Model* modelLawanda = createModel<Lawanda, LawandaWidget>("Lawanda");