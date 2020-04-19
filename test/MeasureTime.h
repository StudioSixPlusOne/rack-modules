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

#include <functional>
#define __STDC_FORMAT_MACROS 
#include <inttypes.h>
#include "TimeStatsCollector.h"

template <typename T>
class TestBuffers;


/**
 * class MeasureTime.
 *
 * Used to estimate the amount of time a function takes to execute.
 * Will run the function over and over in a tight loop. Return value of function
 * is saved to testBuffers. Otherwise the compiler might optimize the whole thing away.
 * Usually ends by printing out the data.
 *
 * One of that statistics printed out is "quota used per 1 percent". Here we are arbitrarily
 * stating that a synthesizer module "should" use only one percent of the available audio processing CPU.
 * This won't be the case for all plugins/developers, but certainly is a plausible benchmark.
 *
 * A big caveat to all this is that code running inside VCV Rack will most likely run slower than the
 * same code running in at right loop. Some reasons are:
 *      - VCV's audio processing thread probably has some internal overheard, so
 *          it's unlikely that modules will get 100% of one core.
 *      - In a tight loop all the data you code references will probably end up in fast on-chip
 *          cache memory. In VCV your step function is called once, then all the other module's step
 *          functions are called. This may force your data out of cache.
 */
template <typename T>
class MeasureTime
{
public:
    MeasureTime() = delete;       // we are only static

    /**
     * Executes function "func" and measures how long it takes.
     * Will call func in a tight loop lasting minTime seconds.
     * When done, prints out statistics.
     *
     * returns - percent used
     */
    static double run(double overhead, const char * name, std::function<T()> func, float minTime)
    {
        int64_t iterations;
        bool done = false;
        double percent = 0;

        //keep increasing the number of iterations until we last at least minTime seconds
        for (iterations = 100; !done; iterations *= 2) {
            double elapsed = measureTimeSub(func, iterations);
            if (elapsed >= minTime) {
                double itersPerSec = iterations / elapsed;
                double full = 44100;
                percent = full * 100 / itersPerSec;
                percent -= overhead;
                printf("\nmeasure %s over time %f\n", name, minTime);

                printf("did %" PRId64 " iterations in %f seconds\n", iterations, elapsed);
                printf("that's %f per sec\n", itersPerSec);
                printf("percent CPU usage: %f\n", percent);
                printf("best case instances: %f\n", 100 / percent);
                printf("quota used per 1 percent : %f\n", percent * 100);
                fflush(stdout);
                done = true;
            }
        }
        return percent;
    }

   /**
    * Run test iterators time, return total seconds.
    */
    static double measureTimeSub(std::function<T()> func, int64_t iterations)
    {
        const double t0 = SqTime::seconds();
        for (int64_t i = 0; i < iterations; ++i) {
            const T x = func();
            TestBuffers<T>::put(x);
        }

        const double t1 = SqTime::seconds();
        const double elapsed = t1 - t0;
        return elapsed;
    }
};

/**
 * Simple producer / consumer for test data.
 * Serves up a pre-calculated list of random numbers.
 */
template <typename T>
class TestBuffers
{
public:
    static const size_t size = 60000;
    static void put(T x)
    {
        destData[destIndex++] = x;
        if (destIndex >= size) {
            destIndex = 0;
        }
    }
    static T get()
    {
        T ret = sourceData[sourceIndex++];
        if (sourceIndex >= size) {
            sourceIndex = 0;
        }
        return ret;
    }
    //
    TestBuffers()
    {
        for (int i = 0; i < size; ++i) {
            sourceData[i] = (float) rand() / (float) RAND_MAX;
        }
    }
private:
    static size_t sourceIndex;
    static size_t destIndex;
    static T sourceData[size];
    static T destData[size];
};

template <typename T>
T TestBuffers<T>::sourceData[size];

template <typename T>
T TestBuffers<T>::destData[size];

template <typename T>
size_t TestBuffers<T>::sourceIndex = 0;

template <typename T>
size_t TestBuffers<T>::destIndex = 512;
