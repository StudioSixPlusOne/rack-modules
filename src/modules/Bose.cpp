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
#include "Bose.h"
#include "WidgetComposite.h"
#include "../ctrl/SqMenuItem.h"
#include "../widgets.h"
#include "AudioMath.h"

using Comp = BoseComp<WidgetComposite>;

struct Bose : Module
{
    std::shared_ptr<Comp> ma;

    Bose()
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

struct BoseWidget : ModuleWidget
{
    BoseWidget (Bose* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/Bose.svg");

        addChild (createWidget<ScrewSilver> (Vec (0, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 1 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (createParam<sspo::TwoWaySwitch> (mm2px (Vec (19.241, 13.265)), module, Comp::POLAR_PARAM));
        addParam (createParam<sspo::TwoWaySwitch> (mm2px (Vec (19.241, 19.829)), module, Comp::DROOP_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (7.729, 41.251)), module, Comp::SCALE_ONE_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (7.729, 57.92)), module, Comp::SCALE_TWO_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (7.729, 74.589)), module, Comp::SCALE_THREE_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (7.729, 91.257)), module, Comp::SCALE_FOUR_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (7.729, 107.926)), module, Comp::SCALE_FIVE_PARAM));

        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.729, 20.96)), module, Comp::TRIGGER_INPUT));

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (24.167, 41.251)), module, Comp::ONE_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (24.167, 57.92)), module, Comp::TWO_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (24.167, 74.589)), module, Comp::THREE_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (24.167, 91.257)), module, Comp::FOUR_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (24.167, 107.926)), module, Comp::FIVE_OUTPUT));

        if (module)
        {
            module->configInput (Comp::TRIGGER_INPUT, "Trigger");
            module->configOutput (Comp::ONE_OUTPUT, "One");
            module->configOutput (Comp::TWO_OUTPUT, "Two");
            module->configOutput (Comp::THREE_OUTPUT, "Three");
            module->configOutput (Comp::FOUR_OUTPUT, "Four");
            module->configOutput (Comp::FIVE_OUTPUT, "Five");
        }
    }
};

Model* modelBose = createModel<Bose, BoseWidget> ("Bose");