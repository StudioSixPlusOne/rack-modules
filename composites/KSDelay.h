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
#include "CircularBuffer.h"
#include "HardLimiter.h"
#include "AD.h"

#include <cstdlib>
#include <vector>

namespace rack {
    namespace engine {
        struct Module;
    }
}
using Module = ::rack::engine::Module;

template <class TBase>
class KSDelayDescription : public IComposite
{
public:
    Config getParam(int i) override;
    int getNumParams() override;
};

/**
 * Complete KSDelay composite
 *
 * If TBase is WidgetComposite, this class is used as the implementation part of the KSDelay module.
 * If TBase is TestComposite, this class may stand alone for unit tests.
 */
template <class TBase>
class KSDelayComp : public TBase
{
public:
    KSDelayComp(Module * module) : TBase(module)
    {
    }

    KSDelayComp() : TBase()
    {
    }

    virtual ~KSDelayComp()
    {
    }


    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<KSDelayDescription<TBase>>();
    }

    void setSampleRate(float rate)
    {
        reciprocalSampleRate = 1 / rate;
        sampleRate = rate;

        for (auto &dc : dcInFilters)
			dc.setParameters (rack::dsp::BiquadFilter::HIGHPASS, dcInFilterCutoff / rate, 0.141f, 1.0f);

		for (auto&l : limiters)
			l.setSampleRate (rate);

		for (auto& a : ads)
			a.setSampleRate (rate);
    }

    // must be called after setSampleRate
    void init()
    {
		random::init();
		buffers.resize (maxChannels);
		for (auto& b : buffers)
			b.reset (4096);

		lowpassFilters.resize (maxChannels);

		dcInFilters.resize (maxChannels);
		for (auto &dc : dcInFilters)
			dc.setParameters( rack::dsp::BiquadFilter::HIGHPASS, dcInFilterCutoff / sampleRate, 0.141f, 1.0f);

		dcOutFilters.resize (maxChannels);
		for (auto &dc : dcOutFilters)
			dc.setParameters( rack::dsp::BiquadFilter::HIGHPASS, dcOutFilterCutoff / sampleRate, 0.141f, 1.0f);

		lastWets.resize (maxChannels);
		for (auto& lw : lastWets)
			lw = 0;

		delayTimes.resize (maxChannels);
		for (auto& d :delayTimes)
			d = 0;

		limiters.resize (maxChannels);
		for (auto&l : limiters)
		{
			l.setTimes (0.01f, 0.25f);
			l.setSampleRate (sampleRate);
			l.threshold = -6.0f;
		}

		oscphases.resize (maxChannels);
		for (auto& perChannel : oscphases)
		{
			perChannel.resize (maxOscCount);
			for (auto& phase : perChannel)
			{
				phase = static_cast<float> (std::rand()) / RAND_MAX;
			}	
		}

		triggers.resize(maxChannels);

		resetPending.resize(maxChannels);
		for (auto i = 0; i < maxChannels; ++ i)
			resetPending[i] = false;

		lastDelayTimes.resize (maxChannels);
		fadeLevels.resize (maxChannels);
		for (auto& fl : fadeLevels)
			fl = 1.0f;

		glide.resize (maxChannels);
		for (auto& g : glide)
			g.setRiseFall (0.01f, 0.01f);

		framesToSample.resize (maxChannels);
		for (auto& f : framesToSample)
		 f = 0;

		ads.resize (maxChannels);
		for (auto& ad : ads)
		{
			ad.setSampleRate (sampleRate);
			ad.setParameters(0.1f, 0.1f);
		}

		lastOut.resize (maxChannels);
		for (auto& lo : lastOut)
			lo = 0.000f;

		pitches.resize (maxChannels);
		for (auto& p : pitches)
			p = dsp::FREQ_C4;

		unisonTunings = { 0.0f, -0.01952356f, 0.01991221f, -0.06288439f, 0.06216538f, -0.11002313f, 0.10745242f};
    }

	//supersaw curves from "How to emulate the supersaw, Adam Szabo"
	// 0.0f <= x <= 1

	float unisonCentreLevel(const float x)
	{
		return -0.55366 * x + 0.99785;
	}

	float unisonSideLevel(const float x)
	{
		return -0.73764 * x * x + 1.2841 * x +0.044372;
	}

	float unisonSpreadScalar (const float x)
	{
		return (10028.7312891634*std::pow(x,11))-(50818.8652045924*std::pow(x,10))+(111363.4808729368*std::pow(x,9))-
			(138150.6761080548*std::pow(x,8))+(106649.6679158292*std::pow(x,7))-(53046.9642751875*std::pow(x,6))+(17019.9518580080*std::pow(x,5))-
			(3425.0836591318*std::pow(x,4))+(404.2703938388*std::pow(x,3))-(24.1878824391*std::pow(x,2))+(0.6717417634*x)+0.0030115596;
	}
	

    // Define all the enums here. This will let the tests and the widget access them.
	
    enum ParamIds 
	{
		OCTAVE_PARAM,
		TUNE_PARAM,
		FEEDBACK_PARAM,
		FILTER_PARAM,
		MIX_PARAM,
		UNISON_PARAM,
		UNISON_SPREAD_PARAM,
		UNISON_MIX_PARAM,
		GLIDE_PARAM,
		ATTACK_PARAM,
		DECAY_PARAM,
		PITCH_LOCK_PARAM,
		NUM_PARAMS
	};

	enum InputIds 
	{
		VOCT,
		FILTER_INPUT,
		FEEDBACK_INPUT,
		MIX_INPUT,
		IN_INPUT,
		TRIGGER_INPUT,
		UNISON_INPUT,
		UNISON_SPREAD_INPUT,
		UNISON_MIX_INPUT,
		GLIDE_INPUT,
		ATTACK_INPUT,
		DECAY_INPUT,
		NUM_INPUTS
	};

	enum OutputIds 
	{
		OUT_OUTPUT,
		NUM_OUTPUTS
	};

    enum LightIds
    {
        NUM_LIGHTS
    };

    /**
     * Main processing entry point. Called every sample
     */
    void step() override;

    typedef float T;        // use floats for all signals

	constexpr static int maxChannels = 16;
	constexpr static float maxCutoff = 20000.0f;
	constexpr static float dcInFilterCutoff = 5.5f;
	constexpr static float dcOutFilterCutoff = 10.0f;
	constexpr static int maxOscCount = 7;
	constexpr static float fadeDecay = 0.9f; //changed to try to remove pop
	constexpr static float fadeMin = 0.05;

	//Oscillator detunings for unisson from "How to emulate the super saw, Adam Szabo"
	//
	std::vector<float> unisonTunings;  
	std::vector<float> unisonLevels;
	std::vector<CircularBuffer<float> > buffers;
	std::vector<rack::dsp::BiquadFilter>  lowpassFilters;
	std::vector<rack::dsp::BiquadFilter> dcInFilters;
	std::vector<rack::dsp::BiquadFilter> dcOutFilters;
	std::vector<sspo::Limiter> limiters;
	std::vector<float> lastWets;
	std::vector<float> delayTimes; 
	std::vector<std::vector<float> >  oscphases;
	std::vector<dsp::SchmittTrigger> triggers;
	std::vector<bool> resetPending;
	std::vector<float> lastDelayTimes;
	std::vector<float> fadeLevels;
	std::vector<dsp::SlewLimiter> glide;
	std::vector<int> framesToSample;
	std::vector<sspo::AttackDecay<float> > ads;
	std::vector<float> lastOut;
	std::vector<float> pitches;

private:


    float reciprocalSampleRate = 1.0f;
    float sampleRate = 1.0f;
};

template <class TBase>
inline void KSDelayComp<TBase>::step()
{

    auto channels = std::max(TBase::inputs[IN_INPUT].getChannels(), TBase::inputs[TRIGGER_INPUT].getChannels());
		auto octaveParam = TBase::params[OCTAVE_PARAM].getValue();
		auto tuneParam = TBase::params[TUNE_PARAM].getValue();
		auto feedbackParam = TBase::params[FEEDBACK_PARAM].getValue();
		auto filterParam = (TBase::paramQuantities[FILTER_PARAM])->getDisplayValue();
		auto mixParam = TBase::params[MIX_PARAM].getValue();
		auto unisonCount = TBase::params[UNISON_PARAM].getValue();
		auto unisonSpread = TBase::params[UNISON_SPREAD_PARAM].getValue();
		auto unisonMix = TBase::params[UNISON_MIX_PARAM].getValue();
		auto glideParam = TBase::params[GLIDE_PARAM].getValue();
		auto attackParam = TBase::params[ATTACK_PARAM].getValue();
		auto decayParam = TBase::params[DECAY_PARAM].getValue();
		auto pitchLockParam = TBase::params[PITCH_LOCK_PARAM].getValue();

		channels = std::max(channels, 1);



		for (auto i = 0; i < channels; ++i)
		{	
			auto unisonSpreadCoefficient = unisonSpreadScalar (unisonSpread + std::abs(TBase::inputs[UNISON_SPREAD_INPUT].getPolyVoltage (i) / 10.f));
			auto unisonSideLevelCoefficient = unisonSideLevel (unisonMix + std::abs(TBase::inputs[UNISON_MIX_INPUT].getPolyVoltage (i) / 10.f));
			auto unisonCentreLevelCoefficient = unisonCentreLevel (unisonMix + std::abs(TBase::inputs[UNISON_MIX_INPUT].getPolyVoltage (i) / 10.f));
			auto triggerSignal = TBase::inputs[TRIGGER_INPUT].isConnected() 
				? TBase::inputs[TRIGGER_INPUT].getPolyVoltage (i)
				: 0;

			if (triggers[i].process (triggerSignal))
			{
				resetPending[i] = true;
				pitches[i] = dsp::FREQ_C4 * std::pow(2.0f, TBase::inputs[VOCT].getPolyVoltage (i) + octaveParam + tuneParam / 12.0f);
			}

			//set trigger attack delay
			auto aTime = attackParam + TBase::inputs[ATTACK_INPUT].getPolyVoltage (i) * 0.1f;
			auto dTime = decayParam + TBase::inputs[DECAY_INPUT].getPolyVoltage (i) * 0.1f;

			aTime = clamp (aTime, 0.001f, 0.5f);
			dTime = clamp (dTime, 0.01f, 0.5f);

			ads[i].setParameters(aTime, dTime);

			float in = 0.0f;

			// Get input to delay block

			auto l = ads[i].tick();
			if (l > 0.0) 
			{
				if (TBase::inputs[IN_INPUT].isConnected())
				{
					in += TBase::inputs[IN_INPUT].getVoltage (i);
				}
				else
				{
					in += static_cast<float>(drand48()) * 10.0f - 5.0f;
				}
				in *= l;
			}

			
			if ((! TBase::inputs[TRIGGER_INPUT].isConnected()) && TBase::inputs[IN_INPUT].isConnected())
			{
				in += TBase::inputs[IN_INPUT].getVoltage (i);
			}

			
			in = dcInFilters[i].process (in);
			auto feedback = feedbackParam + TBase::inputs[FEEDBACK_INPUT].getPolyVoltage (i) / 10.0f;
			feedback = clamp (feedback, 0.0f, 0.5f);
			
			auto glideTime = glideParam + TBase::inputs[GLIDE_INPUT].getPolyVoltage (i) * 0.005f;
			glideTime = clamp(glideTime, 0.000001, 0.1f);
			glide[i].setRiseFall (glideTime, glideTime);
			auto glideFreq = (pitchLockParam > 0.1f) && TBase::inputs[TRIGGER_INPUT].isConnected() 
				 ? glide[i].process (10.0f, pitches[i])
				 : glide[i].process (10.0f, dsp::FREQ_C4 * std::pow(2.0f, TBase::inputs[VOCT].getPolyVoltage (i) + octaveParam + tuneParam / 12.0f));
			glideFreq = clamp (glideFreq, 20.0f, 20000.0f);
			delayTimes[i] =  1.0f / glideFreq;

			auto color = filterParam;
			if (TBase::inputs[FILTER_INPUT].isConnected())
				color += std::pow (2, TBase::inputs[FILTER_INPUT].getPolyVoltage (i)) * dsp::FREQ_C4;	
			color = clamp (color, 40.0f, maxCutoff);

			lowpassFilters[i].setParameters (rack::dsp::BiquadFilter::LOWPASS, color / sampleRate, 0.707f, 1.0f);
			in = lowpassFilters[i].process (in);


			// trigger reset execution
			auto index = delayTimes[i] * sampleRate - 1.5f;
			if (resetPending[i])
			{
				in = 0.0f; 
				if  (fadeLevels[i] < fadeMin) 
				{
					buffers[i].clear();
					resetPending[i] = false;
					lastDelayTimes[i] = delayTimes[i];
					fadeLevels[i] = 1.0f;
					ads[i].noteOn();
				}
				else
				{
					index = lastDelayTimes[i] * sampleRate - 1.5f; 
					fadeLevels[i] *= fadeDecay;
					ads[i].reset(); 
					lastWets[i]=buffers[i].readBuffer (index) * fadeLevels[i]; 
				}
			}
			else
			{
				lastDelayTimes[i] = delayTimes[i];
			}
			


			// update buffer
			auto wet = buffers[i].readBuffer (index);
			// Add -noise
			in += 1e-3f * (2.0f * drand48() - 1.0f);
			auto dry = 0.5f * in + lastWets[i] * feedback + 0.5f * wet;
			dry =  5.0f * limiters[i].process (dry / 5.0f); 
			buffers[i].writeBuffer (dry);
			//wet =  5.0f * limiters[i].process (wet / 5.0f); 
			lastWets[i] = wet;



			// calc phases 
			auto mixedOsc = 0.0f;
			auto unison = std::abs( std::min (static_cast<int>(unisonCount + TBase::inputs[UNISON_INPUT].getPolyVoltage(i)),7));
			if (unisonCount == 1)
				unisonCentreLevelCoefficient = 1.0f;
			for (int osc = 0; osc < unison; ++osc)
			{	
				oscphases[i][osc] += (unisonTunings[osc] * unisonSpreadCoefficient) / index; 
				if (oscphases[i][osc] >= 1.0f)
					oscphases[i][osc] -= 1.0f;
				if (oscphases[i][osc] < 0.0f)
					oscphases[i][osc] += 1.0f;

				auto phaseoffset = index - oscphases[i][osc] * index;
				if (osc == 0)
					mixedOsc += buffers[i].readBuffer (phaseoffset) * unisonCentreLevelCoefficient;			
				else
					mixedOsc += buffers[i].readBuffer (phaseoffset) * unisonSideLevelCoefficient;	
				
			}
			wet = mixedOsc;


			auto mix = mixParam + TBase::inputs[MIX_INPUT].getPolyVoltage (i) / 10.0f;
			mix = clamp (mix, 0.0f, 1.0f);
			if (resetPending[i]) in = 0.0f; 
			float out = crossfade (in, wet, mix);

			out = dcOutFilters[i].process (out);
			out = out * fadeLevels[i];
			lastOut[i] = out;
			
		  TBase::outputs[OUT_OUTPUT].setVoltage (out, i);
	  	}
	TBase::outputs[OUT_OUTPUT].setChannels (channels);
}

template <class TBase>
int KSDelayDescription<TBase>::getNumParams()
{
    return KSDelayComp<TBase>::NUM_PARAMS;
}


template <class TBase>
IComposite::Config KSDelayDescription<TBase>::getParam(int i)
{
   
    IComposite::Config ret = {0.0f, 1.0f, 0.0f, "Code type", "unit", 0.0f, 1.0f, 0.0f};
    switch (i) {
        case KSDelayComp<TBase>::OCTAVE_PARAM:      
            ret = {-4.0f, 0.0f, 0.0f, "Octave", " octave", 0.0f, 1.0f, 0.0f};
            break;
        case KSDelayComp<TBase>::TUNE_PARAM:      
            ret = {-7.0f, 7.0f, 0.0f, "Tune", " semitones", 0.0f, 1.0f, 0.0f};
            break;
        case KSDelayComp<TBase>::FEEDBACK_PARAM:      
            ret = {0.0f, 0.5f, 0.5f, "Feedback", "%", 0, 100, 0.0f};
            break;
        case KSDelayComp<TBase>::FILTER_PARAM:      
            ret = {0.0f, 1.125f, 1.125f, "Frequency", " Hz", std::pow (2, 10.f), dsp::FREQ_C4 / std::pow (2, 5.f), 0.0f};
            break;
        case KSDelayComp<TBase>::MIX_PARAM:      
            ret = {0.0f, 1.0f, 1.0f, "Mix", "%", 0, 100, 0.0f};
            break;
		case KSDelayComp<TBase>::UNISON_PARAM:
			ret = {1.0f, 7.0f, 1.0f, "Unison count" ," ", 0, 1 , 0.0f};
			break;
		case KSDelayComp<TBase>::UNISON_SPREAD_PARAM:
			ret ={0.0f, 1.0f, 0.5f, "Unison Spread", " ", 0, 1, 0.0f};
			break;
		case KSDelayComp<TBase>::UNISON_MIX_PARAM:
			ret = {0.0f, 1.0f, 1.0f, "Unison Mix", "  ", 0, 1, 0.0f};
			break;
		case KSDelayComp<TBase>::GLIDE_PARAM:
			ret = {0.0001f, 0.05f, 0.05f, "Glide", " ", 0, 1, 0.0f};
			break;
		case KSDelayComp<TBase>::ATTACK_PARAM:
			ret = {0.0f, 0.5f, 0.01f, "Attack", " ", 0, 1, 0.0f};
			break;
		case KSDelayComp<TBase>::DECAY_PARAM:
			ret = {0.0f, 2.0f, 0.5f, "Decay", " ", 0, 1, 0.0f};
			break;
		case KSDelayComp<TBase>::PITCH_LOCK_PARAM:
			ret = {0.0f, 1.0f, 0.0f, "Pitch Lock", " ", 0, 1, 0.0f};
			break;
        default:
            assert(false);
    }
    return ret;

}

