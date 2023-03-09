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
#include "easing.h"
#include "WidgetComposite.h"
#include "ctrl/SqMenuItem.h"
#include <atomic>
#include <assert.h>
#include "AudioMath.h"
#include "widgets.h"

using Comp = ZazelComp<WidgetComposite>;

/// The module and parameter to automate.
/// if the values are -1, no update is required
struct RequestedParamId
{
    int64_t moduleid = -1;
    int paramid = -1;
};

/// module Zazel
/// A plugin designed to automate parameters over time
struct Zazel : Module
{
    std::shared_ptr<Comp> zazel;
    /// request parameter from the UI
    RequestedParamId requestedParameter;
    /// Automated Parameter
    ParamHandle paramHandle;
    ///request to clear automated parameter from the UI
    std::atomic<bool> clearParam;
    /// last automated parameter vaule when in learn mode
    float lastEnd = 0;
    /// how long in learn mode since the automated parameter was changed
    int endFrameCounter = 0;

    Zazel()
    {
        //due to change in moduleid from int to int_64 this can no longer fit
        //together with int paramid in a 64 bit struct, to be lock free atomic
        //removing the assert to all ruinng with locks,
        //this may cause the audio thread to lock when selecting the automation
        //parameter

        //  assert (std::atomic<RequestedParamId>{}.is_lock_free());
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
        zazel = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);

        //init parameter handle
        paramHandle.color = nvgRGB (0xcd, 0xde, 0x87);
        APP->engine->addParamHandle (&paramHandle);
        clearParam.store (false);

        //init composite
        onSampleRateChange();
        zazel->init();
    }

    ~Zazel() override
    {
        APP->engine->removeParamHandle (&paramHandle);
    }
    /// Called whe module reset.
    /// Resets the connected parameter
    void onReset() override
    {
        RequestedParamId rpi;
        rpi.moduleid = -1;
        rpi.paramid = -1;
        requestedParameter = rpi;
    }

    json_t* dataToJson() override
    {
        json_t* rootJ = json_object();

        json_object_set_new (rootJ, "moduleId", json_integer (paramHandle.moduleId));
        json_object_set_new (rootJ, "parameterId", json_integer (paramHandle.paramId));
        json_object_set_new (rootJ, "retriggerMode", json_integer (int (zazel->retriggerMode)));
        json_object_set_new (rootJ, "DurationMultiplier", json_real (zazel->durationMultiplier));

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
        //        requestedParameter.store (rpi);
        APP->engine->updateParamHandle (&paramHandle, rpi.moduleid, rpi.paramid, true);

        json_t* retriggerModeJ = json_object_get (rootJ, "retriggerMode");
        auto re = json_integer_value (retriggerModeJ);
        zazel->retriggerMode = Comp::RetriggerMode (re);

        json_t* durationJ = json_object_get (rootJ, "DurationMultiplier");
        zazel->durationMultiplier = json_real_value (durationJ);
    }

    void onSampleRateChange() override
    {
        float rate = SqHelper::engineGetSampleRate();
        zazel->setSampleRate (rate);
    }

    /// Updates the paramHandle and prepares parameters for learning.
    /// Called by the UI widget
    void updateParamHandle()
    {
        RequestedParamId rpi = requestedParameter;
        APP->engine->updateParamHandle (&paramHandle, rpi.moduleid, rpi.paramid, true);

        ParamQuantity* pq = paramHandle.module->paramQuantities[paramHandle.paramId];
        if (pq != nullptr)
        {
            lastEnd = pq->getScaledValue();
            zazel->setStartParamScaled (lastEnd);
            zazel->setEndParamScaled (lastEnd);
        }
    }

    /// Clears selected automation parameter.
    /// Called by th UI widget.
    void removeParam()
    {
        APP->engine->updateParamHandle (&paramHandle, -1, -1, true);
        clearParam.store (false);
        return;
    }

    /// Called every frame to update changed parameters and set start value,
    ///  and learn mode for end values.
    void paramChange()
    {
        RequestedParamId rpi = requestedParameter;
        if (rpi.moduleid != -1)
        // modulated parameter changed
        {
            //reset requested parameter so only updates on request.
            rpi.moduleid = -1;
            rpi.moduleid = -1;
            requestedParameter = rpi;

            //setup parameter learrning
            zazel->changePhase (Comp::Mode::LEARN_END);
            endFrameCounter = 0;
            lastEnd = 0.0f;
        }

        auto newParam = 0.0f;
        if (paramHandle.moduleId != -1 && paramHandle.module != nullptr)
        /// selected automation parameter is present.
        {
            ParamQuantity* pq = paramHandle.module->paramQuantities[paramHandle.paramId];
            if (pq != nullptr)
            {
                newParam = pq->getScaledValue();
            }
        }

        if (zazel->mode == Comp::Mode::LEARN_END && endFrameCounter > zazel->sampleRate)
        /// if learning and the parameter has not be changed for 1 second.
        {
            if (zazel->oneShot)
                zazel->changePhase (Comp::Mode::ONESHOT_DECAY);
            else
                zazel->changePhase (Comp::Mode::CYCLE_DECAY);

            endFrameCounter = 0;
        }
        else if (zazel->mode == Comp::Mode::LEARN_END
                 && (! sspo::AudioMath::areSame (lastEnd, newParam, 0.0001f)))
        /// learning mode, parameter value updated.
        {
            endFrameCounter = 0;
            lastEnd = newParam;
            zazel->setEndParamScaled (newParam);
        }
        else
        {
            endFrameCounter++;
        }
    }

    /// the index of the curve used for automation
    int getEasing()
    {
        return zazel->getCurrentEasing();
    }

    /// true of in oneShote mode, false indicates cycle mode.
    bool getOneShot()
    {
        return zazel->oneShot;
    }

    ///process loop.
    ///Checks for changed parameter, then ZazelComp.step(),
    /// set the modulated parameter
    void process (const ProcessArgs& args) override
    {
        paramChange();

        zazel->step();

        if (! (paramHandle.moduleId == -1
               || paramHandle.module == nullptr
               || zazel->mode == Comp::Mode::LEARN_END))
        {
            ParamQuantity* pq = paramHandle.module->paramQuantities[paramHandle.paramId];
            if (pq != nullptr)
                pq->setScaledValue (zazel->out / 2.0f + 0.5);
        }
    }
};

/*****************************************************
User Interface
*****************************************************/

struct RetriggerMenuItem : MenuItem
{
    Zazel* module;
    Comp::RetriggerMode mode;
    void onAction (const event::Action& e) override
    {
        module->zazel->setRetriggerMode (mode);
    }
};

struct DurationMiltiplierMenuItem : MenuItem
{
    Zazel* module;
    float multiplier;
    void onAction (const event::Action& e) override
    {
        module->zazel->durationMultiplier = multiplier;
    }
};

struct EasingWidget : Widget
{
    Zazel* module = nullptr;
    NVGcolor lineColor;
    Easings::EasingFactory ef;

    EasingWidget()
    {
        box.size = mm2px (Vec (14.142, 14.084));
        lineColor = nvgRGBA (0xf0, 0xf0, 0xf0, 0xff);
    }

    void setModule (Zazel* module)
    {
        this->module = module;
    }

    void draw (const DrawArgs& args) override
    {
        if (module == nullptr)
            return;

        box.size = mm2px (Vec (14.142, 14.084));
        lineColor = nvgRGBA (0xf0, 0xf0, 0xf0, 0xff);

        const auto border = 14.142f * 0.1f; //bordersize in mm
        const auto width = 11.0f;
        const auto permm = width;
        auto easing = ef.getEasingVector().at (module->getEasing());

        nvgBeginPath (args.vg);
        nvgMoveTo (args.vg, mm2px (border), mm2px (border + width));
        for (auto i = 0.0f; i < 1.0f; i += 0.01f)
        {
            auto easingY = easing->easeInOut (i, 0.0f, 1.0f, 1.0f);
            nvgLineTo (args.vg,
                       mm2px (permm * i + border),
                       mm2px (border + width - width * easingY));
        }
        nvgStrokeColor (args.vg, lineColor);
        nvgStrokeWidth (args.vg, 1.5f);
        nvgStroke (args.vg);
    }
};

struct ParameterSelectWidget : Widget
{
    Zazel* module = nullptr;
    bool learning = false;

    std::shared_ptr<Font> font;
    NVGcolor txtColor;
    const int fontHeight = 12;

    ParameterSelectWidget()
    {
        // moved constructor initialization to draw()
        //        box.size = mm2px (Vec (30.408, 14.084));
        //        font = APP->window->loadFont (asset::system ("res/fonts/ShareTechMono-Regular.ttf"));
        //        txtColor = nvgRGBA (0xf0, 0xf0, 0xf0, 0xff);
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
            module->removeParam();
            e.consume (this);
        }

        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            RequestedParamId rpi;
            rpi.moduleid = -1;
            rpi.paramid = -1;
            module->requestedParameter = rpi;
            module->clearParam.store (true);
            module->removeParam();
            e.consume (this);
        }
    }

    //if the next object click on is a parameter, use it
    void onDeselect (const event::Deselect& e) override
    {
        if (module == nullptr)
            return;

        ParamWidget* touchedParam = APP->scene->rack->getTouchedParam(); // touchedParam;
        if (learning && touchedParam)
        {
            APP->scene->rack->setTouchedParam (nullptr);
            RequestedParamId rpi;
            rpi.moduleid = touchedParam->getParamQuantity()->module->id;
            rpi.paramid = touchedParam->getParamQuantity()->paramId;
            module->requestedParameter = rpi;
            module->updateParamHandle();
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
            return pq->getLabel();
        }
    }

    void draw (const DrawArgs& args) override
    {
        box.size = mm2px (Vec (30.408, 14.084));
        font = APP->window->loadFont (asset::system ("res/fonts/ShareTechMono-Regular.ttf"));
        txtColor = nvgRGBA (0xf0, 0xf0, 0xf0, 0xff);

        nvgFontSize (args.vg, fontHeight);
        nvgFontFaceId (args.vg, font->handle);
        //nvgTextLetterSpacing (args.vg, -2);
        nvgTextAlign (args.vg, NVG_ALIGN_LEFT);
        nvgFillColor (args.vg, txtColor);
        std::string txt;

        //module name text
        Vec c = Vec (5, 15);
        auto moduleTxt = getSelectedModuleName();
        moduleTxt.resize (14);
        nvgText (args.vg, c.x, c.y, moduleTxt.c_str(), NULL);
        auto parameterTxt = getSelectedParameterName();
        parameterTxt.resize (14);
        c = Vec (5, 35);
        nvgText (args.vg, c.x, c.y, parameterTxt.c_str(), NULL);
    }
};

struct ZazelButton : app::SvgSwitch
{
    ZazelButton()
    {
        momentary = true;
        addFrame (APP->window->loadSvg (asset::plugin (pluginInstance, "res/ZazelButton.svg")));
        addFrame (APP->window->loadSvg (asset::plugin (pluginInstance, "res/ZazelButton.svg")));
    }
};

struct ZazelTriggerButton : ZazelButton
{
    ZazelTriggerButton()
    {
        momentary = false;
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

        addParam (SqHelper::createParamCentered<sspo::LargeKnob> (icomp, mm2px (Vec (48.161, 58.514)), module, Comp::START_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (28.925, 40.324)), module, Comp::EASING_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (28.925, 58.514)), module, Comp::START_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<sspo::LargeKnob> (icomp, mm2px (Vec (48.161, 40.324)), module, Comp::EASING_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (28.925, 76.704)), module, Comp::END_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<sspo::LargeKnob> (icomp, mm2px (Vec (48.161, 76.704)), module, Comp::END_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (28.925, 94.894)), module, Comp::DURATION_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<sspo::LargeKnob> (icomp, mm2px (Vec (48.161, 94.894)), module, Comp::DURATION_PARAM));
        addParam (SqHelper::createParamCentered<CKSS> (icomp, mm2px (Vec (5.05, 112.575)), module, Comp::ONESHOT_PARAM));
        addParam (SqHelper::createParamCentered<ZazelButton> (icomp, mm2px (Vec (16.93, 115.62)), module, Comp::SYNC_BUTTON_PARAM));
        addParam (SqHelper::createParamCentered<ZazelButton> (icomp, mm2px (Vec (28.814, 115.62)), module, Comp::TRIG_BUTTON_PARAM));
        addParam (SqHelper::createParamCentered<ZazelButton> (icomp, mm2px (Vec (40.697, 115.62)), module, Comp::PAUSE_BUTTON_PARAM));

        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (9.689, 40.324)), module, Comp::EASING_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (9.689, 58.514)), module, Comp::START_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (9.689, 76.704)), module, Comp::END_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (9.689, 94.894)), module, Comp::DURATION_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (40.697, 112.422)), module, Comp::STOP_CONT_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (16.93, 112.575)), module, Comp::CLOCK_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (28.814, 112.575)), module, Comp::START_CONT_INPUT));

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (52.581, 112.422)), module, Comp::MAIN_OUTPUT));
        if (module)
        {
            module->configInput (Comp::EASING_INPUT, "Easing");
            module->configInput (Comp::START_INPUT, "Start");
            module->configInput (Comp::END_INPUT, "End");
            module->configInput (Comp::DURATION_INPUT, "Duration");
            module->configInput (Comp::STOP_CONT_INPUT, "Pause");
            module->configInput (Comp::CLOCK_INPUT, "Clock");
            module->configInput (Comp::STOP_CONT_INPUT, "Start Cont");

            module->configOutput (Comp::MAIN_OUTPUT, "Main out");
        }

        addChild (createLightCentered<SmallLight<RedLight>> (mm2px (Vec (37.52, 108.25)), module, Comp::PAUSE_LIGHT));

        auto* paramSelectwidget = createWidget<ParameterSelectWidget> (mm2px (Vec (5.591, 14.19)));
        paramSelectwidget->setModule (module);
        addChild (paramSelectwidget);

        auto* easingWidget = createWidget<EasingWidget> (mm2px (Vec (40.315, 14.19)));
        easingWidget->setModule (module);

        addChild (easingWidget);
    }

    void appendContextMenu (Menu* menu) override
    {
        auto* module = dynamic_cast<Zazel*> (this->module);

        menu->addChild (new MenuEntry);

        MenuLabel* retriggerLabel = new MenuLabel();
        retriggerLabel->text = "Retrigger mode";
        menu->addChild (retriggerLabel);

        RetriggerMenuItem* restartMenuItem = new RetriggerMenuItem();
        restartMenuItem->mode = Comp::RetriggerMode::RESTART;
        restartMenuItem->text = "Restart";
        restartMenuItem->module = module;
        restartMenuItem->rightText = CHECKMARK (module->zazel->retriggerMode
                                                == restartMenuItem->mode);
        menu->addChild (restartMenuItem);

        RetriggerMenuItem* ignoreMenuItem = new RetriggerMenuItem();
        ignoreMenuItem->mode = Comp::RetriggerMode::IGNORE;
        ignoreMenuItem->text = "Ignore";
        ignoreMenuItem->module = module;
        ignoreMenuItem->rightText = CHECKMARK (module->zazel->retriggerMode
                                               == ignoreMenuItem->mode);
        menu->addChild (ignoreMenuItem);

        //TODO implement after next round of testing
        /*         RetriggerMenuItem* restartFromCurrentMenuItem = new RetriggerMenuItem();
        restartFromCurrentMenuItem->mode = Comp::RetriggerMode::RESTART_FROM_CURRENT;
        restartFromCurrentMenuItem->text = "Restart from current";
        restartFromCurrentMenuItem->module = module;
        restartFromCurrentMenuItem->rightText = CHECKMARK (module->zazel->retriggerMode
                                                == restartFromCurrentMenuItem->mode);
        menu->addChild (restartFromCurrentMenuItem); */

        // Duration multiplier
        menu->addChild (new MenuEntry);

        MenuLabel* durationLabel = new MenuLabel();
        durationLabel->text = "Duration Multiplier";
        menu->addChild (durationLabel);

        DurationMiltiplierMenuItem* duration001MenuItem = new DurationMiltiplierMenuItem();
        duration001MenuItem->multiplier = 0.01f;
        duration001MenuItem->text = "0.01";
        duration001MenuItem->module = module;
        duration001MenuItem->rightText = CHECKMARK (module->zazel->durationMultiplier
                                                    == duration001MenuItem->multiplier);
        menu->addChild (duration001MenuItem);

        DurationMiltiplierMenuItem* duration01MenuItem = new DurationMiltiplierMenuItem();
        duration01MenuItem->multiplier = 0.1f;
        duration01MenuItem->text = "0.1";
        duration01MenuItem->module = module;
        duration01MenuItem->rightText = CHECKMARK (module->zazel->durationMultiplier
                                                   == duration01MenuItem->multiplier);
        menu->addChild (duration01MenuItem);

        DurationMiltiplierMenuItem* duration1MenuItem = new DurationMiltiplierMenuItem();
        duration1MenuItem->multiplier = 1.0f;
        duration1MenuItem->text = "1.0";
        duration1MenuItem->module = module;
        duration1MenuItem->rightText = CHECKMARK (module->zazel->durationMultiplier
                                                  == duration1MenuItem->multiplier);
        menu->addChild (duration1MenuItem);

        DurationMiltiplierMenuItem* duration10MenuItem = new DurationMiltiplierMenuItem();
        duration10MenuItem->multiplier = 10.0f;
        duration10MenuItem->text = "10.0";
        duration10MenuItem->module = module;
        duration10MenuItem->rightText = CHECKMARK (module->zazel->durationMultiplier
                                                   == duration10MenuItem->multiplier);
        menu->addChild (duration10MenuItem);

        DurationMiltiplierMenuItem* duration50MenuItem = new DurationMiltiplierMenuItem();
        duration50MenuItem->multiplier = 50.0f;
        duration50MenuItem->text = "50.0";
        duration50MenuItem->module = module;
        duration50MenuItem->rightText = CHECKMARK (module->zazel->durationMultiplier
                                                   == duration50MenuItem->multiplier);
        menu->addChild (duration50MenuItem);

        DurationMiltiplierMenuItem* duration100MenuItem = new DurationMiltiplierMenuItem();
        duration100MenuItem->multiplier = 100.0f;
        duration100MenuItem->text = "100.0";
        duration100MenuItem->module = module;
        duration100MenuItem->rightText = CHECKMARK (module->zazel->durationMultiplier
                                                    == duration100MenuItem->multiplier);
        menu->addChild (duration100MenuItem);

        DurationMiltiplierMenuItem* duration500MenuItem = new DurationMiltiplierMenuItem();
        duration500MenuItem->multiplier = 500.0f;
        duration500MenuItem->text = "500.0";
        duration500MenuItem->module = module;
        duration500MenuItem->rightText = CHECKMARK (module->zazel->durationMultiplier
                                                    == duration500MenuItem->multiplier);
        menu->addChild (duration500MenuItem);

        DurationMiltiplierMenuItem* duration1000MenuItem = new DurationMiltiplierMenuItem();
        duration1000MenuItem->multiplier = 1000.0f;
        duration1000MenuItem->text = "1000.0";
        duration1000MenuItem->module = module;
        duration1000MenuItem->rightText = CHECKMARK (module->zazel->durationMultiplier
                                                     == duration1000MenuItem->multiplier);
        menu->addChild (duration1000MenuItem);
    }
};

Model* modelZazel = createModel<Zazel, ZazelWidget> ("Zazel");