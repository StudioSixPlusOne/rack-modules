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
#include "Chaplin.h"
#include "WidgetComposite.h"
#include "../ctrl/SqMenuItem.h"
#include "../widgets.h"
#include "AudioMath.h"

using Comp = ChaplinComp<WidgetComposite>;

struct Chaplin : Module
{
    std::shared_ptr<Comp> ma;

    Chaplin()
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

struct ChaplinWidget : ModuleWidget
{
    ChaplinWidget (Chaplin* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/Chaplin.svg");

        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (createParam<sspo::TwoWaySwitch> (mm2px (Vec (21.311, 13.993)), module, Comp::LOOP_PARAM));
        addParam (createParam<sspo::FourWaySwitch> (mm2px (Vec (37.266 - 1.5, 11.493 - 1.5)), module, Comp::MODE_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (23.096, 33.103)), module, Comp::DRY_WET_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (51.452, 33.103)), module, Comp::FEEDBACK_PARAM));
        addParam (createParamCentered<sspo::SmallKnob> (mm2px (Vec (23.096, 49.691)), module, Comp::DELAY_LEFT_PARAM));
        addParam (createParamCentered<sspo::ExtraLargeKnob> (mm2px (Vec (39.024, 57.805)), module, Comp::DELAY_PARAM));
        addParam (createParamCentered<sspo::SmallKnob> (mm2px (Vec (23.096, 65.919)), module, Comp::DELAY_RIGHT_PARAM));

        addParam (createParamCentered<sspo::SmallKnob> (mm2px (Vec (23.096, 82.016)), module, Comp::FILTER_LEFT_PARAM));
        addParam (createParamCentered<sspo::LargeKnob> (mm2px (Vec (39.024, 90.016)), module, Comp::FC_PARAM));
        addParam (createParamCentered<sspo::SmallKnob> (mm2px (Vec (23.096, 98.243)), module, Comp::FILTER_RIGHT_PARAM));

        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (10.669, 16.656)), module, Comp::LOOP_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (10.669, 33.103)), module, Comp::DRY_WET_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (39.024, 33.103)), module, Comp::FEEDBACK_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (10.669, 49.407)), module, Comp::DELAY_LEFT_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (51.452, 49.407)), module, Comp::CLOCK_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (10.669, 65.634)), module, Comp::DELAY_RIGHT_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (10.669, 81.731)), module, Comp::FILTER_LEFT_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (10.669, 97.959)), module, Comp::FILTER_RIGHT_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (10.669, 112.023)), module, Comp::LEFT_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (23.096, 112.023)), module, Comp::RIGHT_INPUT));

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (39.024, 112.023)), module, Comp::LEFT_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (51.452, 112.023)), module, Comp::RIGHT_OUTPUT));

        if (module)
        {
            module->configInput (Comp::LOOP_INPUT, "LOOP");
            module->configInput (Comp::DRY_WET_INPUT, "DRY / WET");
            module->configInput (Comp::FEEDBACK_INPUT, "FEEDBACK");
            module->configInput (Comp::DELAY_LEFT_INPUT, "DELAY_LEFT");
            module->configInput (Comp::CLOCK_INPUT, "CLOCK");
            module->configInput (Comp::DELAY_RIGHT_INPUT, "DELAY_RIGHT");
            module->configInput (Comp::FILTER_LEFT_INPUT, "FILTER_LEFT");
            module->configInput (Comp::FILTER_RIGHT_INPUT, "FILTER_RIGHT");
            module->configInput (Comp::LEFT_INPUT, "LEFT");
            module->configInput (Comp::RIGHT_INPUT, "RIGHT");
            module->configOutput (Comp::LEFT_OUTPUT, "LEFT");
            module->configOutput (Comp::RIGHT_OUTPUT, "RIGHT");
        }
    }

    struct TimeModeMenuItem : MenuItem
    {
        Chaplin* module;
        Comp::TimeMode timeMode;

        void onAction (const event::Action& e) override
        {
            module->params[Comp::TIMEMODE_PARAM].setValue (timeMode);
        }
    };

    void appendContextMenu (Menu* menu) override;
};

void ChaplinWidget::appendContextMenu (Menu* menu)
{
    // add timeMode selectiopn menu items
    auto* module = dynamic_cast<Chaplin*> (this->module);

    menu->addChild (new MenuEntry);

    MenuLabel* timeModeLabel = new MenuLabel();
    timeModeLabel->text = "Timing mode";
    menu->addChild (timeModeLabel);

    TimeModeMenuItem* timeModeDefaultMenuItem = new TimeModeMenuItem;
    timeModeDefaultMenuItem->timeMode = Comp::TimeMode::DEFAULT_TIMEMODE;
    timeModeDefaultMenuItem->text = "Default";
    timeModeDefaultMenuItem->module = module;
    timeModeDefaultMenuItem->rightText = CHECKMARK (module->params[Comp::TIMEMODE_PARAM].getValue() == Comp::DEFAULT_TIMEMODE);
    menu->addChild (timeModeDefaultMenuItem);

    TimeModeMenuItem* timeModeVoctMenuItem = new TimeModeMenuItem;
    timeModeVoctMenuItem->timeMode = Comp::TimeMode::VOCT_TIMEMODE;
    timeModeVoctMenuItem->text = "Voct";
    timeModeVoctMenuItem->module = module;
    timeModeVoctMenuItem->rightText = CHECKMARK (module->params[Comp::TIMEMODE_PARAM].getValue() == Comp::VOCT_TIMEMODE);
    menu->addChild (timeModeVoctMenuItem);

    TimeModeMenuItem* timeModeClockMenuItem = new TimeModeMenuItem;
    timeModeClockMenuItem->timeMode = Comp::TimeMode::CLOCK_TIMEMODE;
    timeModeClockMenuItem->text = "Clock";
    timeModeClockMenuItem->module = module;
    timeModeClockMenuItem->rightText = CHECKMARK (module->params[Comp::TIMEMODE_PARAM].getValue() == Comp::CLOCK_TIMEMODE);
    menu->addChild (timeModeClockMenuItem);
}

Model* modelChaplin = createModel<Chaplin, ChaplinWidget> ("Chaplin");