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
#include "KSDelay.h"
#include "WidgetComposite.h"
#include "ctrl/SqMenuItem.h"
#include "widgets.h"

using Comp = KSDelayComp<WidgetComposite>;

struct KSDelay : Module
{
    std::shared_ptr<Comp> ks;

    KSDelay()
    {
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
        ks = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);

        onSampleRateChange();
        ks->init();
    }

    void onSampleRateChange() override
    {
        float rate = SqHelper::engineGetSampleRate();
        ks->setSampleRate (rate);
    }

    void process (const ProcessArgs& args) override
    {
        ks->step();
    }
};

/*****************************************************
User Interface
*****************************************************/

struct KSDelayWidget : ModuleWidget
{
    KSDelayWidget (KSDelay* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        //setPanel (APP->window->loadSvg (asset::plugin (pluginInstance, "res/KSDelay.svg")));
        box.size = Vec (8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/KSDelay.svg");

        addChild (createWidget<ScrewSilver> (Vec (15, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 30, 0)));
        addChild (createWidget<ScrewSilver> (Vec (15, 365)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 30, 365)));

        addParam (SqHelper::createParam<sspo::LargeSnapKnob> (icomp, Vec (67, 57), module, Comp::OCTAVE_PARAM));
        addParam (SqHelper::createParam<sspo::SmallKnob> (icomp, Vec (40, 80), module, Comp::TUNE_PARAM));
        addParam (SqHelper::createParam<sspo::LargeKnob> (icomp, Vec (67, 123), module, Comp::FEEDBACK_PARAM));
        addParam (SqHelper::createParam<sspo::SmallSnapKnob> (icomp, Vec (14, 193), module, Comp::UNISON_PARAM));
        addParam (SqHelper::createParam<sspo::SmallKnob> (icomp, Vec (50, 193), module, Comp::UNISON_SPREAD_PARAM));
        addParam (SqHelper::createParam<sspo::SmallKnob> (icomp, Vec (87, 193), module, Comp::UNISON_MIX_PARAM));
        addParam (SqHelper::createParam<sspo::LargeKnob> (icomp, Vec (67, 260), module, Comp::STRETCH_PARAM));
        //addParam (SqHelper::createParam<CKSS> (icomp, Vec (37, 260), module, Comp::STRETCH_LOCK_PARAM));

        addInput (createInput<sspo::PJ301MPort> (Vec (14, 63), module, Comp::VOCT));
        addInput (createInput<sspo::PJ301MPort> (Vec (14, 129), module, Comp::FEEDBACK_INPUT));
        addInput (createInput<sspo::PJ301MPort> (Vec (14, 320), module, Comp::IN_INPUT));
        addInput (createInput<sspo::PJ301MPort> (Vec (14, 223), module, Comp::UNISON_INPUT));
        addInput (createInput<sspo::PJ301MPort> (Vec (50, 223), module, Comp::UNISON_SPREAD_INPUT));
        addInput (createInput<sspo::PJ301MPort> (Vec (87, 223), module, Comp::UNISON_MIX_INPUT));
        addInput (createInput<sspo::PJ301MPort> (Vec (14, 266), module, Comp::STRETCH_INPUT));

        addOutput (createOutput<sspo::PJ301MPort> (Vec (73, 320), module, Comp::OUT_OUTPUT));

        if (module)
        {
            module->configInput(Comp::VOCT, "Voct");
            module->configInput(Comp::FEEDBACK_INPUT, "Feedback");
            module->configInput(Comp::IN_INPUT, "Audio");
            module->configInput(Comp::UNISON_INPUT, "Unison Count");
            module->configInput(Comp::UNISON_SPREAD_INPUT, "Unison Spread");
            module->configInput(Comp::UNISON_MIX_INPUT, "Unison Mix");
            module->configInput(Comp::STRETCH_INPUT, "Stretch");

            module->configOutput(Comp::OUT_OUTPUT, "Audio");
        }
    }
};

Model* modelKSDelay = createModel<KSDelay, KSDelayWidget> ("KSDelay");
