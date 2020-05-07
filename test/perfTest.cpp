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

#include <functional>
#include <time.h>
#include <cmath>
#include <limits>
#include <random>

#include "filter.hpp"


extern double overheadInOut;
extern double overheadOutOnly;


#include "MeasureTime.h"
#include "TestComposite.h"

#include "AudioMath.h"
#include "CircularBuffer.h"
#include "HardLimiter.h"
#include "LookupTable.h"

#include "KSDelay.h"

#ifdef _USE_WINDOWS_PERFTIME
double SqTime::frequency = 0;
#endif

// There are many tests that are disabled with #if 0.
// In most cases they still work, but don't need to be run regularly


static void test1()
{
    double d = .1;
    srand(57);
    const double scale = 1.0 / RAND_MAX;



    MeasureTime<float>::run(overheadInOut, "test1 sin", []() {
        float x = std::sin(TestBuffers<float>::get());
        return x;
        }, 1);

    MeasureTime<double>::run(overheadInOut, "test1 sin double", []() {
        float x = std::sin(TestBuffers<float>::get());
        return x;
        }, 1);

    MeasureTime<float>::run(overheadInOut, "test1 sinx2 float", []() {
        float x = std::sin(TestBuffers<float>::get());
        x = std::sin(x);
        return x;
        }, 1);

    MeasureTime<float>::run(overheadInOut, "mult float-10", []() {
        float x = TestBuffers<float>::get();
        float y = TestBuffers<float>::get();
        return x * y;
        }, 10);

    MeasureTime<double>::run(overheadInOut, "mult dbl", []() {
        double x = TestBuffers<double>::get();
        double y = TestBuffers<double>::get();
        return x * y;
        }, 1);

    MeasureTime<float>::run(overheadInOut, "div float", []() {
        float x = TestBuffers<float>::get();
        float y = TestBuffers<float>::get();
        return x / y;
        }, 1);

    MeasureTime<double>::run(overheadInOut, "div dbl", []() {
        double x = TestBuffers<double>::get();
        double y = TestBuffers<double>::get();
        return x / y;
        }, 1);

    MeasureTime<float>::run(overheadInOut, "test1 (do nothing)", [&d, scale]() {
        return TestBuffers<float>::get();
        }, 1);

    MeasureTime<float>::run(overheadInOut, "test1 pow2 int float", []() {
        float x = std::pow(2, TestBuffers<float>::get());
        return x;
        }, 1);

        MeasureTime<float>::run(overheadInOut, "test1 pow2 float float", []() {
        float x = std::pow(2.0f, TestBuffers<float>::get());
        return x;
        }, 1);

    MeasureTime<float>::run(overheadInOut, "test1 pow rnd float", []() {
        float x = std::pow(TestBuffers<float>::get(), TestBuffers<float>::get());
        return x;
        }, 1);

    MeasureTime<float>::run(overheadInOut, "test1 exp float", []() {
        float x = std::exp(TestBuffers<float>::get());
        return x;
        }, 1);
}


static void testNoise(bool useDefault)
{

    std::default_random_engine defaultGenerator{99};
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<double> distribution{0, 1.0};

    std::string title = useDefault ? "default random" : "fancy random";

    MeasureTime<float>::run(overheadInOut, title.c_str(), [useDefault, &distribution, &defaultGenerator, &gen]() {
        if (useDefault) return distribution(defaultGenerator);
        else return distribution(gen);
        }, 1);
}


static uint64_t xoroshiro128plus_state[2] = {};

static uint64_t rotl(const uint64_t x, int k)
{
    return (x << k) | (x >> (64 - k));
}

static uint64_t xoroshiro128plus_next(void)
{
    const uint64_t s0 = xoroshiro128plus_state[0];
    uint64_t s1 = xoroshiro128plus_state[1];
    const uint64_t result = s0 + s1;

    s1 ^= s0;
    xoroshiro128plus_state[0] = rotl(s0, 55) ^ s1 ^ (s1 << 14); // a, b
    xoroshiro128plus_state[1] = rotl(s1, 36); // c

    return result;
}

float randomUniformX()
{
    // 24 bits of granularity is the best that can be done with floats while ensuring that the return value lies in [0.0, 1.0).
    return (xoroshiro128plus_next() >> (64 - 24)) / powf(2, 24);
}

float randomNormalX()
{
    // Box-Muller transform
    float radius = sqrtf(-2.f * logf(1.f - randomUniformX()));
    float theta = 2.f * M_PI * randomUniformX();
    return radius * sinf(theta);

    // // Central Limit Theorem
    // const int n = 8;
    // float sum = 0.0;
    // for (int i = 0; i < n; i++) {
    // 	sum += randomUniform();
    // }
    // return (sum - n / 2.f) / sqrtf(n / 12.f);
}

static void testNormal()
{
    MeasureTime<float>::run(overheadInOut, "normal", []() {
        return randomNormalX();
        }, 1);
}

static void testFastApprox()
{
        MeasureTime<double>::run(overheadInOut, "tanh", []() {
        float x = std::tanh(TestBuffers<float>::get());
        return x;
        }, 1);

        MeasureTime<double>::run(overheadInOut, "fastTanh", []() {
        float x = sspo::AudioMath::fastTanh(TestBuffers<float>::get());
        return x;
        }, 1);
}

static void testLookupTable()
{
        MeasureTime<float>::run(overheadInOut, "std::sin", []() {
        float x = std::sin(TestBuffers<float>::get());
        return x;
        }, 1);

        sspo::AudioMath::LookupTable::Table<float> sineTable = sspo::AudioMath::LookupTable::makeTable<float>(2.0f, 5.0f, 0.001f,  [](const float x) -> float { return std::sin(x);});
        MeasureTime<float>::run(overheadInOut, "LookupTable sin", [&sineTable]() {
            float x = sspo::AudioMath::LookupTable::process<float>(sineTable, TestBuffers<float>::get());
        return x;
        }, 1);

        MeasureTime<float>::run(overheadInOut, "std::pow2", []() {
        float x = std::pow(2.0f, TestBuffers<float>::get());
        return x;
        }, 1);

        sspo::AudioMath::LookupTable::Table<float> pow2Table = sspo::AudioMath::LookupTable::makeTable<float>(2.0f, 5.0f, 0.001f,  [](const float x) -> float { return std::pow(2.0f, x);});
        MeasureTime<float>::run(overheadInOut, "LookupTable pow2", [&pow2Table]() {
            float x = sspo::AudioMath::LookupTable::process<float>(pow2Table, TestBuffers<float>::get());
        return x;
        }, 1);

        MeasureTime<float>::run(overheadInOut, "std::pow10", []() {
        float x = std::pow(10.0f, TestBuffers<float>::get());
        return x;
        }, 1);


        sspo::AudioMath::LookupTable::Table<float> pow10Table = sspo::AudioMath::LookupTable::makeTable<float>(-10.1f, 10.1f, 0.001f,  [](const float x) -> float { return std::pow (10.0f, x);});
        MeasureTime<float>::run(overheadInOut, "LookupTable pow10", [&pow10Table]() {
            float x = sspo::AudioMath::LookupTable::process<float>(pow10Table, TestBuffers<float>::get());
        return x;
        }, 1);

        MeasureTime<float>::run(overheadInOut, "std::log10", []() {
        float x = std::log10 ( TestBuffers<float>::get());
        return x;
        }, 1);

        sspo::AudioMath::LookupTable::Table<float> log10Table = sspo::AudioMath::LookupTable::makeTable<float>(0.00001f, 10.1f, 0.001f,  [](const float x) -> float { return std::log10 (x);});
        MeasureTime<float>::run(overheadInOut, "LookupTable log10", [&log10Table]() {
            float x = sspo::AudioMath::LookupTable::process<float>(log10Table, TestBuffers<float>::get());
        return x;
        }, 1);

        MeasureTime<float>::run(overheadInOut, "unison scaler 11th order polynomial", []() {
             float x = sspo::AudioMath::LookupTable::unisonSpreadScalar ( TestBuffers<float>::get());
             return x;
        }, 1);

        sspo::AudioMath::LookupTable::Table<float> usTable = sspo::AudioMath::LookupTable::makeTable<float>(0.00001f, 10.1f, 0.001f,  [](const float x) -> float { return sspo::AudioMath::LookupTable::unisonSpreadScalar(x);});
        MeasureTime<float>::run(overheadInOut, "LookupTable unison scalar", [&usTable]() {
            float x = sspo::AudioMath::LookupTable::process<float>(usTable, TestBuffers<float>::get());
        return x;
        }, 1);

        MeasureTime<float>::run(overheadInOut, "unison scaler lookup::unison", []() {
             float x = lookup.unisonSpread( TestBuffers<float>::get());
             return x;
        }, 1);
}

static void testCircularBuffer()
{
    CircularBuffer<float> c;


    //initial run with inf and ana checks 0.297870
    MeasureTime<double>::run(overheadInOut, "Circular Buffer Write", [&c]()  {
    c.writeBuffer(TestBuffers<float>::get());
    return 0;
    }, 1);

    //initial run with linear interpolate check 0.56
    MeasureTime<double>::run(overheadInOut, "Circular Buffer read interpolate", [&c]()  {
    float x = c.readBuffer(TestBuffers<float>::get() * 1000.0f);
    return x;
    }, 1);
}

static void testHardLimiter()
{
    sspo::Limiter l;
    l.setSampleRate(44100);

    //initial run 8.30
    MeasureTime<double>::run(overheadInOut, "Hard Limiter process", [&l]()  {
        float x = l.process(TestBuffers<float>::get() * 2.0f);
        return x;
    }, 1);
}


using KSDelay = KSDelayComp<TestComposite>;

static void testKSDelay()
{
    KSDelay ks;

    ks.setSampleRate(44100);
    ks.init();

    ks.inputs[KSDelay::IN_INPUT].setVoltage(0, 0);
    ks.inputs[KSDelay::IN_INPUT].setChannels(1);

    //first run 33.16 of one percent
    MeasureTime<double>::run(overheadInOut, "KS Delay", [&ks]()  {
        ks.step();
        return ks.outputs[KSDelay::OUT_OUTPUT].getVoltage(0);
    }, 1);
}



void perfTest()
{
    printf("starting perf test\n");
    fflush(stdout);
  //  setup();
    assert(overheadInOut > 0);
    assert(overheadOutOnly > 0);

    //test1();
    //testNoise (true);
    //testNormal();
    //testFastApprox();
    testCircularBuffer();
    testHardLimiter();
    testKSDelay();
    testLookupTable();
}