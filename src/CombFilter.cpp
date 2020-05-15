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
#include "CombFilter.h"
#include "WidgetComposite.h"
#include "ctrl/SqHelper.h"

using Comp = CombFilterComp<WidgetComposite>;

struct CombFilter : Module
{
    std::shared_ptr<Comp> cf;

    CombFilter()
    {
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
        cf = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);

        onSampleRateChange();
        cf->init();
    }

    void onSampleRateChange() override
    {
        float rate = SqHelper::engineGetSampleRate();
        cf->setSampleRate (rate);
    }

    void process (const ProcessArgs& args) override
    {
        cf->step();
    }
};

/*****************************************************
User Interface
*****************************************************/

struct CombFilterWidget : ModuleWidget
{
    CombFilterWidget (CombFilter* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/CombFilter.svg");

        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (SqHelper::createParamCentered<RoundLargeBlackKnob> (icomp, mm2px (Vec (41.01, 25.312)), module, Comp::FREQUENCY_PARAM));
        addParam (SqHelper::createParamCentered<RoundBlackKnob> (icomp, mm2px (Vec (24.871, 29.546)), module, Comp::FREQUENCY_CV_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<RoundBlackKnob> (icomp, mm2px (Vec (25.135, 47.802)), module, Comp::COMB_CV_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<RoundLargeBlackKnob> (icomp, mm2px (Vec (41.01, 47.802)), module, Comp::COMB_PARAM));
        addParam (SqHelper::createParamCentered<RoundBlackKnob> (icomp, mm2px (Vec (25.135, 70.292)), module, Comp::FEEDBACK_CV_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<RoundLargeBlackKnob> (icomp, mm2px (Vec (41.01, 70.292)), module, Comp::FEEDBACK_PARAM));

        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 21.344)), module, Comp::VOCT_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 29.546)), module, Comp::FREQ_CV_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 47.802)), module, Comp::COMB_CV_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 70.292)), module, Comp::FEEDBACK_CV_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 112.625)), module, Comp::MAIN_INPUT));

        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (41.01, 112.625)), module, Comp::MAIN_OUTPUT));
    }
};

Model* modelCombFilter = createModel<CombFilter, CombFilterWidget> ("CombFilter");