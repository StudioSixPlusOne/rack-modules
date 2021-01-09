/*
 * Copyright (c) 2019, 2021 Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>
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

#include <cmath>
#include <memory>
#include <cassert>
#include <sstream>

#include "AudioMath.h"

template <typename T>
class CircularBuffer
{
public:
    CircularBuffer()
    {
        reset (4096);
    }
    CircularBuffer (const unsigned int minBufferSize)
    {
        reset (minBufferSize);
    }

    std::string toString() const
    {
        std::stringstream ss;
        ss << "length : " << bufferLength << "\n";
        for (auto i = 0; i < bufferLength; ++i)
            ss << buffer[i] << "   ";
        ss << "\n";
        return ss.str();
    }

    void reset (const unsigned int minBufferSize)
    {
        writeIndex = 0;
        bufferLength = static_cast<unsigned int> (std::pow (2, std::ceil (std::log (minBufferSize) / std::log (2))));
        wrapBits = bufferLength - 1;
        buffer.reset (new T[bufferLength]);
        clear();
    }

    void clear()
    {
        for (auto i = 0; i < static_cast<int> (bufferLength); ++i)
        {
            buffer[i] = 0;
        }
    }

    /**
     * Places the provided value at the write index, overriteing any existing data
     * @param newValue
     */
    inline void writeBuffer (const T newValue) noexcept
    {
        writeIndex++;
        writeIndex &= wrapBits;
        buffer[writeIndex] = std::isnan (newValue) || std::isinf (newValue) ? 0 : newValue;
    }

    inline void writeBuffer (int index, const T newValue) noexcept
    {
        index = writeIndex - index;
        index &= wrapBits;
        buffer[index] = std::isnan (newValue) || std::isinf (newValue) ? 0 : newValue;
    }

    inline void addBuffer (int index, T newValue) noexcept
    {
        index = writeIndex - index;
        index &= wrapBits;
        newValue += buffer[index];
        buffer[(index)] = std::isnan (newValue) || std::isinf (newValue) ? 0 : newValue;
    }

    inline T readBuffer (const int delaySamples) const noexcept
    {
        return buffer[(writeIndex - delaySamples) & wrapBits];
    }

    inline T readBuffer (const float delaySamples) const noexcept
    {
        auto y1 = readBuffer (static_cast<int> (delaySamples));
        auto y2 = readBuffer (static_cast<int> (delaySamples) + 1);
        auto fract = delaySamples - static_cast<int> (delaySamples);
        return sspo::AudioMath::linearInterpolate (y1, y2, fract);
    }

    int size() const
    {
        return static_cast<int> (bufferLength);
    }

private:
    std::unique_ptr<T[]> buffer{ nullptr };
    unsigned int writeIndex{ 0 };
    unsigned int bufferLength{ 0 };
    unsigned int wrapBits{ 0 };
};

/**
 *  Two circular buffers, connected in a circular fashion
 *  between the connections are user definable nodes that can
 *  be used to simulate damping effects where the delay lines are
 *  terminated
 *
 * @tparam T
 */
template <typename T>
class WaveGuide
{
public:
    /**
     * Nodes places at each end of delay line, to simulate damping
     *  intended to be base class
     *  example use, filter or lpg
     */
    class Node
    {
    public:
        virtual T step (T in)
        {
            return in;
        }
    };

    /**
     * Default construction
     * fist and second nodes pass signal unaltered
     *
     */
    WaveGuide()
    {
        setNodeFirst (std::make_shared<Node>());
        setNodeSecond (std::make_shared<Node>());
        setBufferSize (4096);
    }

    WaveGuide (std::shared_ptr<Node> firstNode, std::shared_ptr<Node> secondNode, int size = 4096)
    {
        setNodeFirst (firstNode);
        setNodeSecond (secondNode);
        setBufferSize (size);
    }
    std::string toString() const
    {
        std::stringstream ss;
        ss << "length : " << length << "\n";
        ss << "First  : \n"
           << buffers.first.toString().c_str();
        ss << "Second : \n"
           << buffers.second.toString().c_str();
        return ss.str();
    }

    /**
     * size is rounder up to nearest n^2
     * must be greater than delay length
     * resets delay lines.
     * Not RT safe and should NOT be used during
     * audio callback loop.
     * Should be used for initilization only
     *
     * use setLength() for variable length delay lines
     * @param size
     */
    void setBufferSize (int size)
    {
        buffers.first.reset (size);
        buffers.second.reset (size);
    }

    int getBufferSize() const
    {
        return buffers.first.size();
    }

    /**
     * instances of classes derived fron Node to emulate signal erosion
     * at ends of the delay linee
     * @param node
     */
    void setNodeFirst (std::shared_ptr<Node> node)
    {
        if (node != nullptr)
            nodes.first = node;
    }

    /**
    * instances of classes derived fron Node to emulate signal erosion
    * at ends of the delay linee
    * @param node
    */
    void setNodeSecond (std::shared_ptr<Node> node)
    {
        if (node != nullptr)
            nodes.second = node;
    }

    /***
     * delay line length in samples
     * can be used during the audio callback
     * for variable length delay lines
     * @return
     */
    int getLength() const
    {
        return length;
    }

    /**
     * delay line length in samples
     * can be used during the audio callback
     * for variable length delay lines
     * @param length
     */
    void setLength (int length)
    {
        if (length < getBufferSize())
        {
            WaveGuide::length = length;
        }
    }

    /**
     * progressese the buffers by 1 sample
     * wrapping the end of buffers through
     * filters.
     *
     * should be called once per frame
     */
    void step()
    {
        T x = buffers.second.readBuffer (length);
        buffers.second.writeBuffer (length, 0); //remove from buffer
        x = nodes.second.get()->step (x);
        buffers.first.writeBuffer (x);
        x = buffers.first.readBuffer (length);
        buffers.first.writeBuffer (length, 0); //remove from buffer
        x = nodes.first.get()->step (x);
        buffers.second.writeBuffer (x);
    }

    /**
     * input
     * @param frac distance from first node 0.0f - 1.0f
     * @param in signal to be added to the delay lines
     */
    void addBuffer (float frac, T in)
    {
        assert (frac >= 0.0f);
        assert (frac < 1.0f);

        auto firstIndex = static_cast<int> (length * frac);
        auto secondIndex = static_cast<int> (length * (1.0f - frac));
        //DEBUG(" add buffer first index : %d  Second index %d  in : %f\n ", firstIndex, secondIndex, in);

        buffers.first.addBuffer (firstIndex, in * 0.5f);
        buffers.second.addBuffer (secondIndex, in * 0.5f);
    }

    /**
     * output, should be called once per step()
     * can be call multiple times per sample to
     * simulate multiple output points.
     * @param frac distace from first node 0.0f - 1.0f
     * @return output sample
     */
    T readBuffer (float frac)
    {
        assert (frac >= 0.0f);
        assert (frac <= 1.0f);

        auto firstIndex = static_cast<int> (length * frac);
        auto secondIndex = static_cast<int> (length * (1.0f - frac));
        //        printf(" first index : %d  Second index %d \n ", firstIndex, secondIndex);

        return buffers.first.readBuffer (firstIndex)
               + buffers.second.readBuffer (secondIndex);
    }

private:
    std::pair<CircularBuffer<T>, CircularBuffer<T>> buffers;
    std::pair<std::shared_ptr<Node>, std::shared_ptr<Node>> nodes{ nullptr, nullptr };
    float length = 500.0f;
};
