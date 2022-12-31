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
#include "Eva.h"
#include "WidgetComposite.h"
#include "ctrl/SqMenuItem.h"
#include "widgets.h"

using Comp = EvaComp<WidgetComposite>;

struct Eva : Module
{
    std::shared_ptr<Comp> eva;

    Eva()
    {
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);

        eva = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);
    }

    void process (const ProcessArgs& args) override
    {
        eva->step();
    }
};

/*****************************************************
User
*****************************************************/

struct MixWidget : ModuleWidget
{
    MixWidget (Eva* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/Eva.svg");

        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (7.619, 87.69)), module, Comp::ATTENUVERTER_PARAM));

        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.624, 17.85)), module, Comp::ONE_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.632, 26.32)), module, Comp::TWO_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.614, 34.78)), module, Comp::THREE_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.635, 43.25)), module, Comp::FOUR_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.635, 51.72)), module, Comp::FIVE_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.635, 60.18)), module, Comp::SIX_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.635, 68.65)), module, Comp::SEVEN_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.635, 77.12)), module, Comp::EIGHT_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.635, 98.16)), module, Comp::ATTENUATION_CV));

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (7.619, 112.58)), module, Comp::MAIN_OUTPUT));

        if (module)
        {
            module->configInput(Comp::MAIN_OUTPUT, "Attenuverter");

            module->configOutput(Comp::MAIN_OUTPUT, "Main");
        }
    }
};

Model* modelEva = createModel<Eva, MixWidget> ("Eva");