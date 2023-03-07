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
#include "Duffy.h"
#include "WidgetComposite.h"
#include "../ctrl/SqMenuItem.h"
#include "../widgets.h"
#include "AudioMath.h"

using Comp = DuffyComp<WidgetComposite>;

struct Duffy : Module
{
    std::shared_ptr<Comp> ma;

    Duffy()
    {
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
        ma = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);

        configBypass (Comp::ONE_INPUT, Comp::ONE_OUTPUT);
        configBypass (Comp::TWO_INPUT, Comp::TWO_OUTPUT);
        configBypass (Comp::THREE_INPUT, Comp::THREE_OUTPUT);
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

struct DuffyWidget : ModuleWidget
{
    std::shared_ptr<Font> font;
    NVGcolor txtColor;
    const int fontHeight = 20;

    void draw (const DrawArgs& args) override
    {
        ModuleWidget::draw (args);
        if (module == nullptr)
            return;

        font = APP->window->loadFont (asset::system ("res/fonts/ShareTechMono-Regular.ttf"));
        txtColor = nvgRGBA (0xf0, 0xf0, 0xf0, 0xff);

        nvgFontSize (args.vg, fontHeight);
        nvgFontFaceId (args.vg, font->handle);
        //nvgTextLetterSpacing (args.vg, -2);
        nvgTextAlign (args.vg, NVG_ALIGN_LEFT);
        nvgFillColor (args.vg, txtColor);
        std::string txt = std::to_string (dynamic_cast<Duffy*> (module)->ma->currentTranspose);
        auto c = mm2px (Vec (21.0f, 42.0f));
        nvgText (args.vg, c.x, c.y, txt.c_str(), NULL);
    }

    DuffyWidget (Duffy* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/Duffy.svg");

        addChild (createWidget<ScrewSilver> (Vec (0, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 1 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (7.729, 39.44)), module, Comp::SHIFT_PARAM));

        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.729, 20.96)), module, Comp::DOWN_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (23.232, 21.167)), module, Comp::UP_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (23.232, 55.42)), module, Comp::RESET_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.729, 74.589)), module, Comp::ONE_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.729, 91.257)), module, Comp::TWO_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.729, 107.926)), module, Comp::THREE_INPUT));

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (24.167, 74.589)), module, Comp::ONE_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (24.167, 91.257)), module, Comp::TWO_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (24.167, 107.926)), module, Comp::THREE_OUTPUT));

        // mm2px(Vec(9.866, 8.697))
        addChild (createWidget<Widget> (mm2px (Vec (19.234, 35.158))));

        if (module)
        {
            module->configInput (Comp::DOWN_INPUT, "Down");
            module->configInput (Comp::RESET_INPUT, "Reset");
            module->configInput (Comp::UP_INPUT, "Up");
            module->configInput (Comp::ONE_INPUT, "One");
            module->configInput (Comp::TWO_INPUT, "Two");
            module->configInput (Comp::THREE_INPUT, "Three");
            module->configOutput (Comp::ONE_OUTPUT, "One");
            module->configOutput (Comp::TWO_OUTPUT, "Two");
            module->configOutput (Comp::THREE_OUTPUT, "Three");
        }
    }
};

Model* modelDuffy = createModel<Duffy, DuffyWidget> ("Duffy");