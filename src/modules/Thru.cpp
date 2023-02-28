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
#include "Thru.h"
#include "WidgetComposite.h"
#include "../ctrl/SqMenuItem.h"
#include "../widgets.h"
#include "AudioMath.h"

using Comp = ThruComp<WidgetComposite>;

struct Thru : Module
{
    std::shared_ptr<Comp> ma;

    Thru()
    {
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
        ma = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);

        configBypass (Comp::ONE_INPUT, Comp::ONE_OUTPUT);
        configBypass (Comp::TWO_INPUT, Comp::TWO_OUTPUT);
        configBypass (Comp::THREE_INPUT, Comp::THREE_OUTPUT);
        configBypass (Comp::FOUR_INPUT, Comp::FOUR_OUTPUT);
        configBypass (Comp::FIVE_INPUT, Comp::FIVE_OUTPUT);
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

struct LabelTextField : LedDisplayTextField
{
    Thru* module = nullptr;
    int index = 0;

    LabelTextField()
    {
        fontPath = asset::system ("res/fonts/ShareTechMono-Regular.ttf");
        textOffset = math::Vec (0, 0);
        color = sspo::ledDisplayGreen();
        bgColor = nvgRGB (0x00, 0x00, 0x00);
    }

    void step() override
    {
        LedDisplayTextField::step();
        if (module && module->ma->dirtyLabels[index])
        {
            setText (module->ma->lables[index]);
            module->ma->dirtyLabels[index] = false;
        }
        if (module == nullptr)
        {
            //no module, so browser display
            setText ("User Label");
        }
    }

    void onChange (const ChangeEvent& e) override
    {
        if (module)
            module->ma->lables[index] = getText();
    }
};

struct LabelDisplay : LedDisplay
{
    int index = 0;
    void setModule (Thru* module)
    {
        auto* textField = createWidget<LabelTextField> (Vec (0, -6));
        textField->box.size = box.size;
        textField->multiline = false;
        textField->module = module;
        textField->index = index;
        addChild (textField);
    }

    void draw (const DrawArgs& args) override
    {
        //enpty draw function, so as to not draw borders

        nvgScissor (args.vg, RECT_ARGS (args.clipBox));
        Widget::draw (args);
        nvgResetScissor (args.vg);
    }
};

struct ThruWidget : ModuleWidget
{
    ThruWidget (Thru* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (10 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/Thru.svg");

        addChild (createWidget<ScrewSilver> (Vec (0, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 1 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.587, 26.388)), module, Comp::ONE_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.587, 47.555)), module, Comp::TWO_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.587, 68.721)), module, Comp::THREE_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.587, 89.888)), module, Comp::FOUR_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.587, 111.055)), module, Comp::FIVE_INPUT));

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (22.883, 26.388)), module, Comp::ONE_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (22.883, 47.555)), module, Comp::TWO_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (22.883, 68.721)), module, Comp::THREE_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (22.883, 89.888)), module, Comp::FOUR_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (22.883, 111.055)), module, Comp::FIVE_OUTPUT));

        auto* label0 = createWidget<LabelDisplay> (mm2px (Vec (2.29, 16.789)));
        label0->box.size = mm2px (Vec (25.89, 5.636));
        label0->index = 0;
        label0->setModule (module);
        addChild (label0);

        auto* label1 = createWidget<LabelDisplay> (mm2px (Vec (2.29, 37.956)));
        label1->box.size = mm2px (Vec (25.89, 5.636));
        label1->index = 1;
        label1->setModule (module);
        addChild (label1);

        auto* label2 = createWidget<LabelDisplay> (mm2px (Vec (2.29, 59.122)));
        label2->box.size = mm2px (Vec (25.89, 5.636));
        label2->index = 2;
        label2->setModule (module);
        addChild (label2);

        auto* label3 = createWidget<LabelDisplay> (mm2px (Vec (2.29, 80.289)));
        label3->box.size = mm2px (Vec (25.89, 5.636));
        label3->index = 3;
        label3->setModule (module);
        addChild (label3);

        auto* label4 = createWidget<LabelDisplay> (mm2px (Vec (2.29, 101.456)));
        label4->box.size = mm2px (Vec (25.89, 5.636));
        label4->index = 4;
        label4->setModule (module);
        addChild (label4);

        if (module)
        {
            module->configInput (Comp::ONE_INPUT, "ONE");
            module->configInput (Comp::TWO_INPUT, "TWO");
            module->configInput (Comp::THREE_INPUT, "THREE");
            module->configInput (Comp::FOUR_INPUT, "FOUR");
            module->configInput (Comp::FIVE_INPUT, "FIVE");
            module->configOutput (Comp::ONE_OUTPUT, "ONE");
            module->configOutput (Comp::TWO_OUTPUT, "TWO");
            module->configOutput (Comp::THREE_OUTPUT, "THREE");
            module->configOutput (Comp::FOUR_OUTPUT, "FOUR");
            module->configOutput (Comp::FIVE_OUTPUT, "FIVE");
        }
    }
};

Model* modelThru = createModel<Thru, ThruWidget> ("Thru");