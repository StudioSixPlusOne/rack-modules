/*
* Copyright (c) 2023 Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>
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

#include <array>
#include <assert.h>
#include <stdio.h>
#include "dsp/digital.hpp"
#include "TestComposite.h"
#include "ExtremeTester.h"
#include "Analyzer.h"
#include "testSignal.h"

#include "Adsr.h"

namespace ts = sspo::TestSignal;

static void testConstructor()
{
    sspo::Adsr_4 adsr;

    auto currentValues = adsr.getCurrentLevels();
    assertEQ (simd::movemask (currentValues == float_4::zero()), 0xf);
    auto currentPhases = adsr.getCurrentStages();
    assertEQ (simd::movemask (currentPhases == float_4 (sspo::Adsr_4::EOC_STAGE)), 0xf);
}

static void testReset()
{
    sspo::Adsr_4 adsr;
    adsr.reset();

    auto currentValues = adsr.getCurrentLevels();
    assertEQ (simd::movemask (currentValues == float_4::zero()), 0xf);
    auto currentPhases = adsr.getCurrentStages();
    assertEQ (simd::movemask (currentPhases == float_4 (sspo::Adsr_4::EOC_STAGE)), 0xf);
}

static void testResetMoveToAttackNoResetOnTrigger()
{
    sspo::Adsr_4 adsr;
    adsr.reset();
    adsr.setResetOnTrigger (false);

    auto gates = float_4 (1.0f, 0.0f, 1.0f, 0.0f);
    adsr.step (gates);
    auto stages = adsr.getCurrentStages();
    auto expectedStage = float_4 (sspo::Adsr_4::ATTACK_STAGE,
                                  sspo::Adsr_4::EOC_STAGE,
                                  sspo::Adsr_4::ATTACK_STAGE,
                                  sspo::Adsr_4::EOC_STAGE);

    assertEQ (simd::movemask (stages == expectedStage), 0xf);
}

static void testResetMoveToPreAttackWithResetOnTrigger()
{
    sspo::Adsr_4 adsr;
    adsr.reset();
    adsr.setResetOnTrigger (true);

    //set the level, if zero the stage will progress from PRE_ATTACK to ATTACK
    adsr.setOutputLevelsDEBUG (float_4::mask());

    auto gates = float_4 (1.0f, 0.0f, 1.0f, 0.0f);
    adsr.step (gates);
    auto phases = adsr.getCurrentStages();
    auto expectedPhases = float_4 (sspo::Adsr_4::PRE_ATTACK_STAGE,
                                   sspo::Adsr_4::EOC_STAGE,
                                   sspo::Adsr_4::PRE_ATTACK_STAGE,
                                   sspo::Adsr_4::EOC_STAGE);
    //    printf ("current phases %f %f %f %f\n", phases[0], phases[1], phases[2], phases[3]);
    //    printf ("expected phases %f %f %f %f\n", expectedPhases[0], expectedPhases[1], expectedPhases[2], expectedPhases[3]);
    assertEQ (simd::movemask (phases == expectedPhases), 0xF);
}

static void testMoveFromPreAttack()
{
    assert (false);
}

static void testAttackTime()
{
    sspo::Adsr_4 adsr;
    adsr.setResetOnTrigger (false);
    auto sampleRate = 44100.0f;
    auto attackTime = float_4 (0.07f); //10ms
    adsr.setParameters (attackTime, 0, 0, 0, sampleRate);
    auto gates = float_4 (1.0f, 0.0f, 0.0f, 0.0f);

    auto sampleCount = 0;
    while (adsr.step (gates)[0] < 1.0f)
        ++sampleCount;

    int expectedSamples = attackTime[0] * sampleRate;

    assertClose (sampleCount, expectedSamples, 1);
}

static void testMoveToDecay()
{
    sspo::Adsr_4 adsr;
    adsr.setResetOnTrigger (false);
    auto sampleRate = 44100.0f;
    auto attackTime = float_4 (0.01f); //10ms
    adsr.setParameters (attackTime, 0, 0, 0, sampleRate);
    auto gates = float_4 (1.0f, 0.0f, 0.0f, 0.0f);

    while (adsr.step (gates)[0] < 1.0f)
        ;

    assertEQ (adsr.getCurrentStages()[0], sspo::Adsr_4::DECAY_STAGE);
}

static void testDecayTime()
{
    sspo::Adsr_4 adsr;
    adsr.setResetOnTrigger (false);
    auto sampleRate = 44100.0f;
    auto attackTime = float_4 (0.01f); //10ms
    auto decayTime = float_4 (0.07f); // 70ms
    adsr.setParameters (attackTime, decayTime, 0, 0, sampleRate);
    auto gates = float_4 (1.0f, 0.0f, 0.0f, 0.0f);

    while (adsr.step (gates)[0] < 1.0f)
    {
        ;
    }
    assertEQ (adsr.getCurrentStages()[0], sspo::Adsr_4::DECAY_STAGE);

    auto sampleCount = 0;
    float lvl = { 0 };

    while (lvl = adsr.step (gates)[0] > 0.0f)
    {
        ++sampleCount;
    }

    int expectedSamples = decayTime[0] * sampleRate;
    assertClose (sampleCount, expectedSamples, 1);
}

static void testMoveToSustainAtCorrectLevel()
{
    sspo::Adsr_4 adsr;
    adsr.setResetOnTrigger (false);
    auto sampleRate = 44100.0f;
    auto attackTime = float_4 (0.01f); //10ms
    auto decayTime = float_4 (0.07f); // 70ms
    auto sustainLvl = float_4 (0.25);
    adsr.setParameters (attackTime, decayTime, sustainLvl, 0, sampleRate);
    auto gates = float_4 (1.0f, 0.0f, 0.0f, 0.0f);

    while (adsr.step (gates)[0] < 1.0f)
    {
        ;
    }
    assertEQ (adsr.getCurrentStages()[0], sspo::Adsr_4::DECAY_STAGE);

    auto sampleCount = 0;
    float lvl = { 0 };

    while (adsr.getCurrentStages()[0] == sspo::Adsr_4::DECAY_STAGE)
    {
        lvl = adsr.step (gates)[0];
    }
    assertEQ (adsr.getCurrentStages()[0], sspo::Adsr_4::SUSTAIN_STAGE);
    assertClose (lvl, sustainLvl[0], 0.001f);
}

static void testHoldSustainLevel()
{
    sspo::Adsr_4 adsr;
    adsr.setResetOnTrigger (false);
    auto sampleRate = 44100.0f;
    auto attackTime = float_4 (0.01f); //10ms
    auto decayTime = float_4 (0.07f); // 70ms
    auto sustainLvl = float_4 (0.25);
    adsr.setParameters (attackTime, decayTime, sustainLvl, 0, sampleRate);
    auto gates = float_4 (1.0f, 0.0f, 0.0f, 0.0f);

    while (adsr.step (gates)[0] < 1.0f)
    {
        ;
    }

    float lvl = { 0 };

    while (adsr.getCurrentStages()[0] == sspo::Adsr_4::DECAY_STAGE)
    {
        adsr.step (gates)[0];
    }

    //hold SUSTAIN for 1000 samples and check levels
    for (auto i = 0U; i < 1000; ++i)
    {
        lvl = adsr.step (gates)[0];
    }

    assertEQ (adsr.getCurrentStages()[0], sspo::Adsr_4::SUSTAIN_STAGE);
    assertClose (lvl, sustainLvl[0], 0.0001f);
}

static void testMoveToReleaseFromAttack()
{
    sspo::Adsr_4 adsr;
    adsr.setResetOnTrigger (false);
    auto sampleRate = 44100.0f;
    auto attackTime = float_4 (0.01f); //10ms
    auto releaseTime = float_4 (0.01f); //10ms
    adsr.setParameters (attackTime, 0, 0, releaseTime, sampleRate);
    auto gates = float_4 (1.0f, 0.0f, 0.0f, 0.0f);

    //move in to attack stage
    for (auto i = 0U; i < 30; ++i)
        adsr.step (gates);

    assertEQ (adsr.getCurrentStages()[0], sspo::Adsr_4::ATTACK_STAGE);

    gates = float_4::zero();
    adsr.step (gates);

    assertEQ (adsr.getCurrentStages()[0], sspo::Adsr_4::RELEASE_STAGE);
}
static void testMoveToReleaseFromDecay()
{
    sspo::Adsr_4 adsr;
    adsr.setResetOnTrigger (false);
    auto sampleRate = 44100.0f;
    auto attackTime = float_4 (0.01f); //10ms
    auto releaseTime = float_4 (0.01f); //10ms
    adsr.setParameters (attackTime, 0, 0, releaseTime, sampleRate);
    auto gates = float_4 (1.0f, 0.0f, 0.0f, 0.0f);

    while (adsr.step (gates)[0] < 1.0f)
        ;

    assertEQ (adsr.getCurrentStages()[0], sspo::Adsr_4::DECAY_STAGE);
    gates = float_4::zero();
    adsr.step (gates);
    assertEQ (adsr.getCurrentStages()[0], sspo::Adsr_4::RELEASE_STAGE);
}
static void testMoveToReleaseFromSustain()
{
    sspo::Adsr_4 adsr;
    adsr.setResetOnTrigger (false);
    auto sampleRate = 44100.0f;
    auto attackTime = float_4 (0.01f); //10ms
    auto decayTime = float_4 (0.07f); // 70ms
    auto sustainLvl = float_4 (0.25);
    auto releaseTime = float_4 (0.01f); //10ms
    adsr.setParameters (attackTime, decayTime, sustainLvl, releaseTime, sampleRate);
    auto gates = float_4 (1.0f, 0.0f, 0.0f, 0.0f);

    while (adsr.step (gates)[0] < 1.0f)
    {
        ;
    }

    float lvl = { 0 };

    while (adsr.getCurrentStages()[0] == sspo::Adsr_4::DECAY_STAGE)
    {
        adsr.step (gates)[0];
    }

    //hold SUSTAIN for 1000 samples and check levels
    for (auto i = 0U; i < 1000; ++i)
    {
        lvl = adsr.step (gates)[0];
    }

    gates = float_4::zero();
    adsr.step (gates);
    assertEQ (adsr.getCurrentStages()[0], sspo::Adsr_4::RELEASE_STAGE);
}

static void testReleaseTime()
{
    sspo::Adsr_4 adsr;
    adsr.setResetOnTrigger (false);
    auto sampleRate = 44100.0f;
    auto attackTime = float_4 (0.01f); //10ms
    auto decayTime = float_4 (0.02f);
    auto sustainLevel = float_4 (0.5f);
    auto releaseTime = float_4 (0.5f);
    adsr.setParameters (attackTime, decayTime, sustainLevel, releaseTime, sampleRate);
    auto gates = float_4 (1.0f, 0.0f, 0.0f, 0.0f);

    while (adsr.step (gates)[0] < 1.0f)
    {
        ;
    }

    assertEQ (adsr.getCurrentStages()[0], sspo::Adsr_4::DECAY_STAGE);
    gates = float_4::zero();

    //release
    int sampleCount = 0;
    while (adsr.step (gates)[0] > 0.01f)
    {
        sampleCount++;
    }
    auto expectedSamples = releaseTime[0] * sampleRate;
    assertClose (sampleCount, int (expectedSamples), 4000);
}

static void testMoveToEOC()
{
    sspo::Adsr_4 adsr;
    adsr.setResetOnTrigger (false);
    auto sampleRate = 44100.0f;
    auto attackTime = float_4 (0.01f); //10ms
    auto decayTime = float_4 (0.02f);
    auto sustainLevel = float_4 (0.5f);
    auto releaseTime = float_4 (0.5f);
    adsr.setParameters (attackTime, decayTime, sustainLevel, releaseTime, sampleRate);
    auto gates = float_4 (1.0f, 0.0f, 0.0f, 0.0f);

    while (adsr.step (gates)[0] < 1.0f)
        ;

    gates = float_4::zero();

    //release

    float lvl{ 0 };
    while (adsr.getCurrentStages()[0] != sspo::Adsr_4::EOC_STAGE)
    {
        adsr.step (gates);
    }

    assertEQ (adsr.getCurrentStages()[0], sspo::Adsr_4::EOC_STAGE);
}

static void testLevel0to1()
{
    std::vector<float> out;
    sspo::Adsr_4 adsr;
    adsr.setResetOnTrigger (false);
    auto sampleRate = 44100.0f;
    auto attackTime = float_4 (0.01f); //10ms
    auto decayTime = float_4 (0.02f);
    auto sustainLevel = float_4 (0.5f);
    auto releaseTime = float_4 (0.5f);
    adsr.setParameters (attackTime, decayTime, sustainLevel, releaseTime, sampleRate);
    auto gates = float_4 (1.0f, 0.0f, 0.0f, 0.0f);

    float lvl;
    while ((lvl = adsr.step (gates)[0]) < 1.0f)
    {
        out.push_back (lvl);
    }

    //release
    gates = float_4::zero();
    for (auto i = 0; i < 50000; ++i)
    {
        out.push_back (adsr.step (gates)[0]);
    }

    auto maxLvl = std::max_element (out.begin(), out.end());
    auto minLvl = std::min_element (out.begin(), out.end());

    assertGE (*minLvl, 0.0f);
    assertLE (*maxLvl, 1.001f);
}

void testAdsr()
{
    printf ("test Adsr\n");
    testConstructor();
    testResetMoveToAttackNoResetOnTrigger();
    testResetMoveToPreAttackWithResetOnTrigger();
    testAttackTime();
    testMoveToDecay();
    testDecayTime();
    testMoveToSustainAtCorrectLevel();
    testHoldSustainLevel();
    testMoveToReleaseFromAttack();
    testMoveToReleaseFromDecay();
    testMoveToReleaseFromSustain();
    testReleaseTime();
    testLevel0to1();
    testMoveToEOC(); //hangs
    //        testMoveFromPreAttack();
}