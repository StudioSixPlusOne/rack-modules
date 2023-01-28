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
#include "plugin.hpp"
#include "widgets.h"
#include "WidgetComposite.h"
#include "BascomParamEnum.h"
#include <array>
#include "WaveShaper.h"

struct BascomExpander : Module
{
    enum ParamId
    {
        OVERSAMPLE_EXPANDERPARAM,
        DECIMATOR_FILTERS_EXPANDERPARAM,
        PARAM_UPDATE_DIVIDER_EXPANDERPARAM,
        GAIN_A_EXPANDERPARAM,
        GAIN_B_EXPANDERPARAM,
        GAIN_C_EXPANDERPARAM,
        GAIN_D_EXPANDERPARAM,
        NLD_INPUT_EXPANDERPARAM,
        NLD_1_EXPANDERPARAM,
        NLD_2_EXPANDERPARAM,
        NLD_3_EXPANDERPARAM,
        NLD_4_EXPANDERPARAM,
        GAIN_E_EXPANDERPARAM,
        OFFSET_1_EXPANDERPARAM,
        OFFSET_2_EXPANDERPARAM,
        OFFSET_3_EXPANDERPARAM,
        OFFSET_4_EXPANDERPARAM,
        FEEDBACK_PATH_EXPANDERPARAM,
        NLD_FEEDBACK_EXPANDERPARAM,
        PARAMS_LEN
    };
    enum InputId
    {
        INPUTS_LEN
    };
    enum OutputId
    {
        OUTPUTS_LEN
    };
    enum LightId
    {
        LIGHTS_LEN
    };

    std::array<ParamHandle, NUM_PARAMS> paramHandles;

    BascomExpander()
    {
        sspo::AudioMath::WaveShaper::Nld nld;
        config (PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam (OVERSAMPLE_EXPANDERPARAM, 1.0f, 12.0f, 1.0f, "");
        configParam (DECIMATOR_FILTERS_EXPANDERPARAM, 1.0f, 12.0f, 1.0f, "Decimator Filters");
        configParam (PARAM_UPDATE_DIVIDER_EXPANDERPARAM, 1.0f, 128.0f, 1.0f, "Update Divider");

        configParam (GAIN_A_EXPANDERPARAM, -18.f, 18.f, 0.f, "Mix Coeff A");
        configParam (GAIN_B_EXPANDERPARAM, -18.f, 18.f, 0.f, "Mix Coeff B");
        configParam (GAIN_C_EXPANDERPARAM, -18.f, 18.f, 0.f, "Mix Coeff C");
        configParam (GAIN_D_EXPANDERPARAM, -18.f, 18.f, 0.f, "Mix Coeff D");
        configParam (GAIN_E_EXPANDERPARAM, -18.f, 18.f, 0.f, "Mix Coeff E");
        configParam (NLD_INPUT_EXPANDERPARAM, 0.f, float (nld.size()), 0.f, "Input NLD");
        configParam (NLD_1_EXPANDERPARAM, 0.f, float (nld.size()), 0.f, "S1 NLD");
        configParam (NLD_2_EXPANDERPARAM, 0.f, float (nld.size()), 0.f, "S2 NLD");
        configParam (NLD_3_EXPANDERPARAM, 0.f, float (nld.size()), 0.f, "S3 NLD");
        configParam (NLD_4_EXPANDERPARAM, 0.f, float (nld.size()), 0.f, "S4 NLD");
        configParam (OFFSET_1_EXPANDERPARAM, -24.f, 24.f, 0.f, "Fc offset A");
        configParam (OFFSET_2_EXPANDERPARAM, -24.f, 24.f, 0.f, "Fc offset B");
        configParam (OFFSET_3_EXPANDERPARAM, -24.f, 24.f, 0.f, "Fc offset C");
        configParam (OFFSET_4_EXPANDERPARAM, -24.f, 24.f, 0.f, "Fc offset D");
        configParam (FEEDBACK_PATH_EXPANDERPARAM, 0.f, 1, 0.f, "Feedback Path");
        configParam (NLD_FEEDBACK_EXPANDERPARAM, 0.f, float (nld.size()), 0.f, "Feedback NLD");

        //register paramHandles

        //        testHandle.color = nvgRGB (0xcd, 0xde, 0x87);
        //        APP->engine->addParamHandle (&testHandle);

        for (auto& ph : paramHandles)
        {
            ph.color = nvgRGB (0xcd, 0xde, 0x87);
            APP->engine->addParamHandle (&ph);
        }
    }

    void process (const ProcessArgs& args) override
    {
        //check if connected to parent
        auto parentConnected = leftExpander.module
                               && leftExpander.module->model == modelBascom;

        //if new parent connect parameters
        if (parentConnected && ! isConnected)
        {
            APP->engine->updateParamHandle_NoLock (&paramHandles[OVERSAMPLE_PARAM],
                                                   leftExpander.module->id,
                                                   OVERSAMPLE_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[DECIMATOR_FILTERS_PARAM],
                                                   leftExpander.module->id,
                                                   DECIMATOR_FILTERS_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[PARAM_UPDATE_DIVIDER_PARAM],
                                                   leftExpander.module->id,
                                                   PARAM_UPDATE_DIVIDER_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[COEFF_A_PARAM],
                                                   leftExpander.module->id,
                                                   COEFF_A_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[COEFF_B_PARAM],
                                                   leftExpander.module->id,
                                                   COEFF_B_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[COEFF_C_PARAM],
                                                   leftExpander.module->id,
                                                   COEFF_C_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[COEFF_D_PARAM],
                                                   leftExpander.module->id,
                                                   COEFF_D_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[COEFF_E_PARAM],
                                                   leftExpander.module->id,
                                                   COEFF_E_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[FC_OFFSET_1_PARAM],
                                                   leftExpander.module->id,
                                                   FC_OFFSET_1_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[FC_OFFSET_2_PARAM],
                                                   leftExpander.module->id,
                                                   FC_OFFSET_2_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[FC_OFFSET_3_PARAM],
                                                   leftExpander.module->id,
                                                   FC_OFFSET_3_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[FC_OFFSET_4_PARAM],
                                                   leftExpander.module->id,
                                                   FC_OFFSET_4_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[INPUT_NLD_TYPE_PARAM],
                                                   leftExpander.module->id,
                                                   INPUT_NLD_TYPE_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[RESONANCE_NLD_TYPE_PARAM],
                                                   leftExpander.module->id,
                                                   RESONANCE_NLD_TYPE_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[STAGE_1_NLD_TYPE_PARAM],
                                                   leftExpander.module->id,
                                                   STAGE_1_NLD_TYPE_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[STAGE_2_NLD_TYPE_PARAM],
                                                   leftExpander.module->id,
                                                   STAGE_2_NLD_TYPE_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[STAGE_3_NLD_TYPE_PARAM],
                                                   leftExpander.module->id,
                                                   STAGE_3_NLD_TYPE_PARAM,
                                                   true);

            APP->engine->updateParamHandle_NoLock (&paramHandles[STAGE_4_NLD_TYPE_PARAM],
                                                   leftExpander.module->id,
                                                   STAGE_4_NLD_TYPE_PARAM,
                                                   true);

            isConnected = true;
        }

        //if not connected but was connected disconnect

        if (! parentConnected && isConnected)
        {
            for (auto& ph : paramHandles)
            {
                APP->engine->updateParamHandle_NoLock (&ph,
                                                       -1,
                                                       -1,
                                                       true);
            }
            isConnected = false;
        }

        //update paramhandel values

        if (isConnected)
        {
            ParamQuantity* pq = paramHandles[OVERSAMPLE_PARAM].module->paramQuantities[paramHandles[OVERSAMPLE_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[OVERSAMPLE_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[DECIMATOR_FILTERS_PARAM].module->paramQuantities[paramHandles[DECIMATOR_FILTERS_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[DECIMATOR_FILTERS_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[PARAM_UPDATE_DIVIDER_PARAM].module->paramQuantities[paramHandles[PARAM_UPDATE_DIVIDER_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[PARAM_UPDATE_DIVIDER_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[COEFF_A_PARAM].module->paramQuantities[paramHandles[COEFF_A_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[GAIN_A_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[COEFF_B_PARAM].module->paramQuantities[paramHandles[COEFF_B_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[GAIN_B_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[COEFF_C_PARAM].module->paramQuantities[paramHandles[COEFF_C_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[GAIN_C_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[COEFF_D_PARAM].module->paramQuantities[paramHandles[COEFF_D_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[GAIN_D_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[COEFF_E_PARAM].module->paramQuantities[paramHandles[COEFF_E_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[GAIN_E_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[FC_OFFSET_1_PARAM].module->paramQuantities[paramHandles[FC_OFFSET_1_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[OFFSET_1_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[FC_OFFSET_2_PARAM].module->paramQuantities[paramHandles[FC_OFFSET_2_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[OFFSET_2_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[FC_OFFSET_3_PARAM].module->paramQuantities[paramHandles[FC_OFFSET_3_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[OFFSET_3_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[FC_OFFSET_4_PARAM].module->paramQuantities[paramHandles[FC_OFFSET_4_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[OFFSET_4_EXPANDERPARAM].getValue());
            }

            //nld types
            pq = paramHandles[INPUT_NLD_TYPE_PARAM].module->paramQuantities[paramHandles[INPUT_NLD_TYPE_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[NLD_INPUT_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[RESONANCE_NLD_TYPE_PARAM].module->paramQuantities[paramHandles[RESONANCE_NLD_TYPE_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[NLD_FEEDBACK_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[STAGE_1_NLD_TYPE_PARAM].module->paramQuantities[paramHandles[STAGE_1_NLD_TYPE_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[NLD_1_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[STAGE_2_NLD_TYPE_PARAM].module->paramQuantities[paramHandles[STAGE_2_NLD_TYPE_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[NLD_2_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[STAGE_3_NLD_TYPE_PARAM].module->paramQuantities[paramHandles[STAGE_3_NLD_TYPE_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[NLD_3_EXPANDERPARAM].getValue());
            }

            pq = paramHandles[STAGE_4_NLD_TYPE_PARAM].module->paramQuantities[paramHandles[STAGE_4_NLD_TYPE_PARAM].paramId];
            if (pq != nullptr)
            {
                pq->setValue (params[NLD_4_EXPANDERPARAM].getValue());
            }
        }
    }
        bool isConnected{ false };
    };

    struct BascomExpanderWidget : ModuleWidget
    {
        BascomExpanderWidget (BascomExpander* module)
        {
            setModule (module);
            setPanel (createPanel (asset::plugin (pluginInstance, "res/BascomExpander.svg")));

            addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
            addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
            addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (195.86, 18.48)), module, BascomExpander::OVERSAMPLE_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (195.86, 29.53)), module, BascomExpander::DECIMATOR_FILTERS_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (195.86, 41.3)), module, BascomExpander::PARAM_UPDATE_DIVIDER_EXPANDERPARAM));

            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (82.42, 39.28)), module, BascomExpander::GAIN_B_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (107.283, 48.805)), module, BascomExpander::GAIN_C_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (137.966, 58.859)), module, BascomExpander::GAIN_D_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (57.557, 29.225)), module, BascomExpander::GAIN_A_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (27.88, 62.563)), module, BascomExpander::NLD_INPUT_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (50.634, 62.563)), module, BascomExpander::NLD_1_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (74.976, 62.563)), module, BascomExpander::NLD_2_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (100.376, 62.563)), module, BascomExpander::NLD_3_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (125.247, 62.563)), module, BascomExpander::NLD_4_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (158.604, 62.563)), module, BascomExpander::GAIN_E_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (50.634, 85.317)), module, BascomExpander::OFFSET_1_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (74.976, 85.317)), module, BascomExpander::OFFSET_2_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (100.376, 85.317)), module, BascomExpander::OFFSET_3_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (125.247, 85.317)), module, BascomExpander::OFFSET_4_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (148.53, 91.667)), module, BascomExpander::FEEDBACK_PATH_EXPANDERPARAM));
            addParam (createParamCentered<sspo::Knob> (mm2px (Vec (27.88, 103.309)), module, BascomExpander::NLD_FEEDBACK_EXPANDERPARAM));
        }
    };

    Model* modelBascomExpander = createModel<BascomExpander, BascomExpanderWidget> ("BascomExpander");
