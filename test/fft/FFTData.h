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

#include <complex>
#include <vector>
#include <assert.h>


class FFT;


/**
 * Our wrapper api uses std::complex, so we don't need to expose kiss_fft_cpx
 * outside. Our implementation assumes the two are equivalent, and that a
 * reinterpret_cast can bridge them.
 */
using cpx = std::complex<float>;

template <typename T>
class FFTData
{
public:
    friend FFT;
    FFTData(int numBins);
    ~FFTData();
    T get(int bin) const;
    void set(int bin, T value);

    int size() const
    {
        return (int) buffer.size();
    }

    T * data()
    {
        return buffer.data();
    }

    float getAbs(int bin) const
    {
        return std::abs(buffer[bin]);
    }

    static int _count;
private:
    std::vector<T> buffer;

    /**
    * we store this without type so that clients don't need
    * to pull in the kiss_fft headers. It's mutable so it can
    * be lazy created by FFT functions.
    * Note that the cfg has a "direction" baked into it. For
    * now we assume that all FFT with complex input will be inverse FFTs.
    */
    mutable void * kiss_cfg = 0;
};

using FFTDataReal = FFTData<float>;
using FFTDataCpx = FFTData<cpx>;

template<typename T> int FFTData<T>::_count = 0;

template <typename T>
inline FFTData<T>::FFTData(int numBins) :
    buffer(numBins)
{
    ++_count;
}

template <typename T>
inline FFTData<T>::~FFTData()
{
    // We need to manually delete the cfg, since only "we" know
    // what type it is.
    if (kiss_cfg) {
        free(kiss_cfg);
    }
    --_count;
}

template <typename T>
inline T FFTData<T>::get(int index) const
{
    assert(index < (int) buffer.size() && index >= 0);
    return buffer[index];
}

template <typename T>
inline void FFTData<T>::set(int index, T value)
{
    assert(index < (int) buffer.size() && index >= 0);
    buffer[index] = value;
}
