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

#include "CircularBuffer.h"
#include "HardLimiter.h"
#include "LookupTable.h"
#include "ctrl/SqHelper.h"

struct CombFilter : Module
{
    enum ParamIds
    {
        FREQUENCY_PARAM,
        FREQUENCY_CV_ATTENUVERTER_PARAM,
        COMB_CV_ATTENUVERTER_PARAM,
        COMB_PARAM,
        FEEDBACK_CV_ATTENUVERTER_PARAM,
        FEEDBACK_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        VOCT_INPUT,
        FREQ_CV_INPUT,
        COMB_CV_INPUT,
        FEEDBACK_CV_INPUT,
        MAIN_INPUT,
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
    static constexpr float dcOutCutoff = 40.0f;
    float maxFreq = 20000;
    float sampleRate = 1;
    float samplePeriod = 1;

    std::vector<CircularBuffer<float>> buffers;
    sspo::Saturator saturate;
    std::vector<dsp::RCFilter> dcOutFilters;

    CombFilter()
    {
        config (NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam (FREQUENCY_PARAM, -4.0f, 4.0f, 0.0f, "Frequency");
        configParam (FREQUENCY_CV_ATTENUVERTER_PARAM, -1.0f, 1.0f, 0.0f, "Frequency Attenuverter");
        configParam (COMB_CV_ATTENUVERTER_PARAM, -1.0f, 1.0f, 0.0f, "Comb Attenuverter");
        configParam (COMB_PARAM, -1.0f, 1.0f, 0.0f, "Comb");
        configParam (FEEDBACK_CV_ATTENUVERTER_PARAM, -1.0f, 1.0f, 0.0f, "Feedback Attenuverter");
        configParam (FEEDBACK_PARAM, 0.0f, 1.1f, 0.f, "Feedback");

        onSampleRateChange();
        init();
    }

    void init()
    {
        buffers.resize (16);
        for (auto& b : buffers)
            b.reset (4096);

        saturate.max = 0.86f;
        saturate.kneeWidth = 0.005f;

        dcOutFilters.resize (maxChannels);
        for (auto& d : dcOutFilters)
            d.setCutoffFreq (dcOutCutoff / sampleRate);
    }

    void onSampleRateChange() override
    {
        sampleRate = SqHelper::engineGetSampleRate();
        samplePeriod = 1.0f / sampleRate;

        maxFreq = std::min (20000.0f, sampleRate / 2.0f);

        for (auto& d : dcOutFilters)
            d.setCutoffFreq (dcOutCutoff / sampleRate);
    }

    void process (const ProcessArgs& args) override
    {
        auto channels = std::max (1, inputs[MAIN_INPUT].getChannels());
        auto freqParam = params[FREQUENCY_PARAM].getValue();
        auto freqAttenuverterParam = params[FREQUENCY_CV_ATTENUVERTER_PARAM].getValue();
        auto combParam = params[COMB_PARAM].getValue();
        auto combAttenuverterParam = params[COMB_CV_ATTENUVERTER_PARAM].getValue();
        auto feedbackParam = params[FEEDBACK_PARAM].getValue();
        auto feedbackAttenuverterParam = params[FEEDBACK_CV_ATTENUVERTER_PARAM].getValue();

        for (auto c = 0; c < channels; ++c)
        {
            auto in = inputs[MAIN_INPUT].getPolyVoltage (c) / 5.0f;
            //in += 1e-6f * (2.0f * drand48() - 1.0f);

            auto frequency = freqParam;
            frequency += inputs[VOCT_INPUT].getPolyVoltage (c);
            frequency += (inputs[FREQ_CV_INPUT].getPolyVoltage (c) * freqAttenuverterParam);
            frequency = dsp::FREQ_C4 * lookup.pow2 (frequency);
            frequency = clamp (frequency, 0.1f, maxFreq);

            auto feedback = feedbackParam + feedbackAttenuverterParam * (inputs[FEEDBACK_CV_INPUT].getPolyVoltage (c) / 5.0f);
            feedback = clamp (feedback, -0.9f, 0.9f);

            auto comb = combParam + combAttenuverterParam * (inputs[COMB_CV_INPUT].getPolyVoltage (c) / 5.0f);
            comb = clamp (comb, -1.0f, 1.0f);

            auto delayTime = 1.0f / frequency;
            auto index = delayTime * sampleRate;

            in += buffers[c].readBuffer (index) * comb * feedback;
            buffers[c].writeBuffer (in);

            auto out = in + buffers[c].readBuffer (index) * comb;

            dcOutFilters[c].process (out);
            out = dcOutFilters[c].highpass();
            out = saturate.process (out);

            outputs[MAIN_OUTPUT].setVoltage (out * 5.0f, c);
        }
        outputs[MAIN_OUTPUT].setChannels (channels);
    }
};

struct CombFilterWidget : ModuleWidget
{
    CombFilterWidget (CombFilter* module)
    {
        setModule (module);
        setPanel (APP->window->loadSvg (asset::plugin (pluginInstance, "res/CombFilter.svg")));

        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (createParamCentered<RoundLargeBlackKnob> (mm2px (Vec (41.01, 25.312)), module, CombFilter::FREQUENCY_PARAM));
        addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (24.871, 29.546)), module, CombFilter::FREQUENCY_CV_ATTENUVERTER_PARAM));
        addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (25.135, 47.802)), module, CombFilter::COMB_CV_ATTENUVERTER_PARAM));
        addParam (createParamCentered<RoundLargeBlackKnob> (mm2px (Vec (41.01, 47.802)), module, CombFilter::COMB_PARAM));
        addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (25.135, 70.292)), module, CombFilter::FEEDBACK_CV_ATTENUVERTER_PARAM));
        addParam (createParamCentered<RoundLargeBlackKnob> (mm2px (Vec (41.01, 70.292)), module, CombFilter::FEEDBACK_PARAM));

        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 21.344)), module, CombFilter::VOCT_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 29.546)), module, CombFilter::FREQ_CV_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 47.802)), module, CombFilter::COMB_CV_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 70.292)), module, CombFilter::FEEDBACK_CV_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.26, 112.625)), module, CombFilter::MAIN_INPUT));

        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (41.01, 112.625)), module, CombFilter::MAIN_OUTPUT));
    }
};

Model* modelCombFilter = createModel<CombFilter, CombFilterWidget> ("CombFilter");