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
#include "Farini.h"
#include "WidgetComposite.h"
#include "../ctrl/SqMenuItem.h"
#include "../widgets.h"
#include "AudioMath.h"

using Comp = FariniComp<WidgetComposite>;

struct Farini : Module
{
    std::shared_ptr<Comp> ma;

    Farini()
    {
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
        ma = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);

        configBypass (Comp::LEFT_INPUT, Comp::LEFT_OUTPUT);
        configBypass (Comp::RIGHT_INPUT, Comp::RIGHT_OUTPUT);
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

struct FariniWidget : ModuleWidget
{
    FariniWidget (Farini* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/Farini.svg");

        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (createParam<sspo::ThreeWaySwitch> (mm2px (Vec (0.4, 15.162)), module, Comp::MODE_PARAM));
        addParam (createParam<sspo::TwoWaySwitch> (mm2px (Vec (13.702, 15.162)), module, Comp::RETRIG_PARAM));
        addParam (createParam<sspo::TwoWaySwitch> (mm2px (Vec (26.946, 15.162)), module, Comp::RESET_PARAM));
        addParam (createParam<sspo::TwoWaySwitch> (mm2px (Vec (40.189, 15.162)), module, Comp::CYCLE_PARAM));
        addParam (createParamCentered<sspo::LargeKnob> (mm2px (Vec (30.975, 34.243)), module, Comp::ATTACK_PARAM));
        addParam (createParamCentered<sspo::SmallKnob> (mm2px (Vec (17.434, 34.631)), module, Comp::ATTACK_CV_PARAM));
        addParam (createParamCentered<sspo::LargeKnob> (mm2px (Vec (30.975, 50.471)), module, Comp::DECAY_PARAM));
        addParam (createParamCentered<sspo::SmallKnob> (mm2px (Vec (17.434, 50.859)), module, Comp::DECAY_CV_PARAM));
        addParam (createParamCentered<sspo::LargeKnob> (mm2px (Vec (30.975, 66.699)), module, Comp::SUSTAIN_PARAM));
        addParam (createParamCentered<sspo::SmallKnob> (mm2px (Vec (17.434, 67.087)), module, Comp::SUSTAIN_CV_PARAM));
        addParam (createParamCentered<sspo::LargeKnob> (mm2px (Vec (30.975, 82.927)), module, Comp::RELEASE_PARAM));
        addParam (createParamCentered<sspo::SmallKnob> (mm2px (Vec (17.434, 83.314)), module, Comp::RELEASE_CV_PARAM));

        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (6.065, 34.347)), module, Comp::ATTACK_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (6.065, 50.777)), module, Comp::DECAY_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (6.065, 66.802)), module, Comp::SUSTAIN_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (6.065, 83.03)), module, Comp::RELEASE_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (6.065, 100.211)), module, Comp::LEFT_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (18.941, 100.211)), module, Comp::RIGHT_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (6.065, 113.97)), module, Comp::TRIGGER_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (18.941, 113.97)), module, Comp::GATE_INPUT));

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (44.694, 34.347)), module, Comp::ATTACK_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (44.694, 50.574)), module, Comp::DECAY_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (44.694, 66.802)), module, Comp::SUSTAIN_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (44.694, 83.03)), module, Comp::RELEASE_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (31.818, 100.211)), module, Comp::LEFT_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (44.694, 100.211)), module, Comp::RIGHT_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (31.818, 113.97)), module, Comp::EOC_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (44.694, 113.97)), module, Comp::ENV_OUTPUT));

        if (module)
        {
            module->configInput (Comp::ATTACK_INPUT, "Attack");
            module->configInput (Comp::DECAY_INPUT, "Decay");
            module->configInput (Comp::SUSTAIN_INPUT, "Sustain");
            module->configInput (Comp::RELEASE_INPUT, "Release");
            module->configInput (Comp::LEFT_INPUT, "Left");
            module->configInput (Comp::RIGHT_INPUT, "Right");
            module->configInput (Comp::TRIGGER_INPUT, "Trigger");
            module->configInput (Comp::GATE_INPUT, "Gate");
            module->configOutput (Comp::ATTACK_OUTPUT, "Attack");
            module->configOutput (Comp::DECAY_OUTPUT, "Decay");
            module->configOutput (Comp::SUSTAIN_OUTPUT, "Sustain");
            module->configOutput (Comp::RELEASE_OUTPUT, "Release");
            module->configOutput (Comp::LEFT_OUTPUT, "Left");
            module->configOutput (Comp::RIGHT_OUTPUT, "Right");
            module->configOutput (Comp::EOC_OUTPUT, "EOC");
            module->configOutput (Comp::ENV_OUTPUT, "ENV");
        }
    }

    struct UseWaveShapingMenuItem : MenuItem
    {
        Farini* module;
        void onAction (const event::Action& e) override
        {
            module->params[Comp::USE_NLD_PARAM].setValue (! module->params[Comp::USE_NLD_PARAM].getValue());
        }
    };

    void appendContextMenu (Menu* menu) override;
};

void FariniWidget::appendContextMenu (Menu* menu)
{
    auto* module = dynamic_cast<Farini*> (this->module);

    menu->addChild (new MenuEntry);

    auto useNldMenuItem = new UseWaveShapingMenuItem;
    useNldMenuItem->module = module;
    useNldMenuItem->text = "Waveshaping";
    useNldMenuItem->rightText = CHECKMARK (module->params[Comp::USE_NLD_PARAM].getValue());
    menu->addChild (useNldMenuItem);
}

Model* modelFarini = createModel<Farini, FariniWidget> ("Farini");