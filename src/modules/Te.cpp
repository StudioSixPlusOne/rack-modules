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
#include "PolyShiftRegister.h"
#include "widgets.h"
#include "WidgetComposite.h"

struct Te : Module
{
    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        NUM_INPUTS
    };
    enum OutputIds
    {
        TRIG_OUTPUT,
        SHUFFLE_OUTPUT,
        A_OUTPUT,
        B_OUTPUT,
        RNG_OUTPUT,
        RESET_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    static const int maxChannels = 16;
    //pulse generators per channel
    std::vector<dsp::PulseGenerator> trigGenerator;
    std::vector<dsp::PulseGenerator> shuffleGenerator;
    std::vector<dsp::PulseGenerator> aGenerator;
    std::vector<dsp::PulseGenerator> bGenerator;
    std::vector<dsp::PulseGenerator> rngGenerator;
    std::vector<dsp::PulseGenerator> resetGenerator;
    sspo::TyrantExpanderBuffer* buffer;
    sspo::TyrantExpanderBuffer tempBuffer;

    Te()
    {
        config (NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        trigGenerator.resize (maxChannels);
        shuffleGenerator.resize (maxChannels);
        aGenerator.resize (maxChannels);
        bGenerator.resize (maxChannels);
        rngGenerator.resize (maxChannels);
        resetGenerator.resize (maxChannels);
        buffer = &tempBuffer;
    }

    void process (const ProcessArgs& args) override
    {
        auto parentConnected = leftExpander.module
                               && leftExpander.module->model == modelPolyShiftRegister;
        if (parentConnected)
        {
            buffer = static_cast<sspo::TyrantExpanderBuffer*> (leftExpander.module->rightExpander.consumerMessage);
            {
                for (auto c = 0; c < maxChannels; ++c)
                {
                    if (buffer->triggerAccent[c])
                        trigGenerator[c].trigger();
                    if (buffer->shuffleAccent[c])
                        shuffleGenerator[c].trigger();
                    if (buffer->aAccent[c])
                        aGenerator[c].trigger();
                    if (buffer->bAccent[c])
                        bGenerator[c].trigger();
                    if (buffer->rngAccent[c])
                        rngGenerator[c].trigger();
                    if (buffer->reset[c])
                        resetGenerator[c].trigger();

                    outputs[TRIG_OUTPUT].setVoltage ((float) trigGenerator[c].process (args.sampleTime) * 10.0f, c);
                    outputs[SHUFFLE_OUTPUT].setVoltage ((float) shuffleGenerator[c].process (args.sampleTime) * 10.0f, c);
                    outputs[A_OUTPUT].setVoltage ((float) aGenerator[c].process (args.sampleTime) * 10.0f, c);
                    outputs[B_OUTPUT].setVoltage ((float) bGenerator[c].process (args.sampleTime) * 10.0f, c);
                    outputs[RNG_OUTPUT].setVoltage ((float) rngGenerator[c].process (args.sampleTime) * 10.0f, c);
                    outputs[RESET_OUTPUT].setVoltage ((float) resetGenerator[c].process (args.sampleTime) * 10.0f, c);
                }
            }
            for (auto i = 0; i < NUM_OUTPUTS; ++i)
            {
                outputs[i].setChannels (buffer->currentChannels);
            }
        }
    }
};

struct TeWidget : ModuleWidget
{
    TeWidget (Te* module)
    {
        setModule (module);
        setPanel (APP->window->loadSvg (asset::plugin (pluginInstance, "res/Te.svg")));

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (7.65, 21.237)), module, Te::TRIG_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (7.62, 39.49)), module, Te::SHUFFLE_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (7.62, 57.68)), module, Te::A_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (7.62, 75.87)), module, Te::B_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (7.62, 94.06)), module, Te::RNG_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (7.62, 112.581)), module, Te::RESET_OUTPUT));
    }
};

Model* modelTe = createModel<Te, TeWidget> ("Te");