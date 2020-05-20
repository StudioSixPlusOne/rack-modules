
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

#include <assert.h>

#include "asserts.h"
#include "Analyzer.h"
#include "AudioMath.h"
#include "FFT.h"
#include "FFTData.h"
// #include "SinOscillator.h"  //disables get frequency response
#include "AudioMath.h"


int Analyzer::getMax(const FFTDataCpx& data)
{
    return getMaxExcluding(data, std::set<int>());
}

int Analyzer::getMaxExcluding(const FFTDataCpx& data, int exclusion)
{
    std::set<int> exclusions;
    exclusions.insert(exclusion);
    return getMaxExcluding(data, exclusions);
}

int Analyzer::getMaxExcluding(const FFTDataCpx& data, std::set<int> exclusions)
{
    int maxBin = -1;
    float maxMag = 0;
    for (int i = 0; i < data.size(); ++i) {
        if (exclusions.find(i) == exclusions.end()) {
            const float mag = std::abs(data.get(i));
            if (mag > maxMag) {
                maxMag = mag;
                maxBin = i;
            }
        }
    }
    return maxBin;
}

float Analyzer::getSlopeLowpass(const FFTDataCpx& response, float fTest, float sampleRate)
{
    const int bin1 = FFT::freqToBin(fTest, sampleRate, response.size());
    const int bin2 = bin1 * 4;                // two octaves
    assert(bin2 < response.size());
    const float mag1 = response.getAbs(bin1);
    const float mag2 = response.getAbs(bin2);
    return float(sspo::AudioMath::db(mag2) - sspo::AudioMath::db(mag1)) / 2;
}

float Analyzer::getSlopeHighpass(const FFTDataCpx& response, float fTest, float sampleRate)
{
    const int bin1 = FFT::freqToBin(fTest, sampleRate, response.size());
    const int bin2 = bin1 / 4;                // two octaves
    assert(bin2  > 2);
    const float mag1 = response.getAbs(bin1);
    const float mag2 = response.getAbs(bin2);
    return float(sspo::AudioMath::db(mag2) - sspo::AudioMath::db(mag1)) / 2;
}

std::tuple<int, int, int> Analyzer::getMaxAndShoulders(const FFTDataCpx& data, float atten)
{
    assert(atten < 0);
    int maxBin = getMax(data);
    int iMax = data.size() / 2;

    assert(maxBin >= 0);
    const double dbShoulder = atten + sspo::AudioMath::db(std::abs(data.get(maxBin)));

    int i;
    int iShoulderLow = -1;
    int iShoulderHigh = -1;
    bool done;
    for (done = false, i = maxBin; !done; ) {
        const double db = sspo::AudioMath::db(std::abs(data.get(i)));
        if (i >= iMax) {
            done = true;
        } else if (db <= dbShoulder) {
            iShoulderHigh = i;
            done = true;
        } else {
            i++;
        }
    }
    for (done = false, i = maxBin; !done; ) {
        const double db = sspo::AudioMath::db(std::abs(data.get(i)));
        if (db <= dbShoulder) {
            iShoulderLow = i;
            done = true;
        } else if (i <= 0) {
            done = true;
        } else {
            i--;
        }
    }
   // printf("out of loop, imax=%d, shoulders=%d,%d\n", maxBin, iShoulderLow, iShoulderHigh);

    return std::make_tuple(iShoulderLow, maxBin, iShoulderHigh);
}

std::tuple<double, double, double> Analyzer::getMaxAndShouldersFreq(const FFTDataCpx& data, float atten, float sampleRate)
{
    auto stats = getMaxAndShoulders(data, atten);
    return  std::make_tuple(FFT::bin2Freq(std::get<0>(stats), sampleRate, data.size()),
        FFT::bin2Freq(std::get<1>(stats), sampleRate, data.size()),
        FFT::bin2Freq(std::get<2>(stats), sampleRate, data.size())
    );
}


// TODO: pass in cutoff
std::vector<Analyzer::FPoint> Analyzer::getFeatures(const FFTDataCpx& data, float sensitivityDb, float sampleRate, float dbMinCutoff)
{
   // TODO: pass this in
  //  const float dbMinCutoff = -100;
    assert(sensitivityDb > 0);
    std::vector<FPoint> ret;
    float lastDb = 10000000000;
    // only look at the below nyquist stuff
    for (int i = 0; i < data.size() / 2; ++i) {
        const float db = (float) sspo::AudioMath::db(std::abs(data.get(i)));
        if ((std::abs(db - lastDb) >= sensitivityDb) && (db > dbMinCutoff)) {
            double freq = FFT::bin2Freq(i, sampleRate, data.size());
            FPoint p(float(freq), db);
           // printf("feature at bin %d, db=%f raw val=%f\n", i, db, std::abs(data.get(i)));
            ret.push_back(p);
            lastDb = db;
        }
    }
    return ret;
}


std::vector<Analyzer::FPoint> Analyzer::getPeaks(const FFTDataCpx& data, float sampleRate, float minDb)
{
    std::vector<FPoint> ret;

    // only look at the below nyquist stuff
    for (int i = 0; i < data.size() / 2; ++i) {
        const double mag = std::abs(data.get(i));
        const double db = sspo::AudioMath::db(mag);
        bool isPeak = false;
        if (i < 2 || i >(data.size() / 2) - 2) {
            isPeak = true;
        } else {

            const double magBelow = std::abs(data.get(i - 1));
            const double magAbove = std::abs(data.get(i + 1));
            if (mag <= magBelow || mag <= magAbove) {
                isPeak = false;
            } else {

#if 1
                double average = 0;
                for (int j = 0; j < 5; ++j) {
                    average += std::abs(data.get(i + j - 2));
                }
                average -= mag;             // subtract out our contribution
                average /= 4.0;
#if 0
                double a = std::abs(data.get(i - 2));
                double b = std::abs(data.get(i - 1));
                double c = std::abs(data.get(i - 0));
                double d = std::abs(data.get(i + 1));
                double e = std::abs(data.get(i + 2));
#endif
                isPeak = (mag > (average * 2));
#else
                //this way average db
                double average = 0;
                for (int j = 0; j < 5; ++j) {
                    average += AudioMath::db(std::abs(data.get(i + j - 2)));
                }
                isPeak = (db > (average + 3));
#endif
                //if (isPeak) printf("accepted peak at %f db, average was %f\n", db, average);
            }
        }
        if (db < minDb) {
            isPeak = false;
        }

        if (isPeak) {
        //if ((std::abs(db - lastDb) >= sensitivityDb) && (db > dbMinCutoff)) {
            double freq = FFT::bin2Freq(i, sampleRate, data.size());
            FPoint p(float(freq), (float) db);
            // printf("feature at bin %d, db=%f raw val=%f\n", i, db, std::abs(data.get(i)));
            ret.push_back(p);
        }
    }
    return ret;
}

void Analyzer::getAndPrintFeatures(const FFTDataCpx& data, float sensitivityDb, float sampleRate, float dbMinCutoff)
{
    auto features = getFeatures(data, sensitivityDb, sampleRate, dbMinCutoff);
    printf("there are %d features\n", (int) features.size());
    for (int i = 0; i < (int) features.size(); ++i) {
        printf("feature: freq=%.2f, db=%.2f\n", features[i].freq, features[i].gainDb);
    }
}

void Analyzer::getAndPrintPeaks(const FFTDataCpx& data, float sampleRate, float minDb)
{
    auto peaks = getPeaks(data, sampleRate, minDb);
    printf("there are %d peaks\n", (int) peaks.size());
    for (int i = 0; i < (int) peaks.size(); ++i) {
        printf("peak: freq=%f, db=%f\n", peaks[i].freq, peaks[i].gainDb);
    }
}


void Analyzer::getAndPrintFreqOfInterest(const FFTDataCpx& data, float sampleRate, const std::vector<double>& freqOfInterest)
{
    for (double freq : freqOfInterest) {
        int bin = FFT::freqToBin((float) freq, sampleRate, data.size());
        if (bin > 2 && bin < data.size() - 2) {

            ;
            double a = sspo::AudioMath::db(std::abs(data.get(bin - 2)));
            double b = sspo::AudioMath::db(std::abs(data.get(bin - 1)));
            double c = sspo::AudioMath::db(std::abs(data.get(bin)));
            double d = sspo::AudioMath::db(std::abs(data.get(bin + 1)));
            double e = sspo::AudioMath::db(std::abs(data.get(bin + 2)));

            double db = std::max(e, std::max(
                std::max(a, b),
                std::max(c, d)));

            printf("freq=%.2f db=%f  range:%.2f,%.2f,%.2f,%.2f,%.2f\n", freq, db,
                a, b, c, d, e
            );
        }
    }

#if 0
    for (int i = 0; i < data.size() / 2; ++i) {
        double freq = FFT::bin2Freq(i, sampleRate, data.size());
    }
#endif

}

#if 0
void Analyzer::getFreqResponse(FFTDataCpx& out, std::function<float(float)> func)
{
    /**
    * testSignal is the time domain sweep
    * testOutput if the time domain output of "func"
    * testSpecrum is the FFT of testSignal
    * spectrum is the FFT of testOutput

    */
    // First set up a test signal 
    const int numSamples = out.size();
    //  std::vector<float> testSignal(numSamples);
    FFTDataReal testSignal(numSamples);
    generateSweep(44100, testSignal.data(), numSamples, 20, 20000);

    // Run the test signal though func, capture output in fft real
    FFTDataReal testOutput(numSamples);
    for (int i = 0; i < out.size(); ++i) {
        const float y = func(testSignal.get(i));
        testOutput.set(i, y);
    }

    // then take the inverse fft
    FFTDataCpx spectrum(numSamples);
    FFT::forward(&spectrum, testOutput);


    // take the forward FFT of the test signal
    FFTDataCpx testSpectrum(numSamples);
    FFT::forward(&testSpectrum, testSignal);

    for (int i = 0; i < numSamples; ++i) {
        const cpx x = (std::abs(testSpectrum.get(i)) == 0) ? 0 :
            spectrum.get(i) / testSpectrum.get(i);
        out.set(i, x);
    }

#if 0
    for (int i = 0; i < numSamples; ++i) {
        printf("%d, sig=%f out=%f mag(sig)=%f mag(out)=%f rsp=%f\n",
            i, testSignal.get(i), testOutput.get(i),
            std::abs(testSpectrum.get(i)),
            std::abs(spectrum.get(i)),
            std::abs(out.get(i))
        );
    }
#endif
}

#endif


double Analyzer::hamming(int iSample, int totalSamples)
{
    const double a0 = .53836;
    double theta = sspo::AudioMath::k_pi * 2.0 * double(iSample) / double(totalSamples - 1);
    return a0 - (1.0 - a0) * std::cos(theta);
}

void Analyzer::getSpectrum(FFTDataCpx& out, bool useWindow, std::function<float()> func)
{

    const int numSamples = out.size();

    // Run the test signal though func, capture output in fft real
    FFTDataReal testOutput(numSamples);
    for (int i = 0; i < out.size(); ++i) {
        const float w = useWindow ? (float) hamming(i, out.size()) : 1;
        const float y = float(func() * w);
        testOutput.set(i, y);
    }

    FFT::forward(&out, testOutput);

#if 0
    for (int i = 0; i < numSamples; ++i) {
        printf("%d, sig=%f out=%f mag(sig)=%f mag(out)=%f rsp=%f\n",
            i, testSignal.get(i), testOutput.get(i),
            std::abs(testSpectrum.get(i)),
            std::abs(spectrum.get(i)),
            std::abs(out.get(i))
        );
    }
#endif
}

#if 0
void Analyzer::generateSweep(float sampleRate, float* out, int numSamples, float minFreq, float maxFreq)
{
    assert(maxFreq > minFreq);
    const double minLog = std::log2(minFreq);
    const double maxLog = std::log2(maxFreq);
    const double delta = (maxLog - minLog) / numSamples;

    SinOscillatorParams<double> params;
    SinOscillatorState<double> state;

    double fLog = minLog;
    for (int i = 0; i < numSamples; ++i, fLog += delta) {
        const double freq = std::pow(2, fLog);
        assert(freq < (sampleRate / 2));

        SinOscillator<double, false>::setFrequency(params, freq / sampleRate);
        double val = SinOscillator<double, false>::run(state, params);

       // ::printf("out[%d] = %f f=%f\n", i, val, freq);
        out[i] = (float) val;
    }
}
#endif

double Analyzer::makeEvenPeriod(double desiredFreq, double sampleRate, int numSamples)
{
    assert(desiredFreq <= (sampleRate / 2.0));
    assert(numSamples > 2);
    double desiredPeriodSamples = sampleRate / desiredFreq;
    double periodsPerFrame = numSamples / desiredPeriodSamples;


     //printf("desiredFreq = %f, desired period/ samp = %f, periods per frame = %f\n", desiredFreq, desiredPeriodSamples, periodsPerFrame);


    double evenPeriodsPerFrame = std::floor(periodsPerFrame);

    double period = (double) numSamples / evenPeriodsPerFrame;
    //printf("period = %f\n", period);
    double freq = sampleRate / period;
    //printf("freq = %f\n", freq);
    assert(freq > .0001);       // don't want zero
    return (freq);
}

void Analyzer::assertSingleFreq(const FFTDataCpx& spectrum, float expectedFreq, float sampleRate)
{
    assert(expectedFreq < (sampleRate / 2));
    int maxBin = Analyzer::getMax(spectrum);
    double maxFreq = FFT::bin2Freq(maxBin, sampleRate, spectrum.size());

    int nextMaxBin = Analyzer::getMaxExcluding(spectrum, maxBin);
    float maxPower = std::abs(spectrum.get(maxBin));
    float nextMaxPower = std::abs(spectrum.get(nextMaxBin));

    double spuriousDb = sspo::AudioMath::db(nextMaxPower / maxPower);

    assertClose(maxFreq, static_cast<double>(expectedFreq), 1.0);
    assertLE(spuriousDb, 70);
}