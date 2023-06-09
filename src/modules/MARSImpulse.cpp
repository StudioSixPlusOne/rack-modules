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
#include "MARSImpulse.h"
#include "WidgetComposite.h"
#include "../ctrl/SqMenuItem.h"
#include "../widgets.h"
#include "AudioMath.h"

using Comp = MARSImpulseComp<WidgetComposite>;

struct MARSImpulse : Module
{
    std::shared_ptr<Comp> ma;

    MARSImpulse()
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

struct MARSImpulseWidget : ModuleWidget
{
    MARSImpulseWidget (MARSImpulse* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/MARSImpulse.svg");

        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (41.01, 47.493)), module, Comp::IMPULSE_LEVEL_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (41.01, 70.247)), module, Comp::TRIGGER_PARAM));

        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (28.707, 70.247)), module, Comp::TRIGGER_INPUT));

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (41.01, 112.625)), module, Comp::MAIN_OUTPUT));

        if (module)
        {
            module->configInput (Comp::TRIGGER_INPUT, "TRIGGER");
            module->configOutput (Comp::MAIN_OUTPUT, "MAIN");
        }
    }
};

Model* modelMARSImpulse = createModel<MARSImpulse, MARSImpulseWidget> ("MARSImpulse");