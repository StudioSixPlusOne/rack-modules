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

#include <algorithm>

#include "AudioMath.h"

using namespace sspo::AudioMath;

struct PolyShiftRegister : Module
{
    enum ParamIds
    {
        CHANNELS_PARAM,
        TRIGGER_PROB_PARAM,
        SHUFFLE_PROB_PARAM,
        ACCENT_A_PROB_PARAM,
        ACCENT_A_OFFSET_PARAM,
        ACCENT_B_PROB_PARAM,
        ACCENT_B_OFFSET_PARAM,
        ACCENT_RNG_PROB_PARAM,
        ACCENT_RNG_OFFSET_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        CHANNELS_INPUT,
        TRIGGER_PROB_INPUT,
        SHUFFLE_PROB_INPUT,
        ACCENT_A_PROB_INPUT,
        ACCENT_A_OFFSET_INPUT,
        ACCENT_B_PROB_INPUT,
        ACCENT_B_OFFSET_INPUT,
        ACCENT_RNG_PROB_INPUT,
        ACCENT_RNG_MAX_INPUT,
        MAIN_INPUT,
        RESET_INPUT,
        TRIGGER_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        MAIN_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    static constexpr int maxChannels = 16;
    std::vector<float> channelData;
    dsp::SchmittTrigger clockTrigger;
    dsp::SchmittTrigger resetTrigger;
    int currentChannels = 1;

    // random number generated per trigger used for accents
    std::vector<float> triggerRng; // random number generated per trigger used for accent RNG
    std::vector<float> accentAOffsets;
    std::vector<float> accentBOffsets;
    std::vector<float> accentRngOffsets;


    PolyShiftRegister()
    {
        config (NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam (CHANNELS_PARAM, 1.0f, 16.0f, 4.0f, "Channels");
        configParam (TRIGGER_PROB_PARAM, 0.0f, 1.0f, 0.0f, "Trigger miss probability");
        configParam (SHUFFLE_PROB_PARAM, 0.0f, 1.0f, 0.0f, "Shuffle probability");
        configParam (ACCENT_A_PROB_PARAM, 0.0f, 1.0f, 0.0f, "Accent A probability");
        configParam (ACCENT_A_OFFSET_PARAM, -10.0f, 10.0f, 0.0f, "Accent A offset");
        configParam (ACCENT_B_PROB_PARAM, 0.0f, 1.0f, 0.0f, "Accent A probability");
        configParam (ACCENT_B_OFFSET_PARAM, -10.0f, 10.0f, 0.0f, "Accent B offset");
        configParam (ACCENT_RNG_PROB_PARAM, 0.0f, 1.0f, 0.0f, "Accent RNG probability");
        configParam (ACCENT_RNG_OFFSET_PARAM, -10.0f, 10.0f, 0.0f, "Accent RNG maximum");
        init();
    }

    void init()
    {
        defaultGenerator.seed (time (NULL));

        channelData.resize (maxChannels);
        for (auto& cd : channelData)
            cd = 0.0f;

        triggerRng.resize (maxChannels);
        for (auto& t : triggerRng)
            t = 0.0f;

        accentAOffsets.resize (maxChannels);
        for (auto& a : accentAOffsets)
            a = 0.0f;

        accentBOffsets.resize (maxChannels);
        for (auto& a : accentBOffsets)
            a = 0.0f;

        accentRngOffsets.resize (maxChannels);
        for (auto& a : accentRngOffsets)
            a = 0.0f;
    }

    void shift (float in)
    {
        for (auto i = maxChannels; i > 0; --i)
            channelData[i] = channelData[i - 1];
        channelData[0] = in;
    }

    void triggerRngGenerator()
    {
        for (auto i = 0; i < currentChannels; ++i)
            triggerRng[i] = rand01();
    }

    void generateAccents()
    {
        if (inputs[ACCENT_A_PROB_INPUT].getChannels() > 1)
        {
            for (auto c = 0; c < currentChannels; ++c)
            {
                accentAOffsets[c] = fixedAccent (ACCENT_A_PROB_PARAM, ACCENT_A_PROB_INPUT, ACCENT_A_OFFSET_PARAM, ACCENT_A_OFFSET_INPUT, c);
            }
        }
        else
        {
            auto accent = fixedAccent (ACCENT_A_PROB_PARAM, ACCENT_A_PROB_INPUT, ACCENT_A_OFFSET_PARAM, ACCENT_A_OFFSET_INPUT, 0);
            for (auto& a : accentAOffsets)
                a = accent;
        }

        if (inputs[ACCENT_B_PROB_INPUT].getChannels() > 1)
        {
            for (auto c = 0; c < currentChannels; ++c)
            {
                accentBOffsets[c] = fixedAccent (ACCENT_B_PROB_PARAM, ACCENT_B_PROB_INPUT, ACCENT_B_OFFSET_PARAM, ACCENT_B_OFFSET_INPUT, c);
            }
        }
        else
        {
            auto accent = fixedAccent (ACCENT_B_PROB_PARAM, ACCENT_B_PROB_INPUT, ACCENT_B_OFFSET_PARAM, ACCENT_B_OFFSET_INPUT, 0);
            for (auto& a : accentBOffsets)
                a = accent;
        }

        if (inputs[ACCENT_RNG_PROB_INPUT].getChannels() > 1)
        {
            for (auto c = 0; c < currentChannels; ++c)
            {
                accentRngOffsets[c] = rand01() * fixedAccent (ACCENT_RNG_PROB_PARAM, ACCENT_RNG_PROB_INPUT, ACCENT_RNG_OFFSET_PARAM, ACCENT_RNG_MAX_INPUT, c);
            }
        }
        else
        {
            auto accent = rand01() * fixedAccent (ACCENT_RNG_PROB_PARAM, ACCENT_RNG_PROB_INPUT, ACCENT_RNG_OFFSET_PARAM, ACCENT_RNG_MAX_INPUT, 0);
            for (auto& a : accentRngOffsets)
                a = accent;
        }
    }

    float fixedAccent (ParamIds probParam, InputIds probInput, ParamIds offsetParam, InputIds offsetInput, int c)
    {
        auto ret = 0.0f;
        auto accentProb = clamp (params[probParam].getValue() + inputs[probInput].getPolyVoltage (c) / 10.0f, 0.0f, 1.0f);
        auto useAccent = accentProb > rand01();
        if (useAccent)
            ret = clamp (params[offsetParam].getValue() + inputs[offsetInput].getPolyVoltage (c), -10.0f, 10.0f);

        return ret;
    }

    void process (const ProcessArgs& args) override
    {
        //channels, trigger and shufffle are monophonic as these all act on a single buffer not per output channel
        auto channels = static_cast<int> (clamp (params[CHANNELS_PARAM].getValue() + inputs[CHANNELS_INPUT].getVoltage(), 1.0f, static_cast<float> (maxChannels)));
        auto triggerProb = clamp (params[TRIGGER_PROB_PARAM].getValue() + inputs[TRIGGER_PROB_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f);
        auto shuffleProb = clamp (params[SHUFFLE_PROB_PARAM].getValue() + inputs[SHUFFLE_PROB_INPUT].getVoltage() / 10.0f, 0.0f, 1.0f);

        auto ignoreTrigger = triggerProb > rand01();
        if (clockTrigger.process (inputs[TRIGGER_INPUT].getVoltage()) && ! ignoreTrigger)
        {
            shift (inputs[MAIN_INPUT].getVoltage());
            currentChannels++;
            currentChannels = clamp (currentChannels, 1, channels);
            triggerRngGenerator();
            auto useShuffle = shuffleProb > rand01();
            if (useShuffle)
                std::random_shuffle (channelData.begin(), channelData.end());

            generateAccents();
        }

        if (resetTrigger.process (inputs[RESET_INPUT].getVoltage()))
            currentChannels = 1;

        //output channels
        for (auto c = 0; c < std::min (currentChannels, channels); ++c)
        {
            auto out = channelData[c];
            out += accentAOffsets[c];
            out += accentBOffsets[c];
            out += accentRngOffsets[c];
            outputs[MAIN_OUTPUT].setVoltage (out, c);
        }

        outputs[MAIN_OUTPUT].setChannels (std::min (currentChannels, channels));
    }
};

struct PolyShiftRegisterWidget : ModuleWidget
{
    PolyShiftRegisterWidget (PolyShiftRegister* module)
    {
        setModule (module);
        setPanel (APP->window->loadSvg (asset::plugin (pluginInstance, "res/PolyShiftRegister.svg")));

        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (createParamCentered<RoundBlackSnapKnob> (mm2px (Vec (30.989, 18.522)), module, PolyShiftRegister::CHANNELS_PARAM));
        addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (19.405, 36.711)), module, PolyShiftRegister::TRIGGER_PROB_PARAM));
        addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (42.764, 36.774)), module, PolyShiftRegister::SHUFFLE_PROB_PARAM));
        addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (19.405, 54.901)), module, PolyShiftRegister::ACCENT_A_PROB_PARAM));
        addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (42.764, 54.964)), module, PolyShiftRegister::ACCENT_A_OFFSET_PARAM));
        addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (19.405, 73.091)), module, PolyShiftRegister::ACCENT_B_PROB_PARAM));
        addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (42.764, 73.154)), module, PolyShiftRegister::ACCENT_B_OFFSET_PARAM));
        addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (19.405, 91.281)), module, PolyShiftRegister::ACCENT_RNG_PROB_PARAM));
        addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (42.764, 91.344)), module, PolyShiftRegister::ACCENT_RNG_OFFSET_PARAM));

        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (19.309, 18.522)), module, PolyShiftRegister::CHANNELS_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (7.725, 36.711)), module, PolyShiftRegister::TRIGGER_PROB_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (31.084, 36.774)), module, PolyShiftRegister::SHUFFLE_PROB_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (7.725, 54.901)), module, PolyShiftRegister::ACCENT_A_PROB_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (31.084, 54.964)), module, PolyShiftRegister::ACCENT_A_OFFSET_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (7.725, 73.091)), module, PolyShiftRegister::ACCENT_B_PROB_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (31.084, 73.154)), module, PolyShiftRegister::ACCENT_B_OFFSET_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (7.725, 91.281)), module, PolyShiftRegister::ACCENT_RNG_PROB_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (31.084, 91.344)), module, PolyShiftRegister::ACCENT_RNG_MAX_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (7.725, 109.802)), module, PolyShiftRegister::MAIN_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (31.084, 109.865)), module, PolyShiftRegister::RESET_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (19.405, 110.008)), module, PolyShiftRegister::TRIGGER_INPUT));

        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (42.764, 109.865)), module, PolyShiftRegister::MAIN_OUTPUT));
    }
};

Model* modelPolyShiftRegister = createModel<PolyShiftRegister, PolyShiftRegisterWidget> ("PolyShiftRegister");