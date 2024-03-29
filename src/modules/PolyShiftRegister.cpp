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
#include "PolyShiftRegister.h"
#include "WidgetComposite.h"
#include "ctrl/SqMenuItem.h"
#include "widgets.h"

using Comp = PolyShiftRegisterComp<WidgetComposite>;

struct PolyShiftRegister : Module
{
    std::shared_ptr<Comp> psr;

    PolyShiftRegister()
    {
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
        psr = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);
        psr->init();
        rightExpander.producerMessage = &psr->producerM;
        rightExpander.consumerMessage = &psr->consumerM;
    }

    void process (const ProcessArgs& args) override
    {
        //update pointer to expander module data every frame
        bool isExpander = rightExpander.module
                          && rightExpander.module->model == modelTe;

        if (isExpander)
            psr->expMessage = (sspo::TyrantExpanderBuffer*) rightExpander.producerMessage;
        else
            psr->expMessage = &psr->producerM;

        psr->resetExpanderMessage();
        psr->step();

        //swap expander buffers
        if (isExpander)
            rightExpander.messageFlipRequested = true;
    }
};

struct PolyShiftRegisterWidget : ModuleWidget
{
    PolyShiftRegisterWidget (PolyShiftRegister* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/PolyShiftRegister.svg");

        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (SqHelper::createParamCentered<sspo::SnapKnob> (icomp, mm2px (Vec (30.989 + 5.08, 18.522 + 2.71)), module, Comp::CHANNELS_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (19.405, 36.711 + 2.71)), module, Comp::TRIGGER_PROB_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (42.764 + 10.16, 36.774 + 2.71)), module, Comp::SHUFFLE_PROB_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (19.405, 54.901 + 2.71)), module, Comp::ACCENT_A_PROB_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (42.764 + 10.16, 54.964 + 2.71)), module, Comp::ACCENT_A_OFFSET_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (19.405, 73.091 + 2.71)), module, Comp::ACCENT_B_PROB_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (42.764 + 10.16, 73.154 + 2.71)), module, Comp::ACCENT_B_OFFSET_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (19.405, 91.281 + 2.71)), module, Comp::ACCENT_RNG_PROB_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (42.764 + 10.16, 91.344 + 2.71)), module, Comp::ACCENT_RNG_OFFSET_PARAM));

        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (19.309 + 5.08, 18.522 + 2.71)), module, Comp::CHANNELS_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.725, 36.711 + 2.71)), module, Comp::TRIGGER_PROB_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (31.084 + 10.16, 36.774 + 2.71)), module, Comp::SHUFFLE_PROB_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.725, 54.901 + 2.71)), module, Comp::ACCENT_A_PROB_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (31.084 + 10.16, 54.964 + 2.71)), module, Comp::ACCENT_A_OFFSET_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.725, 73.091 + 2.71)), module, Comp::ACCENT_B_PROB_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (31.084 + 10.16, 73.154 + 2.71)), module, Comp::ACCENT_B_OFFSET_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.725, 91.281 + 2.71)), module, Comp::ACCENT_RNG_PROB_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (31.084 + 10.16, 91.344 + 2.71)), module, Comp::ACCENT_RNG_MAX_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.725, 109.802 + 2.71)), module, Comp::MAIN_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (37.86, 109.865 + 2.71)), module, Comp::RESET_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (22.79, 110.008 + 2.71)), module, Comp::TRIGGER_INPUT));

        if (module)
        {
            module->configInput (Comp::CHANNELS_INPUT, "Channels");
            module->configInput (Comp::TRIGGER_PROB_INPUT, "Trigger Prob");
            module->configInput (Comp::SHUFFLE_PROB_INPUT, "Shuffle");
            module->configInput (Comp::ACCENT_A_PROB_INPUT, "Accent A Prob");
            module->configInput (Comp::ACCENT_A_OFFSET_INPUT, "Accent A Offset");
            module->configInput (Comp::ACCENT_B_PROB_INPUT, "Accent B Prob");
            module->configInput (Comp::ACCENT_B_OFFSET_INPUT, "Accent B Offset");
            module->configInput (Comp::ACCENT_RNG_PROB_INPUT, "Accent Rng Prob");
            module->configInput (Comp::ACCENT_RNG_MAX_INPUT, "Accent Rng Max");
            module->configInput (Comp::MAIN_INPUT, "In");
            module->configInput (Comp::RESET_INPUT, "Reset");
            module->configInput (Comp::TRIGGER_INPUT, "Trigger");
        }

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (42.764 + 10.16, 109.865 + 2.71)), module, Comp::MAIN_OUTPUT));
        if (module)
        {
            module->configOutput (Comp::MAIN_OUTPUT, "Main");
        }
    }
};

Model* modelPolyShiftRegister = createModel<PolyShiftRegister, PolyShiftRegisterWidget> ("PolyShiftRegister");
