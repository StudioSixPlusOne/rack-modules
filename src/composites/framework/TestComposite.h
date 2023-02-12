/*
 MIT License

Copyright (c) 2018 squinkylabs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#pragma once

#include <vector>
#include <cstdint>

#include "common.hpp"
#include "random.hpp"
#include "filter.hpp"
#include "digital.hpp"
#include "math.hpp"

struct Light
{
    /** The square of the brightness value */
    float value = 0.0;
    float getBrightness();
    void setBrightness (float brightness)
    {
        value = (brightness > 0.f) ? brightness * brightness : 0.f;
    }
    void setBrightnessSmooth (float brightness)
    {
    }

    void setSmoothBrightness (float brightness, float time)
    {
    }
};

static const int PORT_MAX_CHANNELS = 16;

struct alignas (32) Port
{
    /** Voltage of the port. */
    union
    {
        /** Unstable API. Use getVoltage() and setVoltage() instead. */
        float voltages[PORT_MAX_CHANNELS] = {};
        /** DEPRECATED. Unstable API. Use getVoltage() and setVoltage() instead. */

        // TODO: get rid of this
        float value;
    };
    union
    {
        /** Number of polyphonic channels
		Unstable API. Use set/getChannels() instead.
		May be 0 to PORT_MAX_CHANNELS.
		*/
        uint8_t channels = 0;
        /** DEPRECATED. Unstable API. Use isConnected() instead. */
        uint8_t _active;
    };
    /** For rendering plug lights on cables.
	Green for positive, red for negative, and blue for polyphonic.
	*/
    Light plugLights[3];

    /** Sets the voltage of the given channel. */
    void setVoltage (float voltage, int channel = 0)
    {
        voltages[channel] = voltage;
    }

    /** Returns the voltage of the given channel.
	Because of proper bookkeeping, all channels higher than the input port's number of channels should be 0V.
	*/
    float getVoltage (int channel = 0)
    {
        return voltages[channel];
    }

    /** Returns the given channel's voltage if the port is polyphonic, otherwise returns the first voltage (channel 0). */
    float getPolyVoltage (int channel)
    {
        return isMonophonic() ? getVoltage (0) : getVoltage (channel);
    }

    /** Returns the voltage if a cable is connected, otherwise returns the given normal voltage. */
    float getNormalVoltage (float normalVoltage, int channel = 0)
    {
        return isConnected() ? getVoltage (channel) : normalVoltage;
    }

    float getNormalPolyVoltage (float normalVoltage, int channel)
    {
        return isConnected() ? getPolyVoltage (channel) : normalVoltage;
    }

    /** Returns a pointer to the array of voltages beginning with firstChannel.
	The pointer can be used for reading and writing.
	*/
    float* getVoltages (int firstChannel = 0)
    {
        return &voltages[firstChannel];
    }

    /** Copies the port's voltages to an array of size at least `channels`. */
    void readVoltages (float* v)
    {
        for (int c = 0; c < channels; c++)
        {
            v[c] = voltages[c];
        }
    }

    /** Copies an array of size at least `channels` to the port's voltages.
	Remember to set the number of channels *before* calling this method.
	*/
    void writeVoltages (const float* v)
    {
        for (int c = 0; c < channels; c++)
        {
            voltages[c] = v[c];
        }
    }

    /** Sets all voltages to 0. */
    void clearVoltages()
    {
        for (int c = 0; c < channels; c++)
        {
            voltages[c] = 0.f;
        }
    }

    /** Returns the sum of all voltages. */
    float getVoltageSum()
    {
        float sum = 0.f;
        for (int c = 0; c < channels; c++)
        {
            sum += voltages[c];
        }
        return sum;
    }

    template <typename T>
    T getVoltageSimd (int firstChannel)
    {
        return T::load (&voltages[firstChannel]);
    }

    template <typename T>
    T getPolyVoltageSimd (int firstChannel)
    {
        return isMonophonic() ? getVoltage (0) : getVoltageSimd<T> (firstChannel);
    }

    template <typename T>
    T getNormalVoltageSimd (T normalVoltage, int firstChannel)
    {
        return isConnected() ? getVoltageSimd<T> (firstChannel) : normalVoltage;
    }

    template <typename T>
    T getNormalPolyVoltageSimd (T normalVoltage, int firstChannel)
    {
        return isConnected() ? getPolyVoltageSimd<T> (firstChannel) : normalVoltage;
    }

    template <typename T>
    void setVoltageSimd (T voltage, int firstChannel)
    {
        voltage.store (&voltages[firstChannel]);
    }

    /** Sets the number of polyphony channels.
	Also clears voltages of higher channels.
	If disconnected, this does nothing (`channels` remains 0).
	If 0 is given, `channels` is set to 1 but all voltages are cleared.
	*/
    void setChannels (int channels)
    {
        // If disconnected, keep the number of channels at 0.
        if (this->channels == 0)
        {
            //return;
        }
        // Set higher channel voltages to 0
        for (int c = channels; c < this->channels; c++)
        {
            voltages[c] = 0.f;
        }
        // Don't allow caller to set port as disconnected
        if (channels == 0)
        {
            channels = 1;
        }
        this->channels = channels;
    }

    /** Returns the number of channels.
	If the port is disconnected, it has 0 channels.
	*/
    int getChannels()
    {
        return channels;
    }

    /** Returns whether a cable is connected to the Port.
	You can use this for skipping code that generates output voltages.
	*/
    bool isConnected()
    {
        return channels > 0;
    }

    /** Returns whether the cable exists and has 1 channel. */
    bool isMonophonic()
    {
        return channels == 1;
    }

    /** Returns whether the cable exists and has more than 1 channel. */
    bool isPolyphonic()
    {
        return channels > 1;
    }

    void process (float deltaTime);

    /** Use getNormalVoltage() instead. */
#if 0
	DEPRECATED float normalize(float normalVoltage) {
		return getNormalVoltage(normalVoltage);
	}
#endif
};

struct Input : Port
{
};
struct Output : Port
{
};

struct Param
{
    float value = 0.0;

    void setValue (const float x)
    {
        value = x;
    }

    float getValue()
    {
        return value;
    }
};

struct ParamQuantities
{
    float getDisplayValue()
    {
        return 50.0f;
    }
};

// Rack functions added, only compiled for tests

template <typename T>
inline T clamp (T x, T a, T b)
{
    return std::min (std::max (x, a), b);
}

inline float crossfade (float a, float b, float p)
{
    return a + (b - a) * p;
}

/**
* Base class for composites embeddable in a unit test
* Isolates test from VCV.
*/

class TestComposite
{
public:
    using Port = ::Port;
    using Param = ::Param;

    TestComposite() : inputs (40),
                      outputs (40),
                      params (60),
                      paramQuantities (60),
                      lights (20)
    {
    }
    virtual ~TestComposite()
    {
    }

    std::vector<Input> inputs;
    std::vector<Output> outputs;
    std::vector<Param> params;
    std::vector<ParamQuantities*> paramQuantities;
    std::vector<Light> lights;

    float engineGetSampleTime()
    {
        return 1.0f / 44100.0f;
    }

    float engineGetSampleRate()
    {
        return 44100.f;
    }

    virtual void step()
    {
    }
};
