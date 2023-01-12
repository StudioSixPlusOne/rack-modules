/*
* Copyright (c) 2023 Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>
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
#include "widgets.h"
#include "WidgetComposite.h"

struct BascomExpander : Module {
	enum ParamId {
		OVERSAMPLE_PARAM,
		GAIN_A_PARAM,
		GAIN_B_PARAM,
		GAIN_C_PARAM,
		GAIN_D_PARAM,
		NLD_INPUT_PARAM,
		NLD_1_PARAM,
		NLD_2_PARAM,
		NLD_3_PARAM,
		NLD_4_PARAM,
		GAIN_E_PARAM,
		OFFSET_1_PARAM,
		OFFSET_2_PARAM,
		OFFSET_3_PARAM,
		OFFSET_4_PARAM,
		FEEDBACK_PATH_PARAM,
		NLD_FEEDBACK_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUTS_LEN
	};
	enum LightId {
		LIGHTS_LEN
	};

	BascomExpander() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(OVERSAMPLE_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GAIN_A_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GAIN_B_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GAIN_C_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GAIN_D_PARAM, 0.f, 1.f, 0.f, "");
		configParam(NLD_INPUT_PARAM, 0.f, 1.f, 0.f, "");
		configParam(NLD_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(NLD_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(NLD_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(NLD_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(GAIN_E_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OFFSET_1_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OFFSET_2_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OFFSET_3_PARAM, 0.f, 1.f, 0.f, "");
		configParam(OFFSET_4_PARAM, 0.f, 1.f, 0.f, "");
		configParam(FEEDBACK_PATH_PARAM, 0.f, 1.f, 0.f, "");
		configParam(NLD_FEEDBACK_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct BascomExpanderWidget : ModuleWidget {
	BascomExpanderWidget(BascomExpander* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/BascomExpander.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(182.317, 18.642)), module, BascomExpander::OVERSAMPLE_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(82.42, 39.28)), module, BascomExpander::GAIN_B_PARAM));
        addParam(createParamCentered<sspo::Knob>(mm2px(Vec(107.283, 48.805)), module, BascomExpander::GAIN_C_PARAM));
        addParam(createParamCentered<sspo::Knob>(mm2px(Vec(137.966, 58.859)), module, BascomExpander::GAIN_D_PARAM));
        addParam(createParamCentered<sspo::Knob>(mm2px(Vec(57.557, 29.225)), module, BascomExpander::GAIN_A_PARAM));
        addParam(createParamCentered<sspo::Knob>(mm2px(Vec(27.88, 62.563)), module, BascomExpander::NLD_INPUT_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(50.634, 62.563)), module, BascomExpander::NLD_1_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(74.976, 62.563)), module, BascomExpander::NLD_2_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(100.376, 62.563)), module, BascomExpander::NLD_3_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(125.247, 62.563)), module, BascomExpander::NLD_4_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(158.604, 62.563)), module, BascomExpander::GAIN_E_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(50.634, 85.317)), module, BascomExpander::OFFSET_1_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(74.976, 85.317)), module, BascomExpander::OFFSET_2_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(100.376, 85.317)), module, BascomExpander::OFFSET_3_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(125.247, 85.317)), module, BascomExpander::OFFSET_4_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(148.53, 91.667)), module, BascomExpander::FEEDBACK_PATH_PARAM));
		addParam(createParamCentered<sspo::Knob>(mm2px(Vec(27.88, 103.309)), module, BascomExpander::NLD_FEEDBACK_PARAM));
	}
};


Model* modelBascomExpander = createModel<BascomExpander, BascomExpanderWidget>("BascomExpander");