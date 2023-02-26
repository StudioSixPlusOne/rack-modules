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
#include "LalaStereo.h"
#include "WidgetComposite.h"
#include "../ctrl/SqMenuItem.h"
#include "../widgets.h"
#include "AudioMath.h"

using Comp = LalaStereoComp<WidgetComposite>;

struct LalaStereo : Module
{
    std::shared_ptr<Comp> lalaStereo;

    LalaStereo()
    {
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
        lalaStereo = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);

        configBypass (Comp::LEFT_INPUT, Comp::LEFT_HIGH_OUTPUT);
        configBypass (Comp::RIGHT_INPUT, Comp::RIGHT_HIGH_OUTPUT);
        onSampleRateChange();
        lalaStereo->init();
    }

    json_t* dataToJson() override
    {
        return lalaStereo->dataToJson();
    }

    void dataFromJson (json_t* rootJ) override
    {
        lalaStereo->dataFromJson (rootJ);
    }

    void onSampleRateChange() override
    {
        float rate = SqHelper::engineGetSampleRate();
        lalaStereo->setSampleRate (rate);
    }

    void process (const ProcessArgs& args) override
    {
        lalaStereo->step();
    }
};

/*****************************************************
User Interface
*****************************************************/

struct LalaStereoWidget : ModuleWidget
{
    LalaStereoWidget (LalaStereo* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/LalaStereo.svg");

        addChild (createWidget<ScrewSilver> (Vec (0, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 1 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (createParamCentered<sspo::LargeKnob> (mm2px (Vec (15.198, 28.962)), module, Comp::FREQ_PARAM));
        addParam (createParamCentered<sspo::LargeKnob> (mm2px (Vec (15.198, 41.774)), module, Comp::FREQ_CV_PARAM));

        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (15.198, 52.668)), module, Comp::FREQ_CV_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.62, 69.806)), module, Comp::LEFT_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (22.966, 69.806)), module, Comp::RIGHT_INPUT));

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (7.62, 87.289)), module, Comp::LEFT_HIGH_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (22.966, 87.289)), module, Comp::RIGHT_HIGH_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (7.62, 105.809)), module, Comp::LEFT_LOW_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (22.966, 105.809)), module, Comp::RIGHT_LOW_OUTPUT));

        if (module)
        {
            module->configInput (Comp::FREQ_CV_INPUT, "FREQ_CV");
            module->configInput (Comp::LEFT_INPUT, "LEFT");
            module->configInput (Comp::RIGHT_INPUT, "RIGHT");
            module->configOutput (Comp::LEFT_HIGH_OUTPUT, "LEFT_HIGH");
            module->configOutput (Comp::RIGHT_HIGH_OUTPUT, "RIGHT_HIGH");
            module->configOutput (Comp::LEFT_LOW_OUTPUT, "LEFT_LOW");
            module->configOutput (Comp::RIGHT_LOW_OUTPUT, "RIGHT_LOW");
        }
    }
};

Model* modelLalaStereo = createModel<LalaStereo, LalaStereoWidget> ("LalaStereo");