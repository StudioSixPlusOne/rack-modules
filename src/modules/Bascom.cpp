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
#include "Bascom.h"
#include "WidgetComposite.h"
#include "ctrl/SqMenuItem.h"
#include "widgets.h"
#include "AudioMath.h"

using Comp = BascomComp<WidgetComposite>;

struct Bascom : Module
{
    std::shared_ptr<Comp> ma;

    Bascom()
    {
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
        ma = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);

        configBypass (Comp::MAIN_INPUT, Comp::MAIN_OUTPUT);
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

struct BascomWidget : ModuleWidget
{
    BascomWidget (Bascom* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/Bascom.svg");

        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (SqHelper::createParamCentered<sspo::LargeKnob> (icomp, mm2px (Vec (41.01, 25.14 + 1.25)), module, Comp::FREQUENCY_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (25.135, 29.10 + 2.5)), module, Comp::FREQUENCY_CV_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (25.135, 47.802 + 2.5)), module, Comp::RESONANCE_CV_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<sspo::LargeKnob> (icomp, mm2px (Vec (41.01, 47.802 + 2.5)), module, Comp::RESONANCE_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (25.135, 70.292 + 2.5)), module, Comp::DRIVE_CV_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<sspo::LargeKnob> (icomp, mm2px (Vec (41.01, 70.292 + 2.5)), module, Comp::DRIVE_PARAM));

        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (25.135, 95.082)), module, Comp::VCA_CV_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<sspo::LargeKnob> (icomp, mm2px (Vec (41.01, 95.082)), module, Comp::VCA_PARAM));

        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (9.26, 21.344)), module, Comp::VOCT_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (9.26, 47.802 + 2.5)), module, Comp::RESONANCE_CV_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (9.26, 70.292 + 2.5)), module, Comp::DRIVE_CV_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (9.26, 112.625)), module, Comp::MAIN_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (19.74, 112.625)), module, Comp::RIGHT_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (9.26, 29.50 + 2.5)), module, Comp::FREQ_CV_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (9.26, 95.082)), module, Comp::VCA_CV_INPUT));

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (30.41, 112.625)), module, Comp::MAIN_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (41.01, 112.625)), module, Comp::RIGHT_OUTPUT));

        if (module)
        {
            module->configInput (Comp::VOCT_INPUT, "Voct");
            module->configInput (Comp::RESONANCE_CV_INPUT, "Resonance");
            module->configInput (Comp::DRIVE_CV_INPUT, "Drive");
            module->configInput (Comp::MAIN_INPUT, "Main");
            module->configOutput (Comp::RIGHT_OUTPUT, "Right");
            module->configInput (Comp::FREQ_CV_INPUT, "Freq");
            module->configInput (Comp::VCA_CV_INPUT, "Gain");

            module->configOutput (Comp::MAIN_OUTPUT, "Main");
            module->configOutput (Comp::MAIN_OUTPUT, "Right");
        }
    }
};

Model* modelBascom = createModel<Bascom, BascomWidget> ("Bascom");