#include "plugin.hpp"
/* #include "common.hpp"
#include "module.hpp"
#include "componentlibrary.hpp" */
#include "widgets.h"
#include "Iverson.h"
#include "WidgetComposite.h"
#include "ctrl/SqMenuItem.h"
#include "app/MidiWidget.hpp"

namespace sspo
{
    using Comp = IversonComp<WidgetComposite>;
    using namespace ::rack;

    struct Iverson : Module
    {
        static constexpr int MAX_SEQUENCE_LENGTH = 64;
        std::shared_ptr<Comp> iverson;

        struct MidiOutput : midi::Output
        {
            int currentCC[Comp::MAX_MIDI];
            bool currentNotes[Comp::MAX_MIDI];

            MidiOutput()
            {
                reset();
            }

            void reset()
            {
                for (auto i = 0; i < Comp::MAX_MIDI; ++i)
                {
                    currentCC[i] = -1;
                    currentNotes[i] = false;
                }
            }

            void setCC (int cc, int val)
            {
                if (val == currentCC[cc])
                    return;
                currentCC[cc] = val;
                // create message
                midi::Message msg;
                msg.setStatus (0xb);
                msg.setNote (cc);
                msg.setValue (val);
                sendMessage (msg);
            }

            void resetNote (int note)
            {
                midi::Message msg;
                msg.setStatus (0x9);
                msg.setNote (note);
                msg.setValue (0);
                sendMessage (msg);
                currentNotes[note] = false;
            }

            void setNote (int note, int velocity)
            {
                if (velocity > 0)
                {
                    //note on
                    midi::Message msg;
                    msg.setStatus (0x9);
                    msg.setNote (note);
                    msg.setValue (velocity);
                    sendMessage (msg);
                }
                else if (velocity == 0)
                {
                    //note off
                    midi::Message msg;
                    msg.setStatus (0x9);
                    msg.setNote (note);
                    msg.setValue (0);
                    sendMessage (msg);
                }
                currentNotes[note] = velocity > 0;
            }
        };

        std::vector<midi::InputQueue> midiInputQueues{ 2 };
        std::vector<MidiOutput> midiOutputs{ 2 };

        dsp::ClockDivider controlPageUpdateDivider;
        dsp::ClockDivider paramMidiUpdateDivider;

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

        std::vector<MidiMapping> midiMappings;
        MidiMapping midiLearnMapping;

        Iverson()
        {
            config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);

            //        midiInputQueues.resize (2);
            //preallocate midi mappings, to avoid allocations during push_back
            // occurring during the audio process loop
            iverson = std::make_shared<Comp> (this);
            midiMappings.reserve (iverson->MIDI_MAP_SIZE);
            std::shared_ptr<IComposite> icomp = Comp::getDescription();
            SqHelper::setupParams (icomp, this);
            onSampleRateChange();
            iverson->init();

            controlPageUpdateDivider.setDivision (10000);
            paramMidiUpdateDivider.setDivision (100);
        }
        void onSampleRateChange() override
        {
            float rate = SqHelper::engineGetSampleRate();
            iverson->setSampleRate (rate);
        }

        json_t* dataToJson() override
        {
            json_t* rootJ = json_object();

            json_t* mutesJ = json_array();
            json_t* lengthsJ = json_array();
            json_t* indexJ = json_array();
            json_t* sequenceHiJ = json_array();
            json_t* sequenceLowJ = json_array();

            for (auto i = 0; i < iverson->TRACK_COUNT; ++i)
            {
                json_array_insert_new (mutesJ, i, json_boolean ((iverson->tracks[i].getMute())));
                json_array_insert_new (lengthsJ, i, json_integer (iverson->tracks[i].getLength()));
                json_array_insert_new (indexJ, i, json_integer (iverson->tracks[i].getIndex()));
                //sequence 64bit, json integers are 32bit, store ass hi low values
                json_array_insert_new (sequenceHiJ,
                                       i,
                                       json_integer (((iverson->tracks[i].getSequence().to_ulong()) >> 32u) & 0xffffffff));
                json_array_insert_new (sequenceLowJ,
                                       i,
                                       json_integer (((iverson->tracks[i].getSequence().to_ulong())) & 0xffffffff));
            }

            json_object_set_new (rootJ, "mutes", mutesJ);
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

            return rootJ;
        }
        void dataFromJson (json_t* rootJ) override
        {
            json_t* mutesJ = json_object_get (rootJ, "mutes");
            for (auto t = 0; t < iverson->TRACK_COUNT; ++t)
            {
                if (mutesJ)
                {
                    json_t* mutesArrayJ = json_array_get (mutesJ, t);
                    if (mutesArrayJ)
                        iverson->tracks[t].setMute (json_boolean_value (mutesArrayJ));
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
        }

        void doLearn();
        void process (const ProcessArgs& args) override
        {
            doLearn();
            if (paramMidiUpdateDivider.process())
            {
                updateMidiOutDeviceDriver();
                midiToParm();
            }

            iverson->step();
            if (controlPageUpdateDivider.process())
            {
                //            updateMidiOutDeviceDriver();
                pageLights();
            }
            //        doMidiOut();
        }

        /// Midi events are used to set assigned params
        /// midi handling would require linking to RACK for unit test
        /// hence all midi to be processed in Iverson.cpp
        void midiToParm();
        /// sends midi to external controllerr to show status
        void pageLights();
        void updateMidiOutDeviceDriver();
    };

    void Iverson::midiToParm()
    {
        midi::Message msg;
        for (auto q = 0; q < 2; ++q)
        {
            while (midiInputQueues[q].shift (&msg))

            {
                switch (msg.getStatus())
                {
                    //note off
                    case 0x8:
                    {
                        //find midiMapping for noteoff
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
                        //find midiMapping for noteon
                        for (auto& m : midiMappings)
                        {
                            //                            ::DEBUG ("NOTE ON, MIDIMappings controller %d  note %d  cc %d param %d",
                            //                                   m.controller,
                            //                                   m.note,
                            //                                   m.cc,
                            //                                   m.paramId);
                            if (m.note == msg.getNote() && m.controller == q)
                                params[m.paramId].setValue ((msg.getValue() == 0 ? 0 : 1));
                        }
                    }
                    break;
                        // cc
                    case 0xb:
                    {
                        //find midiMapping for cc
                        for (auto& m : midiMappings)
                        {
                            //                            DEBUG ("NOTE ON, MIDIMappings controller %d  note %d  cc %d param %d",
                            //                                   m.controller,
                            //                                   m.note,
                            //                                   m.cc,
                            //                                   m.paramId);
                            if (m.cc == msg.getNote() && m.controller == q)
                                params[m.paramId].setValue ((msg.getValue() == 0 ? 0 : 1));
                        }
                    }

                    break;
                    default:
                        break;
                }
            }
        }
    }

    void Iverson::doLearn()
    {
        if (iverson->isLearning)
        // if we have all required params, add to list
        {
            if ((midiLearnMapping.controller != -1)
                && (midiLearnMapping.cc != -1 || midiLearnMapping.note != -1)
                && midiLearnMapping.paramId != -1)
            {
                //delete any existing map to this parameter
                auto mm = std::find_if (midiMappings.begin(), midiMappings.end(), [this] (const MidiMapping& x) { return x.paramId == midiLearnMapping.paramId; });
                if (mm != midiMappings.end())
                    midiMappings.erase (mm);

                //delete any existing map to this midi note
                mm = std::find_if (midiMappings.begin(), midiMappings.end(), [this] (const MidiMapping& x) { return x.note != -1 && x.note == midiLearnMapping.note; });
                if (mm != midiMappings.end())
                    midiMappings.erase (mm);

                //delete any existing map to this midi cc
                mm = std::find_if (midiMappings.begin(), midiMappings.end(), [this] (const MidiMapping& x) { return x.cc != -1 && x.cc == midiLearnMapping.cc; });
                if (mm != midiMappings.end())
                    midiMappings.erase (mm);

                midiMappings.push_back (midiLearnMapping);
                midiLearnMapping.reset();
                iverson->isLearning = false;
            }
            // if midi add to midi learn param
            midi::Message msg;
            for (auto q = 0; q < 2; ++q)
            {
                while (midiInputQueues[q].shift (&msg))

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
                        }
                        break;
                        default:
                            break;
                    }
                }
            }

            //if param add to midi learn param
            for (auto i = (int) iverson->GRID_1_1_PARAM; i <= iverson->GRID_16_8_PARAM; ++i)
            {
                if ((int) iverson->params[i].getValue() != 0)
                {
                    midiLearnMapping.paramId = i;
                    return;
                }
            }
            for (auto i = (int) iverson->MUTE_1_PARAM; i <= iverson->MUTE_8_PARAM; ++i)
            {
                if ((int) iverson->params[i].getValue() != 0)
                {
                    midiLearnMapping.paramId = i;
                    return;
                }
            }

            for (auto i = (int) iverson->PAGE_ONE_PARAM; i <= iverson->PAGE_FOUR_PARAM; ++i)
            {
                if ((int) iverson->params[i].getValue() != 0)
                {
                    midiLearnMapping.paramId = i;
                    return;
                }
            }

            std::vector<int> learnableButtons = { iverson->SET_LENGTH_PARAM,
                                                  iverson->RESET_PARAM };

            for (auto lb : learnableButtons)
            {
                if ((int) iverson->params[lb].getValue() != 0)
                {
                    midiLearnMapping.paramId = lb;
                    return;
                }
            }
        }
    }

    void Iverson::pageLights()
    {
        for (auto& mm : midiMappings)
        //    pageLightIndex++;
        //    pageLightIndex /= midiMappings.size();
        //    auto mm = midiMappings[pageLightIndex];
        {
            //        midiOutputs[mm.controller].resetNote (mm.note);

            if (mm.paramId <= iverson->GRID_16_8_PARAM)
            // sequence
            {
                auto t = mm.paramId / iverson->GRID_WIDTH;
                auto i = mm.paramId - t * iverson->GRID_WIDTH;
                auto midiColor = 0;
                if (iverson->tracks[t].getIndex() != i)
                {
                    if (iverson->tracks[t].getLength() - 1 == i + iverson->page * iverson->GRID_WIDTH)
                    {
                        midiColor = iverson->getStateGridIndex (iverson->page, t, i)
                                        ? 5
                                        : 3;
                    }
                    else
                        midiColor = iverson->getStateGridIndex (iverson->page, t, i)
                                        ? 1
                                        : 0;
                }
                else
                    midiColor = 5;
                midiOutputs[mm.controller].setNote (mm.note, midiColor);
            }
            else if (mm.paramId >= iverson->MUTE_1_PARAM && mm.paramId <= iverson->MUTE_8_PARAM)
            {
                auto t = mm.paramId - iverson->MUTE_1_PARAM;
                midiOutputs[mm.controller].setNote (mm.note, iverson->tracks[t].getMute());
            }

            else if (mm.paramId >= iverson->PAGE_ONE_PARAM && mm.paramId <= iverson->PAGE_FOUR_PARAM)
            {
                auto pageIndex = mm.paramId - iverson->PAGE_ONE_PARAM;
                midiOutputs[mm.controller].setNote (mm.note, pageIndex == iverson->page);
            }
        }
    }
    void Iverson::updateMidiOutDeviceDriver()
    {
        for (auto i = 0; i < 2; ++i)
        {
            if (midiOutputs[i].driverId != midiInputQueues[i].driverId)
                midiOutputs[i].setDriverId (midiInputQueues[i].driverId);
            if (midiOutputs[i].deviceId != midiInputQueues[i].deviceId)
                midiOutputs[i].setDeviceId (midiInputQueues[i].deviceId);
            midiOutputs[i].setChannel (0);
        }
    }

    /*****************************************************
User Interface
*****************************************************/

    struct SummaryWidget : Widget
    {
        Iverson* module = nullptr;

        struct GridColors
        {
            NVGcolor none;
            NVGcolor on;
            NVGcolor loop;
            NVGcolor loopAndBeat;
            NVGcolor index;
            NVGcolor page;
        } gridColors;

        SummaryWidget()
        {
            gridColors.none = nvgRGBA (0, 0, 0, 255);
            gridColors.on = nvgRGBA (0, 255, 0, 255);
            gridColors.loop = nvgRGBA (255, 0, 0, 255);
            gridColors.loopAndBeat = nvgRGBA (255, 255, 0, 255);
            gridColors.index = nvgRGBA (255, 255, 0, 255);
            gridColors.page = nvgRGBA (255, 255, 255, 100);
        }

        void setModule (Iverson* module)
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
            auto beatWidth = box.size.x / Iverson::MAX_SEQUENCE_LENGTH;
            auto trackHeight = box.size.y / module->iverson->tracks.size();
            //        nvgBeginPath (args.vg);

            for (auto t = 0; t < int (module->iverson->tracks.size()); ++t)
            {
                //plot beats
                for (auto b = 0; b < Iverson::MAX_SEQUENCE_LENGTH; ++b)
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
            auto pageWidth = beatWidth * (Iverson::MAX_SEQUENCE_LENGTH / module->iverson->pages);
            nvgFillColor (args.vg, gridColors.page);
            nvgBeginPath (args.vg);
            nvgRect (args.vg, page * pageWidth, 0, pageWidth, box.size.y);

            nvgFill (args.vg);
        }
    };

    struct IversonWidget : ModuleWidget
    {
        IversonWidget (Iverson* module)
        {
            setModule (module);

            std::shared_ptr<IComposite> icomp = Comp::getDescription();
            box.size = Vec (30 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
            SqHelper::setPanel (this, "res/Iverson.svg");

            addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
            addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
            addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

            //parameter grid inputs
            Vec grid_1_1 (9.8f, 23.7f);
            constexpr auto gridXDelta = 8.5f;
            constexpr auto gridYDelta = 8.35f;
            constexpr auto trackCount = 8;
            constexpr auto gridWidth = 16;
            constexpr auto muteX = 147.2f;
            constexpr auto triggerX = 160.262f;

            for (auto t = 0; t < trackCount; ++t)
            {
                for (auto s = 0; s < gridWidth; ++s)
                {
                    addParam (SqHelper::createParamCentered<LEDButton> (icomp,
                                                                        mm2px (Vec (grid_1_1.x + s * gridXDelta, grid_1_1.y + t * gridYDelta)),
                                                                        module,
                                                                        Comp::GRID_1_1_PARAM + t * gridWidth + s));
                    addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (grid_1_1.x + s * gridXDelta, grid_1_1.y + t * gridYDelta)),
                                                                          module,
                                                                          Comp::GRID_1_1_LIGHT + t * gridWidth + s));
                }
                addParam (SqHelper::createParamCentered<LEDButton> (icomp,
                                                                    mm2px (Vec (muteX, grid_1_1.y + t * gridYDelta)),
                                                                    module,
                                                                    Comp::MUTE_1_PARAM + t));

                addChild (createLightCentered<MediumLight<GreenLight>> (mm2px (Vec (muteX, grid_1_1.y + t * gridYDelta)),
                                                                        module,
                                                                        Comp::MUTE_1_LIGHT + t));

                addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (triggerX, grid_1_1.y + t * gridYDelta)),
                                                             module,
                                                             Comp::TRIGGER_1_OUTPUT + t));
            }

            addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (9.807, 100.475)), module, Comp::PAGE_ONE_PARAM));
            addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (16.157, 100.475)), module, Comp::PAGE_TWO_PARAM));
            addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (22.507, 100.475)), module, Comp::PAGE_THREE_PARAM));
            addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (28.857, 100.475)), module, Comp::PAGE_FOUR_PARAM));
            addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (147.211, 102.013)), module, Comp::RESET_PARAM));
            addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (160.262, 102.013)), module, Comp::CLOCK_PARAM));
            addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (9.807, 112.101)), module, Comp::SET_LENGTH_PARAM));
            addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (28.857, 112.101)), module, Comp::MIDI_LEARN_PARAM));

            addInput (createInputCentered<PJ301MPort> (mm2px (Vec (147.212, 112.101)), module, Comp::RESET_INPUT));
            addInput (createInputCentered<PJ301MPort> (mm2px (Vec (160.262, 112.101)), module, Comp::CLOCK_INPUT));

            addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 100.475)), module, Comp::PAGE_ONE_LIGHT));
            addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 100.475)), module, Comp::PAGE_TWO_LIGHT));
            addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 100.475)), module, Comp::PAGE_THREE_LIGHT));
            addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 100.475)), module, Comp::PAGE_FOUR_LIGHT));
            addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (147.211, 102.013)), module, Comp::RESET_LIGHT));
            addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (160.262, 102.013)), module, Comp::CLOCK_LIGHT));
            addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 112.101)), module, Comp::SET_LENGTH_LIGHT));
            addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 112.101)), module, Comp::MIDI_LEARN_LIGHT));

            MidiWidget* midiAWidget = createWidget<MidiWidget> (mm2px (Vec (43.23, 98.094)));
            midiAWidget->box.size = mm2px (Vec (40, 25));
            midiAWidget->setMidiPort (module ? &module->midiInputQueues[0] : NULL);
            addChild (midiAWidget);

            MidiWidget* midiBWidget = createWidget<MidiWidget> (mm2px (Vec (89.6, 98.094)));
            midiBWidget->box.size = mm2px (Vec (40, 25));
            midiBWidget->setMidiPort (module ? &module->midiInputQueues[1] : NULL);
            addChild (midiBWidget);

            SummaryWidget* summaryWidget = createWidget<SummaryWidget> (mm2px (Vec (8.5, 87.5)));

            summaryWidget->box.size = mm2px (Vec (130, 4));
            summaryWidget->setModule (module);
            addChild (summaryWidget);

            /*         // mm2px(Vec(60.747, 69.094))
        addChild (createWidget<Widget> (mm2px (Vec (4.515, 18.372))));

        // mm2px(Vec(60.747, 69.094))
        addChild (createWidget<Widget> (mm2px ()));
        // mm2px(Vec(39.881, 16.094))
        addChild (createWidget<Widget> (mm2px (Vec (38.646, 98.094))));
        // mm2px(Vec(39.881, 16.094))
        addChild (createWidget<Widget> (mm2px (Vec (85.02, 98.094)))); */
        }
    };
} // namespace sspo
Model* modelIverson = createModel<sspo::Iverson, sspo::IversonWidget> ("Iverson");