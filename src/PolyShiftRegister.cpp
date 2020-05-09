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

struct PolyShiftRegister : Module
{
    enum ParamIds
    {
        CHANNELS1_PARAM,
        CHANNELS2_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        TRIGGER1_INPUT,
        IN1_INPUT,
        TRIGGER2_INPUT,
        IN2_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        OUT1_OUTPUT,
        OUT2_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    dsp::SchmittTrigger trigger1;
    dsp::SchmittTrigger trigger2;
    std::vector<float> channel1Values;
    std::vector<float> channel2Values;
    static constexpr int maxChannels = 16;

    PolyShiftRegister()
    {
        config (NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam (CHANNELS1_PARAM, 1.0f, 16.0f, 16.0f, "Channels");
        configParam (CHANNELS2_PARAM, 1.0f, 16.0f, 16.0f, "Channels");

        init();
    }

    void init()
    {
        channel1Values.resize (maxChannels);
        for (auto& c1 : channel1Values)
            c1 = 0.0f;

        channel2Values.resize (maxChannels);
        for (auto& c2 : channel2Values)
            c2 = 0.0f;
    }

    void doShift (const float in, std::vector<float>& values)
    {
        for (auto i = maxChannels - 1; i > 0; --i)
            values[i] = values[i - 1];
        values[0] = in;
    }

    void process (const ProcessArgs& args) override
    {
        if (trigger1.process (inputs[TRIGGER1_INPUT].getVoltage()))
            doShift (inputs[IN1_INPUT].getVoltageSum(), channel1Values);
        if (trigger2.process (inputs[TRIGGER2_INPUT].getVoltage()))
            doShift (inputs[IN2_INPUT].getVoltageSum(), channel2Values);

        outputs[OUT1_OUTPUT].setChannels (params[CHANNELS1_PARAM].getValue());
        outputs[OUT1_OUTPUT].writeVoltages (channel1Values.data());

        outputs[OUT2_OUTPUT].setChannels (params[CHANNELS2_PARAM].getValue());
        outputs[OUT2_OUTPUT].writeVoltages (channel2Values.data());
    }
};

struct PolyShiftRegisterWidget : ModuleWidget
{
    struct RoundSmallBlackSnapKnob : RoundSmallBlackKnob
    {
        RoundSmallBlackSnapKnob()
        {
            snap = true;
        }
    };

    PolyShiftRegisterWidget (PolyShiftRegister* module)
    {
        setModule (module);
        setPanel (APP->window->loadSvg (asset::plugin (pluginInstance, "res/PolyShiftRegister.svg")));

        addParam (createParamCentered<RoundSmallBlackSnapKnob> (mm2px (Vec (7.937, 56.885)), module, PolyShiftRegister::CHANNELS1_PARAM));
        addParam (createParamCentered<RoundSmallBlackSnapKnob> (mm2px (Vec (8.108, 112.935)), module, PolyShiftRegister::CHANNELS2_PARAM));

        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (7.865, 21.12)), module, PolyShiftRegister::TRIGGER1_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (7.937, 33.073)), module, PolyShiftRegister::IN1_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (8.036, 77.169)), module, PolyShiftRegister::TRIGGER2_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (8.108, 89.122)), module, PolyShiftRegister::IN2_INPUT));

        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (7.937, 44.979)), module, PolyShiftRegister::OUT1_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (8.108, 101.028)), module, PolyShiftRegister::OUT2_OUTPUT));
    }
};

Model* modelPolyShiftRegister = createModel<PolyShiftRegister, PolyShiftRegisterWidget> ("PolyShiftRegister");