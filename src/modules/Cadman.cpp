/*
 * Copyright (c) 2021 Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>
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
#include "UtilityFilters.h"
#include "CircularBuffer.h"
#include "ClockDuration.h"
#include "ctrl/SqMenuItem.h"
#include <cmath>

struct Cadman : Module
{
    enum ParamIds
    {
        TIME_PARAM,
        IN_A_POSITION_PARAM,
        OUT_A_POSITION_PARAM,
        IN_B_POSITION_PARAM,
        OUT_B_POSITION_PARAM,
        OUTPUT_A_MIX_PARAM,
        OUTPUT_B_MIX_PARAM,
        NODE_1_ATTENUVERTER_PARAM,
        NODE_2_ATTENUVERTER_PARAM,
        SMOOTH_SCALE_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        A_INPUT,
        TIME_INPUT,
        B_INPUT,
        SYNC_INPUT,
        IN_A_POSITION_INPUT,
        OUT_A_POSITION_INPUT,
        IN_B_POSITION_INPUT,
        OUT_B_POSITION_INPUT,
        OUTPUT_A_LEVEL_INPUT,
        OUTPUT_B_LEVEL_INPUT,
        NODE_1_FILTER_INPUT,
        NODE_2_FILTER_INPUT,
        NODE_1_INPUT,
        NODE_2_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        NODE_1_OUTPUT,
        NODE_2_OUTPUT,
        A_OUTPUT,
        B_OUTPUT,
        MIX_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    constexpr static float maxBufferDuration = 10.0f;
    constexpr static float dcCutoffFreq = 5.5f;
    std::vector<WaveGuide<float>> wg;

    class AttenuverterNode : public WaveGuide<float>::Node
    {
    public:
        inline float step (float in) override
        {
            return in * gain;
        }

        void setGainParameter (float gain)
        {
            this->gain = gain;
        }

    private:
        float gain = 1.0;
    };

    /**
     * Node used for patching external damping
     */
    class ExternalNode : public WaveGuide<float>::Node
    {
    public:
        float step (float in) override
        {
            module->outputs[outId].setVoltage (in * 5.0f, channel);
            return module->inputs[inId].getPolyVoltage (channel) / 5.0f;
        }

        void setChannel (int c)
        {
            channel = c;
        }

        void setInOutId (InputIds in, OutputIds out)
        {
            inId = in;
            outId = out;
        }

        void setModule (Cadman* m)
        {
            module = m;
        }

    private:
        int channel = 0;
        OutputIds outId = OutputIds::NODE_1_OUTPUT;
        InputIds inId = InputIds::NODE_1_INPUT;
        Cadman* module;
    };

    std::vector<std::shared_ptr<AttenuverterNode>> firstNodes;
    std::vector<std::shared_ptr<AttenuverterNode>> secondNodes;
    std::vector<sspo::BiQuad<float>> dcBlockersA;
    std::vector<sspo::BiQuad<float>> dcBlockersB;
    std::shared_ptr<ExternalNode> firstExternal;
    std::shared_ptr<ExternalNode> secondExternal;
    sspo::ClockDuration clockDuration;
    std::vector<sspo::AudioMath::Slew<float>> smoothedLengths;
    std::vector<sspo::AudioMath::Slew<float>> smoothedInPosAs;
    std::vector<sspo::AudioMath::Slew<float>> smoothedInPosBs;
    std::vector<sspo::AudioMath::Slew<float>> smoothedOutPosAs;
    std::vector<sspo::AudioMath::Slew<float>> smoothedOutPosBs;

    Cadman()
    {
        config (NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam (TIME_PARAM, 0.f, 4.f, 1.f, "Delay Time");
        configParam (IN_A_POSITION_PARAM, 0.f, 1.f, 0.f, "Input A Position");
        configParam (OUT_A_POSITION_PARAM, 0.f, 1.f, 0.f, "Output A Position");
        configParam (IN_B_POSITION_PARAM, 0.f, 1.f, 0.f, "Input B Position");
        configParam (OUT_B_POSITION_PARAM, 0.f, 1.f, 0.f, "Output B Position");
        configParam (OUTPUT_A_MIX_PARAM, 0.f, 1.f, 1.f, "Output A Mix");
        configParam (OUTPUT_B_MIX_PARAM, 0.f, 1.f, 1.f, "Output B Mix");
        configParam (NODE_1_ATTENUVERTER_PARAM, -1.f, 1.f, 1.f, "Node 1 Feedback");
        configParam (NODE_2_ATTENUVERTER_PARAM, -1.f, 1.f, 1.f, "Node 2 Feedback");
        configParam (SMOOTH_SCALE_PARAM, 0.0001f, 1.0f, 0.001, "Smoothing scale");
        //        wg.setBufferSize(10 * 44100);
        wg.resize (PORT_MAX_CHANNELS);
        firstNodes.resize (PORT_MAX_CHANNELS);
        secondNodes.resize (PORT_MAX_CHANNELS);
        dcBlockersA.resize (PORT_MAX_CHANNELS);
        dcBlockersB.resize (PORT_MAX_CHANNELS);
        smoothedInPosAs.resize (PORT_MAX_CHANNELS);
        smoothedInPosBs.resize (PORT_MAX_CHANNELS);
        smoothedLengths.resize (PORT_MAX_CHANNELS);
        smoothedOutPosAs.resize (PORT_MAX_CHANNELS);
        smoothedOutPosBs.resize (PORT_MAX_CHANNELS);
        for (auto i = 0; i < PORT_MAX_CHANNELS; ++i)
        {
            firstNodes[i] = std::make_shared<AttenuverterNode>();
            wg[i].setNodeFirst (firstNodes[i]);
            secondNodes[i] = std::make_shared<AttenuverterNode>();
            wg[i].setNodeSecond (secondNodes[i]);
        }
        firstExternal = std::make_shared<ExternalNode>();
        secondExternal = std::make_shared<ExternalNode>();
        firstExternal->setInOutId (InputIds::NODE_1_INPUT, OutputIds::NODE_1_OUTPUT);
        firstExternal->setModule (this);
        secondExternal->setInOutId (InputIds::NODE_2_INPUT, OutputIds::NODE_2_OUTPUT);
        secondExternal->setModule (this);
        onSampleRateChange();
    }

    void onSampleRateChange() override
    {
        float rate = SqHelper::engineGetSampleRate();
        for (auto& w : wg)
        {
            w.setBufferSize (rate * maxBufferDuration);
            //            DEBUG ("rate  : %f  BufferSize %d", rate, w.getBufferSize());
        }

        for (auto& dcA : dcBlockersA)
            dcA.setButterworthHp2 (rate, dcCutoffFreq);

        for (auto& dcB : dcBlockersB)
            dcB.setButterworthHp2 (rate, dcCutoffFreq);

        Module::onSampleRateChange();
    }

    void process (const ProcessArgs& args) override
    {
        auto channelCount = std::max (inputs[A_INPUT].getChannels(), 1);
        for (auto c = 0; c < channelCount; ++c)
        {
            smoothedOutPosAs[c].setScale (params[SMOOTH_SCALE_PARAM].getValue());
            smoothedOutPosBs[c].setScale (params[SMOOTH_SCALE_PARAM].getValue());
            smoothedInPosAs[c].setScale (params[SMOOTH_SCALE_PARAM].getValue());
            smoothedInPosBs[c].setScale (params[SMOOTH_SCALE_PARAM].getValue());
            smoothedLengths[c].setScale (params[SMOOTH_SCALE_PARAM].getValue());

            auto nodeFilter1Param = inputs[NODE_1_FILTER_INPUT].isConnected()
                                        ? inputs[NODE_1_FILTER_INPUT].getPolyVoltage (c) * 0.1f
                                        : 1.0f;
            auto nodeFilter2Param = inputs[NODE_2_FILTER_INPUT].isConnected()
                                        ? inputs[NODE_2_FILTER_INPUT].getPolyVoltage (c) * 0.1f
                                        : 1.0f;
            firstNodes[c]->setGainParameter (params[NODE_1_ATTENUVERTER_PARAM].getValue()
                                             * nodeFilter1Param);
            secondNodes[c]->setGainParameter (params[NODE_2_ATTENUVERTER_PARAM].getValue()
                                              * nodeFilter2Param);
            // check if external nodes are connected, and update wg to correct nodes
            if (inputs[NODE_1_INPUT].isConnected())
            {
                wg[c].setNodeFirst (firstExternal);
                firstExternal->setChannel (c);
            }
            else
            {
                wg[c].setNodeFirst (firstNodes[c]);
            }

            if (inputs[NODE_2_INPUT].isConnected())
            {
                wg[c].setNodeSecond (secondExternal);
                secondExternal->setChannel (c);
            }
            else
            {
                wg[c].setNodeSecond (secondNodes[c]);
            }

            wg[c].step();
            //getClockDuration
            auto lastClockDuration = args.sampleRate;
            if (inputs[SYNC_INPUT].isConnected())
            {
                lastClockDuration = clockDuration.process (inputs[SYNC_INPUT].getVoltage());
            }
            auto timeInput = inputs[TIME_INPUT].isConnected()
                                 ? inputs[TIME_INPUT].getPolyVoltage (c)
                                 : 1.0f;

            auto lengthSamples = static_cast<int> (std::abs (params[TIME_PARAM].getValue() * lastClockDuration) * timeInput);

            wg[c].setLength (smoothedLengths[c].process (lengthSamples));
            auto inAPositionInput = inputs[IN_A_POSITION_INPUT].isConnected()
                                        ? inputs[IN_A_POSITION_INPUT].getPolyVoltage (c) * 0.1f
                                        : 1.0f;
            auto inBPositionInput = inputs[IN_B_POSITION_INPUT].isConnected()
                                        ? inputs[IN_B_POSITION_INPUT].getPolyVoltage (c) * 0.1f
                                        : 1.0f;
            wg[c].addBuffer (smoothedInPosAs[c].process (std::abs (params[IN_A_POSITION_PARAM].getValue() * inAPositionInput)), inputs[A_INPUT].getPolyVoltage (c) * 0.2f);
            wg[c].addBuffer (smoothedInPosBs[c].process (std::abs (params[IN_B_POSITION_PARAM].getValue() * inBPositionInput)), inputs[B_INPUT].getPolyVoltage (c) * 0.2f);

            auto outAPositionInput = inputs[OUT_A_POSITION_INPUT].isConnected()
                                         ? inputs[OUT_A_POSITION_INPUT].getPolyVoltage (c) * 0.1f
                                         : 1.0f;
            auto outA = dcBlockersA[c].process (wg[c].readBuffer (clamp (smoothedOutPosAs[c].process ((params[OUT_A_POSITION_PARAM].getValue() * outAPositionInput)), 0.0f, 1.0f)) * 5.0f);
            outputs[A_OUTPUT].setVoltage (outA, c);
            auto outBPositionInput = inputs[OUT_B_POSITION_INPUT].isConnected()
                                         ? inputs[OUT_B_POSITION_INPUT].getPolyVoltage (c) * 0.1f
                                         : 1.0f;
            auto outB = dcBlockersB[c].process (wg[c].readBuffer (clamp ((smoothedInPosBs[c].process (params[OUT_B_POSITION_PARAM].getValue() * outBPositionInput)), 0.0f, 1.0f)) * 5.0f);
            outputs[B_OUTPUT].setVoltage (outB, c);
            auto outALevel = inputs[OUTPUT_A_LEVEL_INPUT].isConnected() == true
                                 ? inputs[OUTPUT_A_LEVEL_INPUT].getPolyVoltage (c) * 0.1f
                                 : 1.0f;
            auto outBLevel = inputs[OUTPUT_B_LEVEL_INPUT].isConnected() == true
                                 ? inputs[OUTPUT_B_LEVEL_INPUT].getPolyVoltage (c) * 0.1f
                                 : 1.0f;
            outALevel *= sspo::AudioMath::logGainFrom01Param (params[OUTPUT_A_MIX_PARAM].getValue());
            outBLevel *= sspo::AudioMath::logGainFrom01Param (params[OUTPUT_B_MIX_PARAM].getValue());

            outputs[MIX_OUTPUT].setVoltage (outA * outALevel + outB * outBLevel, c);
        }
        outputs[A_OUTPUT].setChannels (channelCount);
        outputs[B_OUTPUT].setChannels (channelCount);
        outputs[MIX_OUTPUT].setChannels (channelCount);
    }
};

struct CadmanWidget : ModuleWidget
{
    CadmanWidget (Cadman* module)
    {
        setModule (module);
        setPanel (APP->window->loadSvg (asset::plugin (pluginInstance, "res/Cadman.svg")));

        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (createParamCentered<sspo::LargeKnob> (mm2px (Vec (36.069, 21.232)), module, Cadman::TIME_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (19.405, 39.421)), module, Cadman::IN_A_POSITION_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (52.924, 39.484)), module, Cadman::OUT_A_POSITION_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (19.405, 57.611)), module, Cadman::IN_B_POSITION_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (52.924, 57.674)), module, Cadman::OUT_B_POSITION_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (19.405, 75.801)), module, Cadman::OUTPUT_A_MIX_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (52.924, 75.864)), module, Cadman::OUTPUT_B_MIX_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (19.405, 93.991)), module, Cadman::NODE_1_ATTENUVERTER_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (52.924, 94.054)), module, Cadman::NODE_2_ATTENUVERTER_PARAM));

        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.725, 21.232)), module, Cadman::A_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (24.389, 21.232)), module, Cadman::TIME_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (52.924, 21.232)), module, Cadman::B_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (30.533, 34.72)), module, Cadman::SYNC_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.725, 39.421)), module, Cadman::IN_A_POSITION_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (41.244, 39.484)), module, Cadman::OUT_A_POSITION_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.725, 57.611)), module, Cadman::IN_B_POSITION_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (41.244, 57.674)), module, Cadman::OUT_B_POSITION_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.725, 75.801)), module, Cadman::OUTPUT_A_LEVEL_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (41.244, 75.864)), module, Cadman::OUTPUT_B_LEVEL_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.725, 93.991)), module, Cadman::NODE_1_FILTER_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (41.244, 94.054)), module, Cadman::NODE_2_FILTER_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (13.6, 112.575)), module, Cadman::NODE_1_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (30.533, 112.575)), module, Cadman::NODE_2_INPUT));

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (5.133, 112.575)), module, Cadman::NODE_1_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (22.062, 112.575)), module, Cadman::NODE_2_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (39.0, 112.575)), module, Cadman::A_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (47.467, 112.575)), module, Cadman::B_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (55.933, 112.575)), module, Cadman::MIX_OUTPUT));
    }
};

Model* modelCadman = createModel<Cadman, CadmanWidget> ("Cadman");