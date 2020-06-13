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

#include "IComposite.h"
#include "AudioMath.h"
#include "HardLimiter.h"
#include "easing.h"
#include <memory>
#include <vector>
#include <cstdlib>

namespace rack
{
    namespace engine
    {
        struct Module;
    }
} // namespace rack
using Module = ::rack::engine::Module;
using namespace rack;

template <class TBase>
class ZazelDescription : public IComposite
{
public:
    Config getParam (int i) override;
    int getNumParams() override;
};

/**
 * Complete Zazel composite
 *
 * If TBase is WidgetComposite, this class is used as the implementation part of the KSDelay module.
 * If TBase is TestComposite, this class may stand alone for unit tests.
 */

template <class TBase>
class ZazelComp : public TBase
{
public:
    ZazelComp (Module* module) : TBase (module)
    {
    }

    ZazelComp() : TBase()
    {
    }

    virtual ~ZazelComp()
    {
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<ZazelDescription<TBase>>();
    }

    enum ParamIds
    {
        EASING_ATTENUVERTER_PARAM,
        EASING_PARAM,
        START_ATTENUVERTER_PARAM,
        START_PARAM,
        END_ATTENUVERTER_PARAM,
        END_PARAM,
        DURATION_ATTENUVERTER_PARAM,
        DURATION_PARAM,
        ONESHOT_PARAM,
        SYNC_BUTTON_PARAM,
        TRIG_BUTTON_PARAM,
        PAUSE_BUTTON_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        EASING_INPUT,
        START_INPUT,
        END_INPUT,
        DURATION_INPUT,
        STOP_CONT_INPUT,
        CLOCK_INPUT,
        START_CONT_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        MAIN_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        PAUSE_LIGHT,
        NUM_LIGHTS
    };

    enum class Mode
    {
        ONESHOT_ATTACK,
        ONESHOT_HIGH,
        ONESHOT_DECAY,
        ONESHOT_LOW,
        CYCLE_ATTACK,
        CYCLE_DECAY,
        PAUSED,
        LEARN_END,
        COUNT
    };

    //member variables

    float sampleRate = 1.0f;
    float sampleTime = 1.0f;
    std::vector<Easings::Easing> easings;
    int currentEasing = 0.0;
    bool oneShot = true;
    int lastClockDuration = 1000;
    dsp::SchmittTrigger syncTrigger;
    dsp::SchmittTrigger startContTrigger;
    dsp::SchmittTrigger pauseTrigger;
    int framesSinceSync = 0;
    Mode mode = Mode::ONESHOT_LOW;
    Mode lastMode = Mode::ONESHOT_LOW;
    int framesSincePhaseChange = 0;
    Easings::EasingFactory ef;
    float out = 0.0f;
    float framePhaseDuration = 1.0f;
    int framePhaseCount = 0;
    float startParam = 0.0f;
    float endParam = 0.0f;

    void changePhase (Mode m)
    {
        framesSincePhaseChange = 0;
        mode = m;
    }

    int getCurrentEasing()
    {
        return clamp (currentEasing,
                      0,
                      int (Easings::EasingFactory::EasingNames::EASING_COUNT) - 1);
    }

    void setSampleRate (float rate)
    {
        sampleRate = rate;
        sampleTime = 1.0f / rate;
    }

    void syncClock()
    {
        if (TBase::inputs[CLOCK_INPUT].isConnected())
        {
            if (syncTrigger.process (TBase::inputs[CLOCK_INPUT].getVoltage()
                                     + TBase::params[SYNC_BUTTON_PARAM].getValue()))
            {
                framesSinceSync++;
                lastClockDuration = framesSinceSync;
                framesSinceSync = 0;
            }
            else
            {
                framesSinceSync++;
            }
        }
        else
            lastClockDuration = sampleRate;
    }

    void checkOneShotMode()
    {
        bool newOneShot = TBase::params[ONESHOT_PARAM].getValue();

        if (! newOneShot && oneShot)
            changePhase (Mode::CYCLE_ATTACK);
        if (newOneShot && ! oneShot)
            changePhase (Mode::PAUSED);

        oneShot = newOneShot;
    }

    void doTriggers()
    {
        if (startContTrigger.process (TBase::inputs[START_CONT_INPUT].getVoltage()
                                      + TBase::params[TRIG_BUTTON_PARAM].getValue()))
        {
            framesSincePhaseChange = 0;
            if (! oneShot)
                changePhase (Mode::CYCLE_ATTACK);

            else
            {
                changePhase (Mode::ONESHOT_ATTACK);
            }
        }

        //neagative gate

        if (TBase::inputs[START_CONT_INPUT].getVoltage()
                    + +TBase::params[TRIG_BUTTON_PARAM].getValue()
                < 1.0f
            && (mode == Mode::ONESHOT_ATTACK || mode == Mode::ONESHOT_HIGH))
        {
            changePhase (Mode::ONESHOT_DECAY);
        }
    }

    void doStateMachine()
    {
        auto easing = ef.getEasingVector().at (clamp (currentEasing, 0, 10));

        switch (mode)
        {
            case Mode::ONESHOT_ATTACK:
                out = easing->easeOut (framesSincePhaseChange,
                                       startParam,
                                       endParam - startParam,
                                       framePhaseCount);

                if (framesSincePhaseChange > framePhaseCount)
                    changePhase (Mode::ONESHOT_HIGH);
                break;

            case Mode::ONESHOT_HIGH:
                out = endParam;
                break;

            case Mode::ONESHOT_DECAY:
                out = easing->easeIn (framePhaseCount - framesSincePhaseChange,
                                      startParam,
                                      endParam - startParam,
                                      framePhaseCount);
                if (framesSincePhaseChange > framePhaseCount)
                    changePhase (Mode::ONESHOT_LOW);
                break;

            case Mode::ONESHOT_LOW:
                out = startParam;
                break;

            case Mode::CYCLE_ATTACK:
                out = easing->easeInOut (framesSincePhaseChange,
                                         startParam,
                                         endParam - startParam,
                                         framePhaseCount / 2.0);
                if (framesSincePhaseChange > framePhaseCount / 2.0)
                    changePhase (Mode::CYCLE_DECAY);
                break;

            case Mode::CYCLE_DECAY:
                out = easing->easeInOut ((framePhaseCount / 2.0) - framesSincePhaseChange,
                                         startParam,
                                         endParam - startParam,
                                         framePhaseCount / 2.0);
                if (framesSincePhaseChange > framePhaseCount / 2.0)
                    changePhase (Mode::CYCLE_ATTACK);
                break;
            case Mode::PAUSED:
                break;
            case Mode::LEARN_END:
                out = endParam;
                break;
            default:
                break;
        }
    }

    void calcParameters()
    {
        currentEasing = TBase::params[EASING_PARAM].getValue()
                        + TBase::params[EASING_ATTENUVERTER_PARAM].getValue()
                              * TBase::inputs[EASING_INPUT].getVoltage();

        framePhaseDuration = TBase::params[DURATION_PARAM].getValue()
                             + TBase::params[DURATION_ATTENUVERTER_PARAM].getValue()
                                   * TBase::inputs[DURATION_INPUT].getVoltage() / 5.0f;

        startParam = TBase::params[START_PARAM].getValue()
                     + TBase::params[START_ATTENUVERTER_PARAM].getValue()
                           * TBase::inputs[START_INPUT].getVoltage() / 5.0f;

        endParam = TBase::params[END_PARAM].getValue()
                   + TBase::params[END_ATTENUVERTER_PARAM].getValue()
                         * TBase::inputs[END_INPUT].getVoltage() / 5.0f;

        framePhaseDuration = clamp (framePhaseDuration, 0.001f, 5.0f);
        framePhaseCount = framePhaseDuration * lastClockDuration;
    }

    void doPause()
    {
        if (pauseTrigger.process (TBase::inputs[STOP_CONT_INPUT].getVoltage()
                                  + TBase::params[PAUSE_BUTTON_PARAM].getValue()))
        {
            if (mode == Mode::PAUSED)
                mode = lastMode;
            else
            {
                lastMode = mode;
                mode = Mode::PAUSED;
            }
        }
    }

    void setStartParamScaled (float x)
    {
        TBase::paramQuantities[START_PARAM]->setScaledValue (x);
    }

    void setEndParamScaled (float x)
    {
        TBase::paramQuantities[END_PARAM]->setScaledValue (x);
    }

    // must be called after setSampleRate
    void init()
    {
        lastClockDuration = sampleRate;
    }

    void step() override;
};

template <class TBase>
inline void ZazelComp<TBase>::step()
{
    doPause();
    syncClock();
    checkOneShotMode();
    doTriggers();
    if (mode != Mode::PAUSED)
    {
        framesSincePhaseChange++;
        calcParameters();
        doStateMachine();
        TBase::lights[PAUSE_LIGHT].setSmoothBrightness (0.0f, 5e-6f);
    }
    else
    {
        TBase::lights[PAUSE_LIGHT].setSmoothBrightness (1.0f, 5e-6f);
    }

    TBase::outputs[MAIN_OUTPUT].setVoltage (sspo::voltageSaturate (out * 10.0f));
}

template <class TBase>
int ZazelDescription<TBase>::getNumParams()
{
    return ZazelComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config ZazelDescription<TBase>::getParam (int i)
{
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Code type", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        case ZazelComp<TBase>::EASING_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "Easing Cv", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::EASING_PARAM:
            ret = { 0.0f, 10.0f, 0.0f, "Easing", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::START_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.f, "Start Cv", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::START_PARAM:
            ret = { -1.0f, 1.0f, 0.f, "Start", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::END_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.0f, "End Cv", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::END_PARAM:
            ret = { -1.0f, 1.0f, 0.f, "End", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::DURATION_ATTENUVERTER_PARAM:
            ret = { -1.0f, 1.0f, 0.f, "Duration Cv", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::DURATION_PARAM:
            ret = { 0.0f, 10.0f, 0.5f, "Duration", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::ONESHOT_PARAM:
            ret = { -1.0f, 0.0f, 0.0f, "Oneshot / cycle ", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::SYNC_BUTTON_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Sync", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::TRIG_BUTTON_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Trigger", " ", 0, 1, 0.0f };
            break;
        case ZazelComp<TBase>::PAUSE_BUTTON_PARAM:
            ret = { 0.0f, 1.0f, 0.0f, "Pause", " ", 0, 1, 0.0f };
            break;
        default:
            assert (false);
    }
    return ret;
}