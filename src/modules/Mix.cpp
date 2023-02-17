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

#include "../plugin.hpp"
#include "Mix.h"
#include "WidgetComposite.h"
#include "../ctrl/SqMenuItem.h"
#include "../widgets.h"
#include "AudioMath.h"

using Comp = MixComp<WidgetComposite>;

struct Mix : Module
{
    std::shared_ptr<Comp> ma;

    Mix()
    {
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
        ma = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);

        onSampleRateChange();
        ma->init();
    }

    json_t* dataToJson() override
    {
        return ma->dataToJson();
    }

    void dataFromJson (json_t* rootJ) override
    {
        ma->dataFromJson (rootJ);
    }

    void onSampleRateChange() override
    {
        float rate = SqHelper::engineGetSampleRate();
        ma->setSampleRate (rate);
    }

    void process (const ProcessArgs& args) override
    {
        ma->step();
    }
};

/*****************************************************
User Interface
*****************************************************/

struct MixWidget : ModuleWidget
{
    MixWidget (Mix* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/Mix.svg");

        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (7.587, 24.391)), module, Comp::ONE_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (22.883, 24.391)), module, Comp::TWO_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (7.587, 44.183)), module, Comp::THREE_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (22.883, 44.183)), module, Comp::FOUR_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (7.587, 63.975)), module, Comp::FIVE_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (22.883, 63.975)), module, Comp::MAIN_PARAM));

        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.587, 83.096)), module, Comp::ONE_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (22.883, 83.096)), module, Comp::TWO_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.587, 98.134)), module, Comp::THREE_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (22.883, 98.134)), module, Comp::FOUR_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.587, 113.171)), module, Comp::FIVE_INPUT));

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (22.883, 113.171)), module, Comp::MAIN_OUTPUT));

        if (module)
        {
            module->configInput (Comp::ONE_INPUT, "ONE");
            module->configInput (Comp::TWO_INPUT, "TWO");
            module->configInput (Comp::THREE_INPUT, "THREE");
            module->configInput (Comp::FOUR_INPUT, "FOUR");
            module->configInput (Comp::FIVE_INPUT, "FIVE");
            module->configOutput (Comp::MAIN_OUTPUT, "MAIN");
        }
    }
};

Model* modelMix = createModel<Mix, MixWidget> ("Mix");