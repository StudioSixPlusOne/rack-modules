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

//class FFTDataCpx;
//class FFTDataReal;

#include "FFTData.h"

class ColoredNoiseSpec
{
public:
    float slope = 0;
    float highFreqCorner = 4000;
    float sampleRate = 44100;
    bool operator != (const ColoredNoiseSpec& other) const
    {
        return (slope != other.slope) ||
            (highFreqCorner != other.highFreqCorner) ||
            (sampleRate != other.sampleRate);
    }
};

class FFT
{
public:
    /** Forward FFT will do the 1/N scaling
     */
    static bool forward(FFTDataCpx* out, const FFTDataReal& in);
    static bool inverse(FFTDataReal* out, const FFTDataCpx& in);

  //  static FFTDataCpx* makeNoiseFormula(float slope, float highFreqCorner, int frameSize);

    /**
     * Fills a complex FFT frame with frequency domain data describing noise
     */
    static void makeNoiseSpectrum(FFTDataCpx* output, const ColoredNoiseSpec&);

    static void normalize(FFTDataReal*, float maxValue);
    static double bin2Freq(int bin, double sampleRate, int numBins);
    static int freqToBin(double freq, double sampleRate, int numBins);
};