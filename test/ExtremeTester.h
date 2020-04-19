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

#include "asserts.h"
#include <vector>

#include "TestComposite.h"

/**
 * Utility class that counts up in binary using an array of ints to hold the bits.
 *
 */
class BitCounter
{
public:
    void reset(int size)
    {
        done = false;
        state.resize(size);
        for (int i = 0; i < size; ++i) {
            state[i] = 0;
        }
    }
    bool atMax() const
    {
        for (size_t i = 0; i < state.size(); ++i) {
            if (state[i] == 0) {
                return false;
            }
        }
        return true;
    }
    bool isDone() const
    {
        return done;
    }
    void next()
    {
        if (atMax()) {
            done = true;
            return;
        }
        state[0]++;
        for (size_t i = 0; i < state.size(); ++i) {
            if (state[i] > 1) {
                state[i] = 0;
                ++state[i + 1];
            }
        }

    }



    void setInputState(std::vector<Input>& testSignal, const std::vector< std::pair<float, float>>* signalLimits)
    {
        for (int i = (int) state.size() - 1; i >= 0; --i) {
            float min = -10.f;
            float max = 10.f;
            if (signalLimits) { // here we want to clip these to the possible values of params
                min = (*signalLimits)[i].first;
                max = (*signalLimits)[i].second;

                assertNE(min, max);
                assertGT(max, min);
            }
            testSignal[i].setVoltage((state[i] > 0 ? max : min), 0);
        }
    }

    void setParamState(std::vector<TestComposite::Param>& testSignal, const std::vector< std::pair<float, float>>* signalLimits)
    {
        for (int i = (int) state.size() - 1; i >= 0; --i) {
            float min = -10.f;
            float max = 10.f;
            if (signalLimits) { // here we want to clip these to the possible values of params
                min = (*signalLimits)[i].first;
                max = (*signalLimits)[i].second;

                assertNE(min, max);
                assertGT(max, min);
            }
            testSignal[i].value = state[i] > 0 ? max : min;
        }
    }


    void dump(const char * label)
    {
        printf("State (%s): ", label);
        for (int i = (int) state.size() - 1; i >= 0; --i) {
            printf("%d ", state[i]);
        }
        printf("\n");
    }
private:
    std::vector<int> state;
    bool done;

};


/**
* Tests a composite by feeding all the inputs and parameters (in every possible permutation)
* with extreme inputs. Asserts that the output stays in a semi-sane range.
* Typically this test will fail by triggering an assert in some distant code.
*/
template <typename T>
class ExtremeTester
{
public:

    static void test(T& dut,
        const std::vector< std::pair<float, float>>& paramLimits,
        bool checkOutput,
        const char * testName)
    {
        printf("extreme test starting for %s. %s ....\n", testName, checkOutput ? "test output" : "");
        const int numInputs = dut.NUM_INPUTS;
        const int numParams = dut.NUM_PARAMS;
        const int numOutputs = dut.NUM_OUTPUTS;
        assert(numInputs < 20);
        assertEQ(paramLimits.size(), numParams);

        const std::vector< std::pair<float, float>> * nullLimits = nullptr;
        BitCounter inputState;
        BitCounter paramsState;
        for (inputState.reset(numInputs); !inputState.isDone(); inputState.next()) {
            inputState.setInputState(dut.inputs, nullLimits);
            for (paramsState.reset(numParams); !paramsState.isDone(); paramsState.next()) {
                paramsState.setParamState(dut.params, &paramLimits);
                for (int i = 0; i < 100; ++i) {
                    dut.step();
                    for (int j = 0; j < numOutputs; ++j) {
                        const float out = dut.outputs[j].getVoltage(0);
                        if (checkOutput) {
                            assertGE(out, -150);
                            assertLE(out, 150);
                        }
                    }
                }
            }
        }
        printf("extreme test done\n");
    }
};