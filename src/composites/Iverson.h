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

#pragma once

#include <memory>
#include <bitset>
#include "IComposite.h"
#include "TriggerSequencer.h"
#include "digital.hpp"

namespace rack
{
    namespace engine
    {
        struct Module;
    }
} // namespace rack
using Module = ::rack::engine::Module;
using namespace rack;

namespace sspo
{
    template <class TBase>
    class IversonDescription : public IComposite
    {
    public:
        Config getParam (int i) override;
        int getNumParams() override;
    };

    /**
 * Complete Iverson composite
 *
 * If TBase is WidgetComposite, this class is used as the implementation part of the KSDelay module.
 * If TBase is TestComposite, this class may stand alone for unit tests.
 */

    template <class TBase>
    class IversonComp : public TBase
    {
    public:
        int MAX_SEQUENCE_LENGTH = 64;
        int GRID_WIDTH = 16;
        int TRACK_COUNT = 8;
        static constexpr int MIDI_MAP_SIZE = 200;
        static constexpr int MAX_MIDI = 128;

        static constexpr float LED_FADE_DELTA = 0.00001f;

        static constexpr int pages = 4;

        enum ParamIds
        {
            GRID_1_1_PARAM,
            GRID_2_1_PARAM,
            GRID_3_1_PARAM,
            GRID_4_1_PARAM,
            GRID_5_1_PARAM,
            GRID_6_1_PARAM,
            GRID_7_1_PARAM,
            GRID_8_1_PARAM,
            GRID_9_1_PARAM,
            GRID_10_1_PARAM,
            GRID_11_1_PARAM,
            GRID_12_1_PARAM,
            GRID_13_1_PARAM,
            GRID_14_1_PARAM,
            GRID_15_1_PARAM,
            GRID_16_1_PARAM,
            GRID_1_2_PARAM,
            GRID_2_2_PARAM,
            GRID_3_2_PARAM,
            GRID_4_2_PARAM,
            GRID_5_2_PARAM,
            GRID_6_2_PARAM,
            GRID_7_2_PARAM,
            GRID_8_2_PARAM,
            GRID_9_2_PARAM,
            GRID_10_2_PARAM,
            GRID_11_2_PARAM,
            GRID_12_2_PARAM,
            GRID_13_2_PARAM,
            GRID_14_2_PARAM,
            GRID_15_2_PARAM,
            GRID_16_2_PARAM,
            GRID_1_3_PARAM,
            GRID_2_3_PARAM,
            GRID_3_3_PARAM,
            GRID_4_3_PARAM,
            GRID_5_3_PARAM,
            GRID_6_3_PARAM,
            GRID_7_3_PARAM,
            GRID_8_3_PARAM,
            GRID_9_3_PARAM,
            GRID_10_3_PARAM,
            GRID_11_3_PARAM,
            GRID_12_3_PARAM,
            GRID_13_3_PARAM,
            GRID_14_3_PARAM,
            GRID_15_3_PARAM,
            GRID_16_3_PARAM,
            GRID_1_4_PARAM,
            GRID_2_4_PARAM,
            GRID_3_4_PARAM,
            GRID_4_4_PARAM,
            GRID_5_4_PARAM,
            GRID_6_4_PARAM,
            GRID_7_4_PARAM,
            GRID_8_4_PARAM,
            GRID_9_4_PARAM,
            GRID_10_4_PARAM,
            GRID_11_4_PARAM,
            GRID_12_4_PARAM,
            GRID_13_4_PARAM,
            GRID_14_4_PARAM,
            GRID_15_4_PARAM,
            GRID_16_4_PARAM,
            GRID_1_5_PARAM,
            GRID_2_5_PARAM,
            GRID_3_5_PARAM,
            GRID_4_5_PARAM,
            GRID_5_5_PARAM,
            GRID_6_5_PARAM,
            GRID_7_5_PARAM,
            GRID_8_5_PARAM,
            GRID_9_5_PARAM,
            GRID_10_5_PARAM,
            GRID_11_5_PARAM,
            GRID_12_5_PARAM,
            GRID_13_5_PARAM,
            GRID_14_5_PARAM,
            GRID_15_5_PARAM,
            GRID_16_5_PARAM,
            GRID_1_6_PARAM,
            GRID_2_6_PARAM,
            GRID_3_6_PARAM,
            GRID_4_6_PARAM,
            GRID_5_6_PARAM,
            GRID_6_6_PARAM,
            GRID_7_6_PARAM,
            GRID_8_6_PARAM,
            GRID_9_6_PARAM,
            GRID_10_6_PARAM,
            GRID_11_6_PARAM,
            GRID_12_6_PARAM,
            GRID_13_6_PARAM,
            GRID_14_6_PARAM,
            GRID_15_6_PARAM,
            GRID_16_6_PARAM,
            GRID_1_7_PARAM,
            GRID_2_7_PARAM,
            GRID_3_7_PARAM,
            GRID_4_7_PARAM,
            GRID_5_7_PARAM,
            GRID_6_7_PARAM,
            GRID_7_7_PARAM,
            GRID_8_7_PARAM,
            GRID_9_7_PARAM,
            GRID_10_7_PARAM,
            GRID_11_7_PARAM,
            GRID_12_7_PARAM,
            GRID_13_7_PARAM,
            GRID_14_7_PARAM,
            GRID_15_7_PARAM,
            GRID_16_7_PARAM,
            GRID_1_8_PARAM,
            GRID_2_8_PARAM,
            GRID_3_8_PARAM,
            GRID_4_8_PARAM,
            GRID_5_8_PARAM,
            GRID_6_8_PARAM,
            GRID_7_8_PARAM,
            GRID_8_8_PARAM,
            GRID_9_8_PARAM,
            GRID_10_8_PARAM,
            GRID_11_8_PARAM,
            GRID_12_8_PARAM,
            GRID_13_8_PARAM,
            GRID_14_8_PARAM,
            GRID_15_8_PARAM,
            GRID_16_8_PARAM,
            ACTIVE_1_PARAM,
            ACTIVE_2_PARAM,
            ACTIVE_3_PARAM,
            ACTIVE_4_PARAM,
            ACTIVE_5_PARAM,
            ACTIVE_6_PARAM,
            ACTIVE_7_PARAM,
            ACTIVE_8_PARAM,
            LENGTH_1_PARAM,
            LENGTH_2_PARAM,
            LENGTH_3_PARAM,
            LENGTH_4_PARAM,
            LENGTH_5_PARAM,
            LENGTH_6_PARAM,
            LENGTH_7_PARAM,
            LENGTH_8_PARAM,
            PAGE_ONE_PARAM,
            PAGE_TWO_PARAM,
            PAGE_THREE_PARAM,
            PAGE_FOUR_PARAM,
            RESET_PARAM,
            CLOCK_PARAM,
            SET_LENGTH_PARAM,
            MIDI_LEARN_PARAM,
            PRIMARY_PROB_1,
            PRIMARY_PROB_2,
            PRIMARY_PROB_3,
            PRIMARY_PROB_4,
            PRIMARY_PROB_5,
            PRIMARY_PROB_6,
            PRIMARY_PROB_7,
            PRIMARY_PROB_8,
            ALT_PROB_1,
            ALT_PROB_2,
            ALT_PROB_3,
            ALT_PROB_4,
            ALT_PROB_5,
            ALT_PROB_6,
            ALT_PROB_7,
            ALT_PROB_8,
            PROB_NOTCH_WIDTH,
            NUM_PARAMS
        };
        enum InputIds
        {
            RESET_INPUT,
            CLOCK_INPUT,
            NUM_INPUTS
        };
        enum OutputIds
        {
            TRIGGER_1_OUTPUT,
            TRIGGER_2_OUTPUT,
            TRIGGER_3_OUTPUT,
            TRIGGER_4_OUTPUT,
            TRIGGER_5_OUTPUT,
            TRIGGER_6_OUTPUT,
            TRIGGER_7_OUTPUT,
            TRIGGER_8_OUTPUT,
            ALT_OUTPUT_1,
            ALT_OUTPUT_2,
            ALT_OUTPUT_3,
            ALT_OUTPUT_4,
            ALT_OUTPUT_5,
            ALT_OUTPUT_6,
            ALT_OUTPUT_7,
            ALT_OUTPUT_8,
            NUM_OUTPUTS
        };
        enum LightIds
        {
            ACTIVE_1_LIGHT,
            ACTIVE_2_LIGHT,
            ACTIVE_3_LIGHT,
            ACTIVE_4_LIGHT,
            ACTIVE_5_LIGHT,
            ACTIVE_6_LIGHT,
            ACTIVE_7_LIGHT,
            ACTIVE_8_LIGHT,
            PAGE_ONE_LIGHT,
            PAGE_TWO_LIGHT,
            PAGE_THREE_LIGHT,
            PAGE_FOUR_LIGHT,
            RESET_LIGHT,
            CLOCK_LIGHT,
            SET_LENGTH_LIGHT,
            MIDI_LEARN_LIGHT,
            NUM_LIGHTS
        };

        float sampleRate = 1.0f;
        float sampleTime = 1.0f;

        /// current page
        int page = 0;
        std::vector<TriggerSequencer<64>> tracks;
        bool isLearning = false;
        bool isSetLength = false;
        bool isClearMapping = false;
        bool isClearAllMapping = false;
        bool clock = false;
        dsp::ClockDivider ledDivider;

        struct Triggers
        {
            std::vector<dsp::TSchmittTrigger<float>> pages{ 4 };
            dsp::TSchmittTrigger<float> length;
            dsp::TSchmittTrigger<float> learn;
            dsp::TSchmittTrigger<float> reset;
            dsp::TSchmittTrigger<float> clock;

            std::vector<dsp::TSchmittTrigger<float>> actives;
            std::vector<dsp::TSchmittTrigger<float>> grid;

            Triggers()
            {
                actives.resize (8);
                grid.resize (128);
            }
        } triggers;

        IversonComp (Module* module) : TBase (module)
        {
        }

        IversonComp() : TBase()
        {
        }

        virtual ~IversonComp() = default;

        /** Implement IComposite
     */
        static std::shared_ptr<IComposite> getDescription()
        {
            return std::make_shared<IversonDescription<TBase>>();
        }

        void setSampleRate (float rate)
        {
            sampleRate = rate;
            sampleTime = 1.0f / rate;
        }

        // must be called after setSampleRate
        void init()
        {
            tracks.resize (TRACK_COUNT);
            for (auto& t : tracks)
                t.setActive (true);
            ledDivider.setDivision (512);
        }

        void step() override;

        /// update tracks from inputs
        void gridInputs();

        /// updates selected page
        void pageChangeInputs();
        /// resets all tracks to -1
        void resetInput();
        /// handles when the length button is pressed
        /// to learn the length from the next grid triggered
        void lengthInput();
        /// midi assign mode
        void learnInput();
        /// reset sequencers
        void clockInput();
        void activeInput();

        int getGridIndex (int x, int y);
        int getStepIndex (int page, int x);
        bool getStateGridIndex (int page, int track, int step);
        void outputSequence();
        void probabilityInput();
    };

    template <class TBase>
    inline void IversonComp<TBase>::step()
    {
        if (ledDivider.process())
        {
            gridInputs();
            lengthInput();
        }

        pageChangeInputs();
        learnInput();
        resetInput();
        clockInput();
        activeInput();
        probabilityInput();
        outputSequence();
    }

    template <class TBase>
    void IversonComp<TBase>::pageChangeInputs()
    {
        for (auto i = 0; i < int (triggers.pages.size()); ++i)
        {
            if (triggers.pages[i].process (TBase::params[PAGE_ONE_PARAM + i].getValue()))
            {
                page = i;
            }
            if (page == i)
                TBase::lights[PAGE_ONE_LIGHT + i].setBrightness (1.0f);
            else
                TBase::lights[PAGE_ONE_LIGHT + i].setBrightness (0.0f);
        }
    }

    template <class TBase>
    void IversonComp<TBase>::resetInput()
    {
        if (triggers.reset.process (TBase::params[RESET_PARAM].getValue()
                                    + std::abs (TBase::inputs[RESET_INPUT].getVoltage())))
        {
            for (auto& t : tracks)
                t.reset();

            TBase::lights[RESET_LIGHT].setBrightness (1.0f);
        }
        else
            TBase::lights[RESET_LIGHT].setSmoothBrightness (0.0f, LED_FADE_DELTA);
    }

    template <class TBase>
    void IversonComp<TBase>::lengthInput()
    {
        for (auto t = 0; t < TRACK_COUNT; ++t)
        {
            if (TBase::params[LENGTH_1_PARAM + t].getValue() > MAX_SEQUENCE_LENGTH)
                TBase::params[LENGTH_1_PARAM + t].setValue (MAX_SEQUENCE_LENGTH);
            tracks[t].setLength (TBase::params[LENGTH_1_PARAM + t].getValue());
        }
        //button length
        if (triggers.length.process (TBase::params[SET_LENGTH_PARAM].getValue())
            && ! isLearning)
        {
            isSetLength = ! isSetLength;
        }
        TBase::lights[SET_LENGTH_LIGHT].setBrightness (isSetLength);
    }

    template <class TBase>
    void IversonComp<TBase>::learnInput()
    {
        if (triggers.learn.process (TBase::params[MIDI_LEARN_PARAM].getValue())
            && ! isSetLength)
        {
            isLearning = ! isLearning;
        }

        TBase::lights[MIDI_LEARN_LIGHT].setBrightness (isLearning);
    }

    template <class TBase>
    void IversonComp<TBase>::gridInputs()
    {
        for (auto t = 0; t < TRACK_COUNT; ++t)
        {
            for (auto s = 0; s < GRID_WIDTH; ++s)
            {
                if (triggers.grid[getGridIndex (s, t)].process (TBase::params[GRID_1_1_PARAM + getGridIndex (s, t)].getValue()))
                {
                    if (isSetLength)
                    {
                        //                    tracks[t].setLength (getStepIndex (page, s));
                        TBase::params[LENGTH_1_PARAM + t].setValue (getStepIndex (page, s + 1));
                        isSetLength = false;
                    }
                    else
                    {
                        tracks[t].invertStep (getStepIndex (page, s));
                    }
                }
            }
        }
    }
    template <class TBase>
    void IversonComp<TBase>::clockInput()
    {
        if (triggers.clock.process (TBase::params[CLOCK_PARAM].getValue()
                                    + std::abs (TBase::inputs[CLOCK_INPUT].getVoltage())))
        {
            clock = true;
            TBase::lights[CLOCK_LIGHT].setBrightness (1.0f);
        }
        else
        {
            clock = false;
            TBase::lights[CLOCK_LIGHT].setSmoothBrightness (0.0f, LED_FADE_DELTA);
        }
    }
    template <class TBase>
    void IversonComp<TBase>::activeInput()
    {
        for (auto i = 0; i < TRACK_COUNT; i++)
        {
            if (triggers.actives[i].process (TBase::params[ACTIVE_1_PARAM + i].getValue()))
            {
                tracks[i].setActive (! tracks[i].getActive());
            }
            TBase::lights[ACTIVE_1_LIGHT + i].setBrightness (tracks[i].getActive());
        }
    }
    template <class TBase>
    int IversonComp<TBase>::getGridIndex (int x, int y)
    {
        return y * GRID_WIDTH + x;
    }
    template <class TBase>
    int IversonComp<TBase>::getStepIndex (int page, int x)
    {
        return page * GRID_WIDTH + x;
    }
    template <class TBase>
    void IversonComp<TBase>::outputSequence()
    {
        for (auto t = 0; t < TRACK_COUNT; t++)
        {
            tracks[t].step (clock);
            if (tracks[t].getPrimaryState())
                TBase::outputs[TRIGGER_1_OUTPUT + t].setVoltage (TBase::inputs[CLOCK_INPUT].getVoltage());
            else
                TBase::outputs[TRIGGER_1_OUTPUT + t].setVoltage (0);

            if (tracks[t].getAltState())
                TBase::outputs[ALT_OUTPUT_1 + t].setVoltage (TBase::inputs[CLOCK_INPUT].getVoltage());
            else
                TBase::outputs[ALT_OUTPUT_1 + t].setVoltage (0);
        }
    }

    template <class TBase>
    bool IversonComp<TBase>::getStateGridIndex (int page, int track, int step)
    {
        return tracks[track].getStep (getStepIndex (page, step));
    }
    template <class TBase>
    void IversonComp<TBase>::probabilityInput()
    {
        for (auto i = 0; i < TRACK_COUNT; ++i)
        {
            //flat notch in center of primary probability to allow for inaccurate placement
            //of controls during performance
            auto prob = 0.0f;
            auto pp = TBase::params[PRIMARY_PROB_1 + i].getValue() - 1; // parameter probability
            auto n = TBase::params[PROB_NOTCH_WIDTH].getValue();
            if (pp < -n)
                prob = pp * (1.0 / (1 - n)) + (n / (1 - n));
            else if (pp > n)
                prob = pp * (1.0 / (1 - n)) - (n / (1 - n));

            tracks[i].setPrimaryProbability (prob + 1);
            tracks[i].setAltProbability (TBase::params[ALT_PROB_1 + i].getValue());
        }
    }

    template <class TBase>
    int IversonDescription<TBase>::getNumParams()
    {
        return IversonComp<TBase>::NUM_PARAMS;
    }

    template <class TBase>
    IComposite::Config IversonDescription<TBase>::getParam (int i)
    {
        IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Code type", "unit", 0.0f, 1.0f, 0.0f };
        if (i <= IversonComp<TBase>::GRID_16_8_PARAM)
        {
            ret = { 0.0f, 1.0f, 0.0f, " ", " ", 0, 1, 0.0f };
            return ret;
        }
        if (i <= IversonComp<TBase>::ACTIVE_8_PARAM)
        {
            ret = { 0.0f, 1.0f, 0.0f, " ", " ", 0, 1, 0.0f };
            return ret;
        }

        if (i <= IversonComp<TBase>::LENGTH_8_PARAM)
        {
            ret = { 1.0f, 64, 64, " ", " ", 0, 1, 0.0f };
            return ret;
        }

        switch (i)
        {
            case IversonComp<TBase>::PAGE_ONE_PARAM:
                ret = { 0.0f, 1.0f, 0.0f, "Page One", " ", 0, 1, 0.0f };
                break;
            case IversonComp<TBase>::PAGE_TWO_PARAM:
                ret = { 0.0f, 1.0f, 0.0f, "Page Two", " ", 0, 1, 0.0f };
                break;
            case IversonComp<TBase>::PAGE_THREE_PARAM:
                ret = { 0.0f, 1.0f, 0.0f, "Page Three", " ", 0, 1, 0.0f };
                break;
            case IversonComp<TBase>::PAGE_FOUR_PARAM:
                ret = { 0.0f, 1.0f, 0.0f, "Page Four", " ", 0, 1, 0.0f };
                break;
            case IversonComp<TBase>::RESET_PARAM:
                ret = { 0.0f, 1.0f, 0.0f, "Reset", " ", 0, 1, 0.0f };
                break;
            case IversonComp<TBase>::CLOCK_PARAM:
                ret = { 0.0f, 1.0f, 0.0f, "Clock", " ", 0, 1, 0.0f };
                break;
            case IversonComp<TBase>::SET_LENGTH_PARAM:
                ret = { 0.0f, 1.0f, 0.0f, "Length", " ", 0, 1, 0.0f };
                break;
            case IversonComp<TBase>::MIDI_LEARN_PARAM:
                ret = { 0.0f, 1.0f, 0.0f, "Midi Learn", " ", 0, 1, 0.0f };
                break;
            case IversonComp<TBase>::PROB_NOTCH_WIDTH:
                ret = { 0.0f, 0.5f, 0.0f, "Probability Notch Width", " ", 0, 1, 0.0f };
                break;
            default:
                if (i <= IversonComp<TBase>::PRIMARY_PROB_8)
                {
                    ret = { 0.0f, 2.0f, 1.0f, " ", " ", 0, 1, 0.0f };
                    return ret;
                }
                if (i <= IversonComp<TBase>::ALT_PROB_8)
                {
                    ret = { 0.0f, 1.0f, 1.0f, " ", " ", 0, 1, 0.0f };
                    return ret;
                }
        }
        return ret;
    }
} // namespace sspo