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
#include "Zazel.h"
#include "WidgetComposite.h"
#include "ctrl/SqMenuItem.h"
#include <atomic>
#include <assert.h>

using Comp = ZazelComp<WidgetComposite>;

struct RequestedParamId
{
    int moduleid = -1;
    int paramid = -1;
};

struct Zazel : Module
{
    std::shared_ptr<Comp> zazel;
    std::atomic<RequestedParamId> requestedParameter;
    ParamHandle paramHandle;

    Zazel()
    {
        assert (std::atomic<RequestedParamId>{}.is_lock_free());
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
        zazel = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);

        //init parameter handle
        paramHandle.color = nvgRGB (0xcd, 0xde, 0x87);
        APP->engine->addParamHandle (&paramHandle);

        //init composite
        onSampleRateChange();
        zazel->init();
    }

    ~Zazel()
    {
        APP->engine->removeParamHandle (&paramHandle);
    }

    void onReset() override
    {
        RequestedParamId rpi;
        rpi.moduleid = -1;
        rpi.paramid = -1;
        requestedParameter.store (rpi);
    }

    json_t* dataToJson() override
    {
        json_t* rootJ = json_object();

        json_object_set_new (rootJ, "moduleId", json_integer (paramHandle.moduleId));
        json_object_set_new (rootJ, "parameterId", json_integer (paramHandle.paramId));

        return rootJ;
    }

    void dataFromJson (json_t* rootJ) override
    {
        json_t* moduleIdJ = json_object_get (rootJ, "moduleId");
        json_t* parameterIdJ = json_object_get (rootJ, "parameterId");
        if (! (moduleIdJ && parameterIdJ))
            return;
        RequestedParamId rpi;
        rpi.moduleid = json_integer_value (moduleIdJ);
        rpi.paramid = json_integer_value (parameterIdJ);
        requestedParameter.store (rpi);
    }

    void onSampleRateChange() override
    {
        float rate = SqHelper::engineGetSampleRate();
        zazel->setSampleRate (rate);
    }

    void paramChange()
    {
        RequestedParamId rpi = requestedParameter.load();
        if (rpi.moduleid == -1)
            return;
        APP->engine->updateParamHandle (&paramHandle, rpi.moduleid, rpi.paramid, true);

        //reset requested parameter so only updates on request.
        rpi.moduleid = -1;
        rpi.moduleid = -1;
        requestedParameter.store (rpi);
    }

    void process (const ProcessArgs& args) override
    {
        paramChange();
        zazel->step();
        //TODO send output to parameter
    }
};

/*****************************************************
User Interface
*****************************************************/

struct ParameterSelectWidget : Widget
{
    Zazel* module = nullptr;
    bool learning = false;

    std::shared_ptr<Font> font;
    NVGcolor txtColor;
    const int fontHeight = 12;

    ParameterSelectWidget()
    {
        box.size = mm2px (Vec (30.408, 14.084));
        font = APP->window->loadFont (asset::system ("res/fonts/ShareTechMono-Regular.ttf"));
        txtColor = nvgRGBA (0xf0, 0xf0, 0xf0, 0xff);
    }

    void setModule (Zazel* module)
    {
        this->module = module;
    }

    void onButton (const event::Button& e) override
    {
        e.stopPropagating();
        if (module == nullptr)
            return;

        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT)
        {
            learning = true;
            e.consume (this);
        }

        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            RequestedParamId rpi;
            rpi.moduleid = -1;
            rpi.paramid = -1;
            module->requestedParameter.store (rpi);
            e.consume (this);
        }
    }

    //if the next object click on is a parameter, use it
    void onDeselect (const event::Deselect& e) override
    {
        if (module == nullptr)
            return;

        ParamWidget* touchedParam = APP->scene->rack->touchedParam;
        if (learning && touchedParam)
        {
            APP->scene->rack->touchedParam = nullptr;
            RequestedParamId rpi;
            rpi.moduleid = touchedParam->paramQuantity->module->id;
            rpi.paramid = touchedParam->paramQuantity->paramId;
            module->requestedParameter.store (rpi);
            learning = false;
        }
        else
            learning = false;
    }

    std::string getSelectedModuleName()
    {
        if (module == nullptr)
            return "";

        if (learning)
            return "learning";

        if (module->paramHandle.moduleId == -1)
            return "Module";
        else
        {
            ModuleWidget* mw = APP->scene->rack->getModule (module->paramHandle.moduleId);
            if (mw == nullptr)
                return "";
            return mw->model->name;
        }
    }

    std::string getSelectedParameterName()
    {
        if (module == nullptr)
            return "";

        if (learning)
            return "learning";

        if (module->paramHandle.moduleId == -1)
            return "Parameter";
        else
        {
            ModuleWidget* mw = APP->scene->rack->getModule (module->paramHandle.moduleId);
            if (mw == nullptr)
                return "";
            Module* m = mw->module;
            if (mw == nullptr)
                return "";
            auto paramId = module->paramHandle.paramId;
            if (paramId >= (int) m->params.size())
                return "";
            ParamQuantity* pq = m->paramQuantities[paramId];
            return pq->label;
        }
    }

    void draw (const DrawArgs& args) override
    {
        nvgFontSize (args.vg, fontHeight);
        nvgFontFaceId (args.vg, font->handle);
        //nvgTextLetterSpacing (args.vg, -2);
        nvgTextAlign (args.vg, NVG_ALIGN_LEFT);
        nvgFillColor (args.vg, txtColor);
        std::string txt;

        //module name text
        Vec c = Vec (5, 15);
        auto moduleTxt = getSelectedModuleName();
        nvgText (args.vg, c.x, c.y, moduleTxt.c_str(), NULL);
        auto parameterTxt = getSelectedParameterName();
        c = Vec (5, 35);
        nvgText (args.vg, c.x, c.y, parameterTxt.c_str(), NULL);
    }
};

struct ZazelWidget : ModuleWidget
{
    ZazelWidget (Zazel* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/Zazel.svg");

        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (SqHelper::createParamCentered<RoundLargeBlackKnob> (icomp, mm2px (Vec (48.161, 58.514)), module, Comp::START_PARAM));
        addParam (SqHelper::createParamCentered<RoundBlackKnob> (icomp, mm2px (Vec (28.925, 40.324)), module, Comp::EASING_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<RoundBlackKnob> (icomp, mm2px (Vec (28.925, 58.514)), module, Comp::START_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<RoundLargeBlackKnob> (icomp, mm2px (Vec (48.161, 40.324)), module, Comp::EASING_PARAM));
        addParam (SqHelper::createParamCentered<RoundBlackKnob> (icomp, mm2px (Vec (28.925, 76.704)), module, Comp::END_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<RoundLargeBlackKnob> (icomp, mm2px (Vec (48.161, 76.704)), module, Comp::END_PARAM));
        addParam (SqHelper::createParamCentered<RoundBlackKnob> (icomp, mm2px (Vec (28.925, 94.894)), module, Comp::DURATION_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<RoundLargeBlackKnob> (icomp, mm2px (Vec (48.161, 94.894)), module, Comp::DURATION_PARAM));
        addParam (SqHelper::createParamCentered<CKSS> (icomp, mm2px (Vec (5.05, 112.575)), module, Comp::ONESHOT_PARAM));

        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.689, 40.324)), module, Comp::EASING_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.689, 58.514)), module, Comp::START_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.689, 76.704)), module, Comp::END_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.689, 94.894)), module, Comp::DURATION_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (40.697, 112.422)), module, Comp::STOP_CONT_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (16.93, 112.575)), module, Comp::CLOCK_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (28.814, 112.575)), module, Comp::START_CONT_INPUT));

        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (52.581, 112.422)), module, Comp::MAIN_OUTPUT));

        auto* paramSelectwidget = createWidget<ParameterSelectWidget> (mm2px (Vec (5.591, 14.19)));
        paramSelectwidget->setModule (module);
        addChild (paramSelectwidget);
        // mm2px(Vec(14.142, 14.084))
        addChild (createWidget<Widget> (mm2px (Vec (40.315, 14.19))));
    }
};

Model* modelZazel = createModel<Zazel, ZazelWidget> ("Zazel");