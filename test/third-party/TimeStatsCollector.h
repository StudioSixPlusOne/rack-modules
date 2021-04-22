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
#include "SqTime.h"
#include <stdio.h>
/**
 * Unfinished time class. Meant to be inserted into other code (like VCV Rack).
 * Averages states over time.
 * Will printf states every now and then.
 * Many ways to improve this - printing from a different thread is one.
 */
class TimeStatsCollector
{
public:
    TimeStatsCollector()
    {
    }

    /**
     * Instrumented code calls this to start a timing sample.
     */
    void start()
    {
        startTime = SqTime::seconds();
    }

    /**
     * Instrumented code calls this to step timing
     * @param numLoops is the number of "events" since start was called.
     */
    void stop(int numLoops)
    {
        const double stop = SqTime::seconds();
        const double delta = stop - startTime;
        numDataPoints += numLoops;
        totalTimeFrame += delta;
        if (delta < minTime) {
            minTime = delta;
        } else if (delta > maxTime) {
            maxTime = delta;
        }

        if (numDataPoints > 1000) {
            const double avgTime = totalTimeFrame / numDataPoints;
            const double srTime = (1.0 / 44100.0);
            const double ratio = avgTime / srTime;
            totalTimeGlobal += avgTime;
            numFramesInTotal++;

            double runningAvg = totalTimeGlobal / (numFramesInTotal * srTime);

            printf("\nsrTime=%f avtTime=%f\n", srTime, avgTime);
            printf("this block: %f%% min=%f max=%f globalmax=%f running avg=%f\n",
                ratio * 100, minTime, maxTime, globalMax, runningAvg * 100);
            if (globalMax < maxTime) {
                globalMax = maxTime;
            }
            numDataPoints = 0;
            totalTimeFrame = 0;
            minTime = 1000000000;
            maxTime = -100000000000;
        }
    }

private:
    int numDataPoints = 0;     // number of samples in current frame.
    double totalTimeFrame = 0; // Time spent in the test code this frame.
    double startTime;          // Start time if current sample
    double minTime = 1000000000;   // shorted time observed in current frame
    double maxTime = -100000000000;// longest time observed in current frame.
    double globalMax = -100000000000;
    double totalTimeGlobal = 0;
    int numFramesInTotal = 0;
};