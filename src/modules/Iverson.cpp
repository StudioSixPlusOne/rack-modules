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

//#include <rack0.hpp>
#include "plugin.hpp"
#include "widgets.h"
#include "Iverson.h"
#include "WidgetComposite.h"
#include "ctrl/SqMenuItem.h"
#include "app/MidiWidget.hpp"

#include <sstream>

namespace sspo
{
    using Comp = IversonComp<WidgetComposite>;
    using namespace ::rack;

    struct IversonBase : Module
    {
        struct MidiOutput : midi::Output
        {
            int currentCC[Comp::MAX_MIDI]{};
            int currentNotes[Comp::MAX_MIDI]{};

            MidiOutput()
            {
                resetState();
            }

            void resetState()
            {
                for (auto i = 0; i < Comp::MAX_MIDI; ++i)
                {
                    currentCC[i] = -1;
                    currentNotes[i] = -1;
                }
            }

            void setCC (int cc, int val)
            {
                DEBUG ("set cc %d %d", cc, val);
                if (val == currentCC[cc])
                    return;
                currentCC[cc] = val;
                // create message
                midi::Message msg;
                msg.setStatus (0xb);
                msg.setNote (cc);
                msg.setValue (val);
                try
                {
                    sendMessage (msg);
                }
                catch (const std::exception& e)
                {
                    DEBUG ("Iverson %s", e.what());
                    DEBUG ("%d", this->channel);
                }
            }

            void resetNote (int note)
            {
                midi::Message msg;
                msg.setStatus (0x9);
                msg.setNote (note);
                msg.setValue (0);
                try
                {
                    sendMessage (msg);
                    currentNotes[note] = false;
                }
                catch (const std::exception& e)
                {
                    DEBUG ("Iverson reset note %s", e.what());
                }
            }

            void setNote (int note, int velocity)
            {
                if (velocity != currentNotes[note])
                {
                    //note on
                    midi::Message msg;
                    msg.setStatus (0x9);
                    msg.setNote (note);
                    msg.setValue (velocity);
                    try
                    {
                        sendMessage (msg);
                    }
                    catch (const std::exception& e)
                    {
                        DEBUG ("Iverson setNote %s", e.what());
                    }
                }
                else if (velocity == -1)
                {
                    //note off
                    midi::Message msg;
                    msg.setStatus (0x9);
                    msg.setNote (note);
                    msg.setValue (0);
                    try
                    {
                        sendMessage (msg);
                    }
                    catch (const std::exception& e)
                    {
                        DEBUG ("Iverson setNote -1 %s", e.what());
                    }
                }
                currentNotes[note] = velocity;
            }
        };

        struct MidiMapping
        {
            int controller = -1;
            int note = -1;
            int cc = -1;
            int paramId = -1;

            void reset()
            {
                controller = -1;
                note = -1;
                cc = -1;
                paramId = -1;
            }
        };

        // to be defined in deriving classes and passed to composite
        int MAX_SEQUENCE_LENGTH = 64;
        int GRID_WIDTH = 16;
        int TRACK_COUNT = 8;

        static constexpr int MIDI_FEEDBACK_SLOW_RATE = 10000;
        static constexpr int MIDI_FEEDBACK_FAST_RATE = 4096;

        std::shared_ptr<Comp> iverson;
        std::vector<midi::InputQueue> midiInputQueues{ 2 };
        std::vector<MidiOutput> midiOutputs{ 2 };
        dsp::ClockDivider controllerPageUpdateDivider;
        dsp::ClockDivider paramMidiUpdateDivider;
        dsp::ClockDivider midiOutStateResetDivider;
        std::vector<MidiMapping> midiMappings;
        MidiMapping midiLearnMapping;

        IversonBase();
        void onReset() override;
        void onSampleRateChange() override;
        json_t* dataToJson() override;
        void dataFromJson (json_t* rootJ) override;
        void doLearn();
        void process (const ProcessArgs& args) override;

        /// Midi events are used to set assigned params
        /// midi handling would require linking to RACK for unit test
        /// hence all midi to be processed in Iverson.cpp
        void midiToParm();

        /// sends midi to external controller to show status
        void pageLights();
        bool isGridMidiMapped (int x, int y);
        std::string getMidiAssignment (int x, int y);
    };

    /**
	 * Checks for midi mappings and passes data to the assigned parameter
	 */
    void IversonBase::midiToParm()
    {
        midi::Message msg;
        for (auto q = 0; q < GRID_WIDTH / 8; ++q) // GRID_WIDTH / 8 = number of midi controllers
        {
            while (midiInputQueues[q].tryPop(&msg, -1)) //TODO -1 used as placeholder
            {
                switch (msg.getStatus())
                {
                    //note off
                    case 0x8:
                    {
                        //find midiMapping for noteOff
                        for (auto& m : midiMappings)
                        {
                            if (m.note == msg.getNote() && m.controller == q)
                                params[m.paramId].setValue (0);
                        }
                    }
                    break;

                        //note on
                    case 0x9:
                    {
                        //find midiMapping for noteOn
                        for (auto& m : midiMappings)
                        {
                            if (m.note == msg.getNote() && m.controller == q)
                                params[m.paramId].setValue (msg.getValue() == 0 ? 0 : 1);
                        }
                    }
                    break;
                        // cc
                    case 0xb:
                    {
                        //find midiMapping for cc
                        for (auto& m : midiMappings)
                        {
                            if (m.cc == msg.getNote() && m.controller == q)
                            {
                                if ((bool) iverson->params[Comp::USE_ROTARY_ENCODERS_PARAM].getValue()
                                    && (m.paramId >= Comp::PRIMARY_PROB_1 && m.paramId <= Comp::ALT_PROB_8))
                                {
                                    auto currentScaledValue = paramQuantities[m.paramId]->getScaledValue();
                                    auto step = 1.0f / 127.0f; // midi cc = 127 steps
                                    currentScaledValue = msg.getValue() > 64
                                                             ? currentScaledValue - step
                                                             : currentScaledValue + step;
                                    paramQuantities[m.paramId]->setScaledValue (currentScaledValue);
                                }
                                else
                                {
                                    paramQuantities[m.paramId]->setScaledValue (
                                        (float) msg.getValue() / 127.0f); //((msg.getValue() == 0 ? 0 : 1));
                                }
                            }
                        }
                    }

                    break;
                    default:
                    {
                    }
                }
            }
        }
    }

    /**
	 * Midi mapping from controller to parameters
	 */
    void IversonBase::doLearn()
    {
        if (! iverson->isLearning)
            midiLearnMapping.reset();
        if (iverson->isClearAllMapping)
        {
            midiMappings.clear();
            iverson->isClearAllMapping = false;
        }

        if (iverson->isClearMapping)
        {
            //parameter selected
            if (midiLearnMapping.paramId != -1)
            {
                auto mm = std::find_if (midiMappings.begin(),
                                        midiMappings.end(),
                                        [this] (const MidiMapping& x) {
                                            return (x.paramId == midiLearnMapping.paramId);
                                        });
                if (mm != midiMappings.end())
                {
                    midiMappings.erase (mm);
                    midiLearnMapping.reset();
                    iverson->isClearMapping = false;
                    iverson->isLearning = false;
                }
            }

            //midi note selected

            auto mm = std::find_if (midiMappings.begin(),
                                    midiMappings.end(),
                                    [this] (const MidiMapping& x) {
                                        return x.note != -1
                                               && x.note == midiLearnMapping.note
                                               && (x.controller == midiLearnMapping.controller);
                                    });

            if (mm != midiMappings.end())
            {
                midiMappings.erase (mm);
                midiLearnMapping.reset();
                iverson->isClearMapping = false;
                iverson->isLearning = false;
            }
        }

        if (iverson->isLearning)
        {
            // if we have all required params, add to list

            if ((midiLearnMapping.controller != -1)
                && (midiLearnMapping.cc != -1 || midiLearnMapping.note != -1)
                && midiLearnMapping.paramId != -1)
            {
                //delete any existing map to this parameter
                auto mm = std::find_if (midiMappings.begin(),
                                        midiMappings.end(),
                                        [this] (const MidiMapping& x) {
                                            return (x.paramId == midiLearnMapping.paramId);
                                        });
                if (mm != midiMappings.end())
                    midiMappings.erase (mm);

                //delete any existing map to this midi note
                mm = std::find_if (midiMappings.begin(),
                                   midiMappings.end(),
                                   [this] (const MidiMapping& x) {
                                       return x.note != -1
                                              && x.note == midiLearnMapping.note
                                              && (x.controller == midiLearnMapping.controller);
                                   });
                if (mm != midiMappings.end())
                    midiMappings.erase (mm);

                //delete any existing map to this midi cc
                mm = std::find_if (midiMappings.begin(),
                                   midiMappings.end(),
                                   [this] (const MidiMapping& x) {
                                       return x.cc != -1
                                              && x.cc == midiLearnMapping.cc
                                              && (x.controller == midiLearnMapping.controller);
                                   });
                if (mm != midiMappings.end())
                    midiMappings.erase (mm);

                midiMappings.push_back (midiLearnMapping);
                midiLearnMapping.reset();
                // dont turn off midi learn, to allow multiple assignments
            }

            if ((iverson->params[Comp::MIDI_LEARN_PARAM_FIRST].getValue() && midiLearnMapping.paramId != -1)
                || (! iverson->params[Comp::MIDI_LEARN_PARAM_FIRST].getValue()))
            {
                // if midi add to midi learn param
                midi::Message msg;
                for (auto q = 0; q < GRID_WIDTH / 8; ++q) // GRID_WIDTH /8 == number of midi controllers
                {
                    while (midiInputQueues[q].tryPop(&msg, -1)) //TODO -1 placeholder
                    {
                        switch (msg.getStatus())
                        {
                            //note on
                            case 0x9:
                            {
                                midiLearnMapping.controller = q;
                                midiLearnMapping.note = msg.getNote();
                            }
                            break;
                                // cc
                            case 0xb:
                            {
                                midiLearnMapping.controller = q;
                                midiLearnMapping.cc = msg.getNote();
                                midiLearnMapping.note = -1; // if both note and cc assigned, just use cc
                            }
                            break;
                            default:
                                break;
                        }
                    }
                }
            }

            //if param add to midi learn param

            if ((iverson->params[Comp::MIDI_LEARN_PARAM_FIRST].getValue())
                || ((! iverson->params[Comp::MIDI_LEARN_PARAM_FIRST].getValue()) && (midiLearnMapping.cc != -1 || midiLearnMapping.note != -1)))
            {
                ParamWidget* touchedParam = APP->scene->rack->touchedParam;
                if (touchedParam)
                {
                    APP->scene->rack->touchedParam = nullptr;
                    // dont learn midi learn param as turning off midi learn may result in overriding
                    // the last learnt parameter with midi learn.
                    if (touchedParam->getParamQuantity()->paramId != iverson->MIDI_LEARN_PARAM)
                        midiLearnMapping.paramId = touchedParam->getParamQuantity()->paramId;
                }
            }
        }
    }

    /**
	 * velocity values used to set colours on midi controllers
	 */
    struct MidiFeedbackVelocity
    {
        int none = 0;
        int activeStep = 1;
        int loop = 3;
        int loopStep = 5;
        int index = 5;
    } midiFeedback;

    void IversonBase::pageLights()
    {
        //set colours for feedback from parameters
        midiFeedback.none = (int) iverson->params[Comp::MIDI_FEEDBACK_VELOCITY_NONE].getValue();
        midiFeedback.activeStep = (int) iverson->params[Comp::MIDI_FEEDBACK_VELOCITY_STEP].getValue();
        midiFeedback.loop = (int) iverson->params[Comp::MIDI_FEEDBACK_VELOCITY_LOOP].getValue();
        midiFeedback.loopStep = (int) iverson->params[Comp::MIDI_FEEDBACK_VELOCITY_LOOP_STEP].getValue();
        midiFeedback.index = (int) iverson->params[Comp::MIDI_FEEDBACK_VELOCITY_INDEX].getValue();

        for (auto& mm : midiMappings)
        {
            if (! iverson->isLearning)
            {
                if (mm.cc == -1)
                {
                    if (mm.paramId <= iverson->GRID_16_8_PARAM && mm.paramId != -1)
                    // sequence
                    {
                        auto t = mm.paramId / iverson->GRID_WIDTH;
                        auto i = mm.paramId - t * iverson->GRID_WIDTH;
                        auto midiColor = 0;
                        if (iverson->tracks[t].getIndex() != i + iverson->page * iverson->GRID_WIDTH)
                        {
                            if (iverson->tracks[t].getLength() - 1 == i + iverson->page * iverson->GRID_WIDTH)
                            {
                                midiColor = iverson->getStateGridIndex (iverson->page, t, i)
                                                ? midiFeedback.loopStep
                                                : midiFeedback.loop;
                            }
                            else
                                midiColor = iverson->getStateGridIndex (iverson->page, t, i)
                                                ? midiFeedback.activeStep
                                                : midiFeedback.none;
                        }
                        else
                            midiColor = midiFeedback.index;
                        midiOutputs[mm.controller].setNote (mm.note, midiColor);
                    }
                    //Active lights
                    else if (mm.paramId >= iverson->ACTIVE_1_PARAM && mm.paramId <= iverson->ACTIVE_8_PARAM)
                    {
                        auto t = mm.paramId - iverson->ACTIVE_1_PARAM;
                        midiOutputs[mm.controller].setNote (mm.note, iverson->tracks[t].getActive());
                    }
                    //Page Lights
                    else if (mm.paramId >= iverson->PAGE_ONE_PARAM && mm.paramId <= iverson->PAGE_FOUR_PARAM)
                    {
                        auto pageIndex = mm.paramId - iverson->PAGE_ONE_PARAM;
                        midiOutputs[mm.controller].setNote (mm.note, pageIndex == iverson->page);
                    }
                    else if (mm.paramId == iverson->SET_LENGTH_PARAM)
                    {
                        midiOutputs[mm.controller].setNote (mm.note, iverson->isSetLength);
                    }
                    else if (mm.paramId == iverson->RESET_PARAM)
                    {
                        midiOutputs[mm.controller].setNote (mm.note, iverson->params[Comp::RESET_PARAM].getValue());
                    }
                    else if (mm.paramId == iverson->CLOCK_PARAM)
                    {
                        midiOutputs[mm.controller].setNote (mm.note, iverson->params[Comp::CLOCK_PARAM].getValue());
                    }
                    else if (mm.paramId == iverson->SET_EUCLIDEAN_HITS_PARAM)
                    {
                        midiOutputs[mm.controller].setNote (mm.note, iverson->isSetEuclideanHits);
                    }
                    else if (mm.paramId == iverson->ROTATE_TRACK_PARAM)
                    {
                        midiOutputs[mm.controller].setNote (mm.note, iverson->isRotateTrack);
                    }
                }
            }
            else if (mm.note != -1) //midi learn
            {
                midiOutputs[mm.controller].setNote (mm.note, 1);
            }
        }
    }

    bool IversonBase::isGridMidiMapped (int x, int y)
    {
        auto mapping = std::find_if (midiMappings.begin(),
                                     midiMappings.end(),
                                     [this, x, y] (const MidiMapping mm) {
                                         return mm.paramId == iverson->getGridIndex (x, y) + Comp::GRID_1_1_PARAM;
                                     });

        return mapping != midiMappings.end();
    }

    std::string IversonBase::getMidiAssignment (int x, int y)
    {
        auto mapping = std::find_if (midiMappings.begin(),
                                     midiMappings.end(),
                                     [this, x, y] (const MidiMapping mm) {
                                         return mm.paramId == iverson->getGridIndex (x, y) + Comp::GRID_1_1_PARAM;
                                     });

        if (mapping == midiMappings.end())
            return "";

        std::stringstream ss;
        ss << mapping->controller << ":" << mapping->note;
        return ss.str();
    }

    void IversonBase::onReset()
    {
        for (auto i = 0; i < int (iverson->tracks.size()); ++i)
        {
            auto& t = iverson->tracks[i];
            t.setIndex (-1);
            t.reset();
            t.setActive (true);
            t.resetSequence();
            params[Comp::ACTIVE_1_PARAM + i].setValue (false);
            params[Comp::PRIMARY_PROB_1 + i].setValue (1.0f);
            params[Comp::ALT_PROB_1 + i].setValue (0);
        }
        Module::onReset();
    }

    void IversonBase::process (const Module::ProcessArgs& args)
    {
        doLearn();
        if (paramMidiUpdateDivider.process())
        {
            midiToParm();
        }

        iverson->step();
        if (controllerPageUpdateDivider.process())
        {
            pageLights();
            controllerPageUpdateDivider.setDivision ((bool) iverson->params[Comp::MIDI_FEEDBACK_DIVIDER_SLOW]
                                                             .getValue()
                                                         ? MIDI_FEEDBACK_SLOW_RATE
                                                         : MIDI_FEEDBACK_FAST_RATE);
        }

        if (midiOutStateResetDivider.process())
        {
            for (auto& m : midiOutputs)
                m.resetState();
        }
    }
    void IversonBase::dataFromJson (json_t* rootJ)
    {
        json_t* activesJ = json_object_get (rootJ, "actives");
        for (auto t = 0; t < iverson->TRACK_COUNT; ++t)
        {
            if (activesJ)
            {
                json_t* activesArrayJ = json_array_get (activesJ, t);
                if (activesArrayJ)
                    iverson->tracks[t].setActive (json_boolean_value (activesArrayJ));
            }
        }

        json_t* lengthsJ = json_object_get (rootJ, "lengths");
        for (auto t = 0; t < iverson->TRACK_COUNT; ++t)
        {
            if (lengthsJ)
            {
                json_t* lengthsArrayJ = json_array_get (lengthsJ, t);
                if (lengthsArrayJ)
                    iverson->tracks[t].setLength (json_integer_value (lengthsArrayJ));
            }
        }

        json_t* indexJ = json_object_get (rootJ, "index");
        for (auto t = 0; t < iverson->TRACK_COUNT; ++t)
        {
            if (indexJ)
            {
                json_t* indexArrayJ = json_array_get (indexJ, t);
                if (indexArrayJ)
                    iverson->tracks[t].setIndex (json_integer_value (indexArrayJ));
            }
        }
        //sequence values 64 bit, split int low hi 32bits
        json_t* sequenceLowJ = json_object_get (rootJ, "sequenceLow");
        for (auto t = 0; t < iverson->TRACK_COUNT; ++t)
        {
            if (sequenceLowJ)
            {
                json_t* sequenceArrayLowJ = json_array_get (sequenceLowJ, t);
                if (sequenceArrayLowJ)
                    iverson->tracks[t].setSequence (json_integer_value (sequenceArrayLowJ));
            }
        }

        json_t* sequenceHiJ = json_object_get (rootJ, "sequenceHi");
        for (auto t = 0; t < iverson->TRACK_COUNT; ++t)
        {
            if (sequenceHiJ)
            {
                json_t* sequenceArrayHiJ = json_array_get (sequenceHiJ, t);
                if (sequenceArrayHiJ)
                    iverson->tracks[t].setSequence (iverson->tracks[t].getSequence().to_ulong()
                                                    + ((int64_t) json_integer_value (sequenceArrayHiJ) << 32u));
            }
        }

        json_t* midiBindingJ = json_object_get (rootJ, "midiBinding");
        midiMappings.resize ((int) json_array_size (midiBindingJ));
        midiMappings.reserve (iverson->MIDI_MAP_SIZE);
        for (auto i = 0; i < (int) json_array_size (midiBindingJ); ++i)
        {
            if (midiBindingJ)
            {
                json_t* mm = json_array_get (midiBindingJ, i);
                if (mm)
                {
                    json_t* controllerJ = json_object_get (mm, "controller");
                    if (controllerJ)
                        midiMappings[i].controller = json_integer_value (controllerJ);

                    json_t* noteJ = json_object_get (mm, "note");
                    if (noteJ)
                        midiMappings[i].note = json_integer_value (noteJ);

                    json_t* ccJ = json_object_get (mm, "cc");
                    if (ccJ)
                        midiMappings[i].cc = json_integer_value (ccJ);

                    json_t* paramJ = json_object_get (mm, "paramId");
                    if (paramJ)
                        midiMappings[i].paramId = json_integer_value (paramJ);
                }
            }
        }
        json_t* midiInputLeftJ = json_object_get (rootJ, "midiInputLeft");
        if (midiInputLeftJ)
            midiInputQueues[0].fromJson (midiInputLeftJ);

        json_t* midiInputRightJ = json_object_get (rootJ, "midiInputRight");
        if (midiInputRightJ)
            midiInputQueues[1].fromJson (midiInputRightJ);

        json_t* midiOutputLeftJ = json_object_get (rootJ, "midiOutputLeft");
        if (midiOutputLeftJ)
            midiOutputs[0].fromJson (midiOutputLeftJ);

        json_t* midiOutputRightJ = json_object_get (rootJ, "midiOutputRight");
        if (midiOutputRightJ)
            midiOutputs[1].fromJson (midiOutputRightJ);
    }
    json_t* IversonBase::dataToJson()
    {
        json_t* rootJ = json_object();

        json_t* activesJ = json_array();
        json_t* lengthsJ = json_array();
        json_t* indexJ = json_array();
        json_t* sequenceHiJ = json_array();
        json_t* sequenceLowJ = json_array();

        for (auto i = 0; i < TRACK_COUNT; ++i)
        {
            json_array_insert_new (activesJ, i, json_boolean ((iverson->tracks[i].getActive())));
            json_array_insert_new (lengthsJ, i, json_integer (iverson->tracks[i].getLength()));
            json_array_insert_new (indexJ, i, json_integer (iverson->tracks[i].getIndex()));
            //sequence 64bit, json integers are 32bit, store ass hi low values
            json_array_insert_new (sequenceHiJ,
                                   i,
                                   json_integer (
                                       ((iverson->tracks[i].getSequence().to_ullong()) >> 32u) & 0xffffffff));
            json_array_insert_new (sequenceLowJ,
                                   i,
                                   json_integer (((iverson->tracks[i].getSequence().to_ullong())) & 0xffffffff));
        }

        json_object_set_new (rootJ, "actives", activesJ);
        json_object_set_new (rootJ, "lengths", lengthsJ);
        json_object_set_new (rootJ, "index", indexJ);
        json_object_set_new (rootJ, "sequenceHi", sequenceHiJ);
        json_object_set_new (rootJ, "sequenceLow", sequenceLowJ);

        json_t* midiMapsJ = json_array();
        for (auto i = 0; i < (int) midiMappings.size(); ++i)
        {
            json_t* mappingJ = json_object();
            json_object_set_new (mappingJ, "controller", json_integer (midiMappings[i].controller));
            json_object_set_new (mappingJ, "note", json_integer (midiMappings[i].note));
            json_object_set_new (mappingJ, "cc", json_integer (midiMappings[i].cc));
            json_object_set_new (mappingJ, "paramId", json_integer (midiMappings[i].paramId));
            json_array_insert_new (midiMapsJ, i, mappingJ);
        }

        json_object_set_new (rootJ, "midiBinding", midiMapsJ);
        json_object_set_new (rootJ, "midiInputLeft", midiInputQueues[0].toJson());
        json_object_set_new (rootJ, "midiInputRight", midiInputQueues[1].toJson());
        json_object_set_new (rootJ, "midiOutputLeft", midiOutputs[0].toJson());
        json_object_set_new (rootJ, "midiOutputRight", midiOutputs[1].toJson());

        return rootJ;
    }

    void IversonBase::onSampleRateChange()
    {
        float rate = SqHelper::engineGetSampleRate();
        iverson->setSampleRate (rate);
    }

    IversonBase::IversonBase()
    {
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);

        //preallocate midi mappings, to avoid allocations during push_back
        // occurring during the audio process loop
        iverson = std::make_shared<Comp> (this);
        midiMappings.reserve (iverson->MIDI_MAP_SIZE);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);
        onSampleRateChange();
        iverson->init();

        controllerPageUpdateDivider.setDivision (4096);
        paramMidiUpdateDivider.setDivision (128);
        midiOutStateResetDivider.setDivision (131072);
    }

    struct Iverson : IversonBase
    {
        Iverson() : IversonBase()
        {
            iverson->MAX_SEQUENCE_LENGTH = 64;
            iverson->GRID_WIDTH = 16;
            iverson->TRACK_COUNT = 8;
            for (auto i = 0; i < TRACK_COUNT; ++i)
                iverson->params[Comp::LENGTH_1_PARAM + i].setValue ((float) iverson->GRID_WIDTH);
        }
    };

    struct IversonJr : IversonBase
    {
        IversonJr() : IversonBase()
        {
            iverson->MAX_SEQUENCE_LENGTH = 32;
            iverson->GRID_WIDTH = 8;
            iverson->TRACK_COUNT = 8;
            for (auto i = 0; i < TRACK_COUNT; ++i)
                iverson->params[Comp::LENGTH_1_PARAM + i].setValue ((float) iverson->GRID_WIDTH);
        }
    };

    /*****************************************************
User Interface
*****************************************************/

    struct GridColors
    {
        NVGcolor none;
        NVGcolor on;
        NVGcolor loop;
        NVGcolor loopAndBeat;
        NVGcolor index;
        NVGcolor page;
        NVGcolor midiLearning;
        NVGcolor midiAssigned;

        GridColors()
        {
            none = nvgRGBA (0, 0, 0, 255);
            on = nvgRGBA (0, 255, 0, 255);
            loop = nvgRGBA (255, 0, 0, 255);
            loopAndBeat = nvgRGBA (255, 255, 0, 255);
            index = nvgRGBA (255, 255, 0, 255);
            page = nvgRGBA (255, 255, 255, 100);
            midiLearning = nvgRGBA (0, 0, 255, 255);
            midiAssigned = nvgRGBA (0, 255, 255, 255);
        }
    };

    struct SummaryWidget : Widget
    {
        IversonBase* module = nullptr;

        GridColors gridColors;

        SummaryWidget()
        {
            gridColors.none = nvgRGBA (0, 0, 0, 255);
        }

        void setModule (IversonBase* module)
        {
            this->module = module;
        }

        void step() override
        {
            Widget::step();
        }

        void draw (const DrawArgs& args) override
        {
            if (module == nullptr)
                return;
            auto beatWidth = box.size.x / (float) module->iverson->MAX_SEQUENCE_LENGTH;
            auto trackHeight = box.size.y / module->iverson->tracks.size();

            for (auto t = 0; t < int (module->iverson->tracks.size()); ++t)
            {
                //plot beats
                for (auto b = 0; b < module->iverson->MAX_SEQUENCE_LENGTH; ++b)
                {
                    auto color = module->iverson->tracks[t].getStep (b)
                                     ? gridColors.on
                                     : gridColors.none;
                    nvgFillColor (args.vg, color);
                    nvgBeginPath (args.vg);
                    nvgRect (args.vg, b * beatWidth, t * trackHeight, beatWidth, trackHeight);
                    nvgFill (args.vg);
                }
                //plot indexes
                if (module->iverson->tracks[t].getIndex() != -1)
                {
                    auto index = module->iverson->tracks[t].getIndex();
                    auto color = gridColors.index;
                    nvgFillColor (args.vg, color);
                    nvgBeginPath (args.vg);
                    nvgRect (args.vg, index * beatWidth, t * trackHeight, beatWidth, trackHeight);
                    nvgFill (args.vg);
                }
                //plot loops
                {
                    auto loop = module->iverson->tracks[t].getLength() - 1;
                    auto color = module->iverson->tracks[t].getStep (loop)
                                     ? gridColors.loopAndBeat
                                     : gridColors.loop;
                    nvgFillColor (args.vg, color);
                    nvgBeginPath (args.vg);
                    nvgRect (args.vg, loop * beatWidth, t * trackHeight, beatWidth, trackHeight);
                    nvgFill (args.vg);
                }
            }

            //draw current page
            auto page = module->iverson->page;
            auto pageWidth =
                beatWidth * ((float) module->iverson->MAX_SEQUENCE_LENGTH / (float) module->iverson->pages);
            nvgFillColor (args.vg, gridColors.page);
            nvgBeginPath (args.vg);
            nvgRect (args.vg, page * pageWidth, 0, pageWidth, box.size.y);

            nvgFill (args.vg);
        }
    };

    struct GridWidget : LightWidget
    {
        IversonBase* module = nullptr;
        GridColors gridColors;
        std::shared_ptr<Font> font;
        NVGcolor txtColor;
        const float fontHeight = 8;

        struct GridLocation
        {
            int x = 0;
            int y = 0;
        } gridLocation;

        GridWidget()
        {
            gridColors.none = nvgRGBA (77, 77, 77, 100);
            font = APP->window->loadFont (asset::system ("res/fonts/ShareTechMono-Regular.ttf"));
            txtColor = nvgRGBA (0, 0, 0, 255);
        }

        void setModule (IversonBase* mod)
        {
            module = mod;
        }

        void setGridLocation (int x, int y)
        {
            gridLocation.x = x;
            gridLocation.y = y;
        }

        void draw (const DrawArgs& args) override
        {
            auto color = gridColors.none;
            if (module != nullptr)
            {
                auto xOffset = module->iverson->page * module->iverson->GRID_WIDTH;
                if (! module->iverson->isLearning) // not learning
                {
                    // step active
                    color = module->iverson->tracks[gridLocation.y].getStep (gridLocation.x + xOffset)
                                ? gridColors.on
                                : gridColors.none;
                    //index
                    color = module->iverson->tracks[gridLocation.y].getIndex() == gridLocation.x + xOffset
                                ? gridColors.index
                                : color;
                    //loop length on active step
                    color = module->iverson->tracks[gridLocation.y].getLength() - 1 == gridLocation.x + xOffset
                                    && module->iverson->tracks[gridLocation.y].getStep (gridLocation.x + xOffset)
                                ? gridColors.loopAndBeat
                                : color;
                    //loop length on inactive step
                    color = module->iverson->tracks[gridLocation.y].getLength() - 1 == gridLocation.x + xOffset
                                    && ! module->iverson->tracks[gridLocation.y].getStep (gridLocation.x + xOffset)
                                ? gridColors.loop
                                : color;
                }
                else //midi learn mode
                {
                    //currently learning
                    color = module->midiLearnMapping.paramId
                                    == module->iverson->GRID_1_1_PARAM
                                           + module->iverson->getGridIndex (gridLocation.x, gridLocation.y)
                                ? gridColors.midiLearning
                                : gridColors.none;
                    //already assigned
                    color = module->isGridMidiMapped (gridLocation.x, gridLocation.y)
                                ? gridColors.midiAssigned
                                : color;
                }
            }
            auto gradient = nvgRadialGradient (args.vg,
                                               box.size.x / 2,
                                               box.size.y / 2,
                                               box.size.y / 10,
                                               box.size.x * 0.75f,
                                               color,
                                               gridColors.none);

            nvgBeginPath (args.vg);
            nvgFillPaint (args.vg, gradient);
            nvgRoundedRect (args.vg, 0, 0, box.size.x, box.size.y, box.size.x / 10.0f);
            nvgFill (args.vg);

            //assignment text
            if (module != nullptr && module->iverson->isLearning)
            {
                nvgFontSize (args.vg, fontHeight);
                nvgFontFaceId (args.vg, font->handle);
                nvgTextAlign (args.vg, NVG_ALIGN_LEFT);
                nvgFillColor (args.vg, txtColor);
                auto txt = module->getMidiAssignment (gridLocation.x, gridLocation.y);
                Vec c = Vec (1, 12);
                nvgText (args.vg, c.x, c.y, txt.c_str(), NULL);
            }
        }
    };

    struct GridButton : app::SvgSwitch
    {
        GridButton()
        {
            momentary = true;
            shadow->opacity = 0;

            addFrame (rack::window::Window().loadSvg (asset::plugin (pluginInstance, "res/8X8_transparent.svg")));
        }
    };

    //context menus

    struct ClearMidiMappingMenuItem : MenuItem
    {
        IversonBase* module;

        void onAction (const event::Action& e) override
        {
            module->iverson->isClearMapping = true;
            module->iverson->isClearAllMapping = false;
            module->iverson->isSetLength = false;
            module->iverson->isLearning = true;
        }
    };

    struct ClearMAllMidiMappingMenuItem : MenuItem
    {
        IversonBase* module;

        void onAction (const event::Action& e) override
        {
            module->iverson->isClearMapping = false;
            module->iverson->isClearAllMapping = true;
            module->iverson->isSetLength = false;
            module->iverson->isLearning = false;
        }
    };

    struct ProbabilityNotchMenuItem : MenuItem
    {
        float notch = 0.0f;
        IversonBase* module;

        void onAction (const event::Action& e) override
        {
            module->iverson->params[Comp::PROB_NOTCH_WIDTH].setValue (notch);
        }
    };

    struct MidiLearnParamFirstMenuItem : MenuItem
    {
        IversonBase* module;
        void onAction (const event::Action& e) override
        {
            module->iverson->params[Comp::MIDI_LEARN_PARAM_FIRST]
                .setValue (! (bool) module->iverson->params[Comp::MIDI_LEARN_PARAM_FIRST].getValue());
        }
    };

    struct UseRotaryEncodersMenuItem : MenuItem
    {
        IversonBase* module;
        void onAction (const event::Action& e) override
        {
            module->iverson->params[Comp::USE_ROTARY_ENCODERS_PARAM]
                .setValue (! (bool) module->iverson->params[Comp::USE_ROTARY_ENCODERS_PARAM].getValue());
        }
    };

    struct MidiFeedbackDividerMenuItem : MenuItem
    {
        IversonBase* module;

        void onAction (const event::Action& e) override
        {
            module->iverson->params[Comp::MIDI_FEEDBACK_DIVIDER_SLOW]
                .setValue (! (bool) module->iverson->params[Comp::MIDI_FEEDBACK_DIVIDER_SLOW].getValue());
        }
    };

    struct MidiVelocityQuantity : Quantity
    {
        IversonBase* module;
        Comp::ParamIds paramId = Comp::NUM_PARAMS;

        void setValue (float value) override
        {
            if (module != nullptr)
                module->params[paramId].setValue (clamp ((int) value, 0, 127));
        }
        float getValue() override
        {
            if (module == nullptr)
                return 0.0f;
            return module->params[paramId].getValue();
        }
        float getMinValue() override
        {
            return 0.0f;
        }
        float getMaxValue() override
        {
            return 127.0f;
        }
        float getDefaultValue() override
        {
            return 0;
        }
        int getDisplayPrecision() override
        {
            return 3;
        }
        std::string getLabel() override
        {
            if (module == nullptr)
                return "";
            return module->paramQuantities[paramId]->getLabel();
        }
    };

    struct MidiVelocitySlider : ui::Slider
    {
        MidiVelocitySlider()
        {
            quantity = new MidiVelocityQuantity;
        }

        ~MidiVelocitySlider()
        {
            delete quantity;
        }
    };

    struct IversonBaseWidget : ModuleWidget
    {
        int trackCount = 8;
        int gridWidth = 0;
        float gridX = 0.0f;
        float pageX = 0.0f;
        float muteX = 198.00f;
        float triggerX = 211.06f;
        int midiSelectorCount = 0;
        std::string filename;
        float summaryLength = 0;
        float summaryX = 0;
        std::vector<float> midiSelectorX;
        float primaryProbX = 0.0f;
        float altProbX = 0.0f;
        float altOutX = 0.0f;

        explicit IversonBaseWidget (IversonBase* module);

        void init (IversonBase* module);

        /**
		 * helper function create and add a MidiWidget to the current widget;
		 * @param module
		 * @param port The midi queue
		 * @return
		 */
        MidiWidget* createMidiWidget (const IversonBase* module, midi::Port* port, Vec pos);

        void appendContextMenu (Menu* menu) override;
    };

    void IversonBaseWidget::init (IversonBase* module)
    {
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (40 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, filename.c_str());

        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (
            Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        //parameter grid inputs
        Vec grid_1_1 (gridX, 23.7f);
        auto gridXDelta = 8.5f;
        auto gridYDelta = 8.35f;

        for (auto t = 0; t < trackCount; ++t)
        {
            addParam (SqHelper::createParamCentered<sspo::SmallKnob> (icomp,
                                                                      mm2px (Vec (primaryProbX,
                                                                                  grid_1_1.y + t * gridYDelta)),
                                                                      module,
                                                                      Comp::PRIMARY_PROB_1 + t));
            addParam (SqHelper::createParamCentered<sspo::SmallKnob> (icomp,
                                                                      mm2px (Vec (altProbX,
                                                                                  grid_1_1.y + t * gridYDelta)),
                                                                      module,
                                                                      Comp::ALT_PROB_1 + t));

            for (auto s = 0; s < gridWidth; ++s)
            {
                addParam (SqHelper::createParamCentered<GridButton> (icomp,
                                                                     mm2px (Vec (grid_1_1.x + s * gridXDelta,
                                                                                 grid_1_1.y + t * gridYDelta)),
                                                                     module,
                                                                     Comp::GRID_1_1_PARAM + t * gridWidth + s));

                auto* gridWidget = createWidget<GridWidget> (mm2px (Vec (grid_1_1.x + s * gridXDelta - 4.0f,
                                                                         grid_1_1.y + t * gridYDelta - 3.5f)));
                gridWidget->box.size = mm2px (Vec (8, 7));
                gridWidget->setGridLocation (s, t);
                gridWidget->setModule (module);
                addChild (gridWidget);
            }
            addParam (SqHelper::createParamCentered<LEDButton> (icomp,
                                                                mm2px (Vec (muteX, grid_1_1.y + t * gridYDelta)),
                                                                module,
                                                                Comp::ACTIVE_1_PARAM + t));

            addChild (createLightCentered<LargeLight<GreenLight>> (mm2px (Vec (muteX, grid_1_1.y + t * gridYDelta)),
                                                                   module,
                                                                   Comp::ACTIVE_1_LIGHT + t));

            addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (triggerX, grid_1_1.y + t * gridYDelta)),
                                                         module,
                                                         Comp::TRIGGER_1_OUTPUT + t));
            addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (altOutX, grid_1_1.y + t * gridYDelta)),
                                                         module,
                                                         Comp::ALT_OUTPUT_1 + t));
        }

        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (pageX, 23.70)), module, Comp::PAGE_ONE_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (pageX, 32.05)), module, Comp::PAGE_TWO_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (pageX, 40.40)), module, Comp::PAGE_THREE_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (pageX, 48.74)), module, Comp::PAGE_FOUR_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (18.57, 118.0)), module, Comp::RESET_PARAM));
        addParam (
            SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (18.57, 102)), module, Comp::CLOCK_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (pageX, 65.45)), module, Comp::SET_LENGTH_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (pageX, 82.15)), module, Comp::MIDI_LEARN_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (triggerX + 5, 102)), module, Comp::SET_EUCLIDEAN_HITS_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (triggerX + 5, 118)), module, Comp::ROTATE_TRACK_PARAM));

        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (8.57, 118.0)), module, Comp::RESET_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (8.57, 102)), module, Comp::CLOCK_INPUT));

        addChild (createLightCentered<LargeLight<RedLight>> (mm2px (Vec (pageX, 23.70)), module, Comp::PAGE_ONE_LIGHT));
        addChild (createLightCentered<LargeLight<RedLight>> (mm2px (Vec (pageX, 32.05)), module, Comp::PAGE_TWO_LIGHT));
        addChild (createLightCentered<LargeLight<RedLight>> (mm2px (Vec (pageX, 40.40)), module, Comp::PAGE_THREE_LIGHT));
        addChild (
            createLightCentered<LargeLight<RedLight>> (mm2px (Vec (pageX, 48.74)), module, Comp::PAGE_FOUR_LIGHT));
        addChild (createLightCentered<LargeLight<RedLight>> (mm2px (Vec (18.57, 118.0)), module, Comp::RESET_LIGHT));
        addChild (createLightCentered<LargeLight<RedLight>> (mm2px (Vec (18.57, 102.0)), module, Comp::CLOCK_LIGHT));
        addChild (createLightCentered<LargeLight<RedLight>> (mm2px (Vec (pageX, 65.45)), module, Comp::SET_LENGTH_LIGHT));
        addChild (createLightCentered<LargeLight<RedLight>> (mm2px (Vec (pageX, 82.15)), module, Comp::MIDI_LEARN_LIGHT));
        addChild (createLightCentered<LargeLight<RedLight>> (mm2px (Vec (triggerX + 5, 102)), module, Comp::SET_EUCLIDEAN_HITS_LIGHT));
        addChild (createLightCentered<LargeLight<RedLight>> (mm2px (Vec (triggerX + 5, 118)), module, Comp::ROTATE_TRACK_LIGHT));

        if (module != nullptr)
        {
            createMidiWidget (module, &module->midiInputQueues[0], Vec (midiSelectorX[0], 98.094));
            createMidiWidget (module, &module->midiOutputs[0], Vec (midiSelectorX[1], 98.094));
            if (midiSelectorCount == 2)
            {
                createMidiWidget (module, &module->midiInputQueues[1], Vec (midiSelectorX[2], 98.094));
                createMidiWidget (module, &module->midiOutputs[1], Vec (midiSelectorX[3], 98.094));
            }
        }

        auto* summaryWidget = createWidget<SummaryWidget> (mm2px (Vec (summaryX, 87.5)));
        summaryWidget->box.size = mm2px (Vec (summaryLength, 4));
        summaryWidget->setModule (module);
        addChild (summaryWidget);
    }

    IversonBaseWidget::IversonBaseWidget (IversonBase* module)
    {
        setModule (module);
    }

    MidiWidget* IversonBaseWidget::createMidiWidget (const IversonBase* module, midi::Port* port, Vec pos)
    {
        auto* midiAInWidget = createWidget<MidiWidget> (mm2px (pos));
        midiAInWidget->box.size = mm2px (Vec (40, 25));
        midiAInWidget->setMidiPort (module ? port : NULL);
        addChild (midiAInWidget);
        return midiAInWidget;
    }

    void IversonBaseWidget::appendContextMenu (Menu* menu)
    {
        auto* module = dynamic_cast<IversonBase*> (this->module);

        menu->addChild (new MenuEntry);

        auto* useRotaryEncoderMenuItem = new UseRotaryEncodersMenuItem();
        useRotaryEncoderMenuItem->text = "Use Rotary Encoders";
        useRotaryEncoderMenuItem->module = (IversonBase*) module;
        useRotaryEncoderMenuItem->rightText = CHECKMARK (
            ((IversonBase*) module)->iverson->params[Comp::USE_ROTARY_ENCODERS_PARAM].getValue());
        menu->addChild (useRotaryEncoderMenuItem);

        auto* clearAllMenuItem = new ClearMAllMidiMappingMenuItem();
        clearAllMenuItem->text = "Clear all Midi Mappings";
        clearAllMenuItem->module = (IversonBase*) module;
        menu->addChild (clearAllMenuItem);

        auto* clearMidiMenuItem = new ClearMidiMappingMenuItem();
        clearMidiMenuItem->text = "Clear Midi Mapping";
        clearMidiMenuItem->module = (IversonBase*) module;
        menu->addChild (clearMidiMenuItem);

        auto* midiParamFirst = new MidiLearnParamFirstMenuItem();
        midiParamFirst->module = (IversonBase*) module;
        midiParamFirst->text = "Midi Learn Parameter First";
        midiParamFirst->rightText = CHECKMARK (
            ((IversonBase*) module)->iverson->params[Comp::MIDI_LEARN_PARAM_FIRST].getValue());
        menu->addChild (midiParamFirst);

        auto* slowMidiFeedback = new MidiFeedbackDividerMenuItem();
        slowMidiFeedback->module = (IversonBase*) module;
        slowMidiFeedback->text = "Slow midi feedback";
        slowMidiFeedback->rightText = CHECKMARK (
            ((IversonBase*) module)->iverson->params[Comp::MIDI_FEEDBACK_DIVIDER_SLOW].getValue());
        menu->addChild (slowMidiFeedback);

        auto* midiVelNoneSlider = new MidiVelocitySlider;
        dynamic_cast<MidiVelocityQuantity*> (midiVelNoneSlider->quantity)->module = module;
        dynamic_cast<MidiVelocityQuantity*> (midiVelNoneSlider->quantity)->paramId = Comp::MIDI_FEEDBACK_VELOCITY_NONE;
        midiVelNoneSlider->box.size.x = 200.0f;
        menu->addChild (midiVelNoneSlider);

        auto* midiVelStepSlider = new MidiVelocitySlider;
        dynamic_cast<MidiVelocityQuantity*> (midiVelStepSlider->quantity)->module = module;
        dynamic_cast<MidiVelocityQuantity*> (midiVelStepSlider->quantity)->paramId = Comp::MIDI_FEEDBACK_VELOCITY_STEP;
        midiVelStepSlider->box.size.x = 200.0f;
        menu->addChild (midiVelStepSlider);

        auto* midiVelIndexSlider = new MidiVelocitySlider;
        dynamic_cast<MidiVelocityQuantity*> (midiVelIndexSlider->quantity)->module = module;
        dynamic_cast<MidiVelocityQuantity*> (midiVelIndexSlider->quantity)->paramId = Comp::MIDI_FEEDBACK_VELOCITY_INDEX;
        midiVelIndexSlider->box.size.x = 200.0f;
        menu->addChild (midiVelIndexSlider);

        auto* midiVelLoopSlider = new MidiVelocitySlider;
        dynamic_cast<MidiVelocityQuantity*> (midiVelLoopSlider->quantity)->module = module;
        dynamic_cast<MidiVelocityQuantity*> (midiVelLoopSlider->quantity)->paramId = Comp::MIDI_FEEDBACK_VELOCITY_LOOP;
        midiVelLoopSlider->box.size.x = 200.0f;
        menu->addChild (midiVelLoopSlider);

        auto* midiVelLoopStepSlider = new MidiVelocitySlider;
        dynamic_cast<MidiVelocityQuantity*> (midiVelLoopStepSlider->quantity)->module = module;
        dynamic_cast<MidiVelocityQuantity*> (midiVelLoopStepSlider->quantity)->paramId = Comp::MIDI_FEEDBACK_VELOCITY_LOOP_STEP;
        midiVelLoopStepSlider->box.size.x = 200.0f;
        menu->addChild (midiVelLoopStepSlider);

        menu->addChild (new MenuEntry);

        auto* notchWidthLabel = new MenuLabel();
        notchWidthLabel->text = "Probability Notch Width";
        menu->addChild (notchWidthLabel);

        auto* noNotch = new ProbabilityNotchMenuItem();
        noNotch->notch = 0.0f;
        noNotch->text = "None  0%";
        noNotch->module = (IversonBase*) module;
        noNotch->rightText = CHECKMARK (((IversonBase*) module)->iverson->params[Comp::PROB_NOTCH_WIDTH].getValue()
                                        == noNotch->notch);
        menu->addChild (noNotch);

        auto* slimNotch = new ProbabilityNotchMenuItem();
        slimNotch->notch = 0.20f / 2.0f;
        slimNotch->text = "Slim  20%";
        slimNotch->module = (IversonBase*) module;
        slimNotch->rightText = CHECKMARK (
            ((IversonBase*) module)->iverson->params[Comp::PROB_NOTCH_WIDTH].getValue()
            == slimNotch->notch);
        menu->addChild (slimNotch);

        auto* midNotch = new ProbabilityNotchMenuItem();
        midNotch->notch = 0.35f / 2.0f;
        midNotch->text = "Mid  35%";
        midNotch->module = (IversonBase*) module;
        midNotch->rightText = CHECKMARK (
            ((IversonBase*) module)->iverson->params[Comp::PROB_NOTCH_WIDTH].getValue()
            == midNotch->notch);
        menu->addChild (midNotch);

        auto* wideNotch = new ProbabilityNotchMenuItem();
        wideNotch->notch = 0.50f / 2.0f;
        wideNotch->text = "Wide  50%";
        wideNotch->module = (IversonBase*) module;
        wideNotch->rightText = CHECKMARK (
            ((IversonBase*) module)->iverson->params[Comp::PROB_NOTCH_WIDTH].getValue()
            == wideNotch->notch);
        menu->addChild (wideNotch);
    }

    struct IversonWidget : IversonBaseWidget
    {
        explicit IversonWidget (IversonBase* module) : IversonBaseWidget (module)
        {
            gridWidth = 16;
            muteX = 192.380f;
            triggerX = 202.53f;
            midiSelectorCount = 2;
            filename = "res/Iverson.svg";
            summaryLength = 130;
            summaryX = 47.0;
            midiSelectorX = { 23.12, 68.92, 114.72, 160.51 };
            primaryProbX = 20.99f;
            altProbX = 31.14f;
            altOutX = 212.7f;
            gridX = 48.26f;
            pageX = 10.82f;
            init (module);
        }
    };

    struct IversonJrWidget : IversonBaseWidget
    {
        explicit IversonJrWidget (IversonBase* module) : IversonBaseWidget (module)
        {
            gridWidth = 8;
            muteX = 126.34f;
            triggerX = 136.5f;
            midiSelectorCount = 1;
            filename = "res/IversonJr.svg";
            summaryLength = 62;
            summaryX = 48.0;
            midiSelectorX = { 35.90, 81.70, 0, 0 };
            primaryProbX = 20.99f;
            altProbX = 31.14f;
            altOutX = 146.66f;
            gridX = 49.26f;
            pageX = 10.82f;

            init (module);
        }
    };

} // namespace sspo
Model* modelIverson = createModel<sspo::Iverson, sspo::IversonWidget> ("Iverson"); // NOLINT(cert-err58-cpp)
Model* modelIversonJr = createModel<sspo::IversonJr, sspo::IversonJrWidget> ("IversonJr"); // NOLINT(cert-err58-cpp)