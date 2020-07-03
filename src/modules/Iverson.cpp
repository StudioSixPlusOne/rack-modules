#include "plugin.hpp"
/* #include "common.hpp"
#include "module.hpp"
#include "componentlibrary.hpp" */
#include "widgets.h"

struct Iverson : Module
{
    enum ParamIds
    {
        GRID_1_1_PARAM,
        GRID_2_1_PARAM,
        GRID_3_1_PARAM,
        GRID_4_1_PARAM,
        GRID_5_1_PARAM,
        GRID_6_1_PARAM,
        GRID_7_1_PARAM,
        GRID_8_1_PARAM,
        GRID_9_1_PARAM,
        GRID_10_1_PARAM,
        GRID_11_1_PARAM,
        GRID_12_1_PARAM,
        GRID_13_1_PARAM,
        GRID_14_1_PARAM,
        GRID_15_1_PARAM,
        GRID_16_1_PARAM,
        MUTE_1_PARAM,
        LENGTH_1_PARAM,
        GRID_1_2_PARAM,
        GRID_2_2_PARAM,
        GRID_3_2_PARAM,
        GRID_4_2_PARAM,
        GRID_5_2_PARAM,
        GRID_6_2_PARAM,
        GRID_7_2_PARAM,
        GRID_8_2_PARAM,
        GRID_9_2_PARAM,
        GRID_10_2_PARAM,
        GRID_11_2_PARAM,
        GRID_12_2_PARAM,
        GRID_13_2_PARAM,
        GRID_14_2_PARAM,
        GRID_15_2_PARAM,
        GRID_16_2_PARAM,
        MUTE_2_PARAM,
        LENGTH_2_PARAM,
        GRID_1_3_PARAM,
        GRID_2_3_PARAM,
        GRID_3_3_PARAM,
        GRID_4_3_PARAM,
        GRID_5_3_PARAM,
        GRID_6_3_PARAM,
        GRID_7_3_PARAM,
        GRID_8_3_PARAM,
        GRID_9_3_PARAM,
        GRID_10_3_PARAM,
        GRID_11_3_PARAM,
        GRID_12_3_PARAM,
        GRID_13_3_PARAM,
        GRID_14_3_PARAM,
        GRID_15_3_PARAM,
        GRID_16_3_PARAM,
        MUTE_3_PARAM,
        LENGTH_3_PARAM,
        GRID_1_4_PARAM,
        GRID_2_4_PARAM,
        GRID_3_4_PARAM,
        GRID_4_4_PARAM,
        GRID_5_4_PARAM,
        GRID_6_4_PARAM,
        GRID_7_4_PARAM,
        GRID_8_4_PARAM,
        GRID_9_4_PARAM,
        GRID_10_4_PARAM,
        GRID_11_4_PARAM,
        GRID_12_4_PARAM,
        GRID_13_4_PARAM,
        GRID_14_4_PARAM,
        GRID_15_4_PARAM,
        GRID_16_4_PARAM,
        MUTE_4_PARAM,
        LENGTH_4_PARAM,
        GRID_1_5_PARAM,
        GRID_2_5_PARAM,
        GRID_3_5_PARAM,
        GRID_4_5_PARAM,
        GRID_5_5_PARAM,
        GRID_6_5_PARAM,
        GRID_7_5_PARAM,
        GRID_8_5_PARAM,
        GRID_9_5_PARAM,
        GRID_10_5_PARAM,
        GRID_11_5_PARAM,
        GRID_12_5_PARAM,
        GRID_13_5_PARAM,
        GRID_14_5_PARAM,
        GRID_15_5_PARAM,
        GRID_16_5_PARAM,
        MUTE_5_PARAM,
        LENGTH_5_PARAM,
        GRID_1_6_PARAM,
        GRID_2_6_PARAM,
        GRID_3_6_PARAM,
        GRID_4_6_PARAM,
        GRID_5_6_PARAM,
        GRID_6_6_PARAM,
        GRID_7_6_PARAM,
        GRID_8_6_PARAM,
        GRID_9_6_PARAM,
        GRID_10_6_PARAM,
        GRID_11_6_PARAM,
        GRID_12_6_PARAM,
        GRID_13_6_PARAM,
        GRID_14_6_PARAM,
        GRID_15_6_PARAM,
        GRID_16_6_PARAM,
        MUTE_6_PARAM,
        LENGTH_6_PARAM,
        GRID_1_7_PARAM,
        GRID_2_7_PARAM,
        GRID_3_7_PARAM,
        GRID_4_7_PARAM,
        GRID_5_7_PARAM,
        GRID_6_7_PARAM,
        GRID_7_7_PARAM,
        GRID_8_7_PARAM,
        GRID_9_7_PARAM,
        GRID_10_7_PARAM,
        GRID_11_7_PARAM,
        GRID_12_7_PARAM,
        GRID_13_7_PARAM,
        GRID_14_7_PARAM,
        GRID_15_7_PARAM,
        GRID_16_7_PARAM,
        MUTE_7_PARAM,
        LENGTH_7_PARAM,
        GRID_1_8_PARAM,
        GRID_2_8_PARAM,
        GRID_3_8_PARAM,
        GRID_4_8_PARAM,
        GRID_5_8_PARAM,
        GRID_6_8_PARAM,
        GRID_7_8_PARAM,
        GRID_8_8_PARAM,
        GRID_9_8_PARAM,
        GRID_10_8_PARAM,
        GRID_11_8_PARAM,
        GRID_12_8_PARAM,
        GRID_13_8_PARAM,
        GRID_14_8_PARAM,
        GRID_15_8_PARAM,
        GRID_16_8_PARAM,
        MUTE_8_PARAM,
        LENGTH_8_PARAM,
        PAGE_1_PARAM,
        PAGE_2_PARAM,
        RUN_PARAM,
        RESET_PARAM,
        CLOCK_PARAM,
        SET_LENGTH_PARAM,
        MIDI_LEARN_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        RUN_INPUT,
        RESET_INPUT,
        CLOCK_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        TRIGGER_1_OUTPUT,
        TRIGGER_2_OUTPUT,
        TRIGGER_3_OUTPUT,
        TRIGGER_4_OUTPUT,
        TRIGGER_5_OUTPUT,
        TRIGGER_6_OUTPUT,
        TRIGGER_7_OUTPUT,
        TRIGGER_8_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        GRID_1_1_LIGHT,
        GRID_2_1_LIGHT,
        GRID_3_1_LIGHT,
        GRID_4_1_LIGHT,
        GRID_5_1_LIGHT,
        GRID_6_1_LIGHT,
        GRID_7_1_LIGHT,
        GRID_8_1_LIGHT,
        GRID_9_1_LIGHT,
        GRID_10_1_LIGHT,
        GRID_11_1_LIGHT,
        GRID_12_1_LIGHT,
        GRID_13_1_LIGHT,
        GRID_14_1_LIGHT,
        GRID_15_1_LIGHT,
        GRID_16_1_LIGHT,
        MUTE_1_LIGHT,
        GRID_1_2_LIGHT,
        GRID_2_2_LIGHT,
        GRID_3_2_LIGHT,
        GRID_4_2_LIGHT,
        GRID_5_2_LIGHT,
        GRID_6_2_LIGHT,
        GRID_7_2_LIGHT,
        GRID_8_2_LIGHT,
        GRID_9_2_LIGHT,
        GRID_10_2_LIGHT,
        GRID_11_2_LIGHT,
        GRID_12_2_LIGHT,
        GRID_13_2_LIGHT,
        GRID_14_2_LIGHT,
        GRID_15_2_LIGHT,
        GRID_16_2_LIGHT,
        MUTE_2_LIGHT,
        GRID_1_3_LIGHT,
        GRID_2_3_LIGHT,
        GRID_3_3_LIGHT,
        GRID_4_3_LIGHT,
        GRID_5_3_LIGHT,
        GRID_6_3_LIGHT,
        GRID_7_3_LIGHT,
        GRID_8_3_LIGHT,
        GRID_9_3_LIGHT,
        GRID_10_3_LIGHT,
        GRID_11_3_LIGHT,
        GRID_12_3_LIGHT,
        GRID_13_3_LIGHT,
        GRID_14_3_LIGHT,
        GRID_15_3_LIGHT,
        GRID_16_3_LIGHT,
        MUTE_3_LIGHT,
        GRID_1_4_LIGHT,
        GRID_2_4_LIGHT,
        GRID_3_4_LIGHT,
        GRID_4_4_LIGHT,
        GRID_5_4_LIGHT,
        GRID_6_4_LIGHT,
        GRID_7_4_LIGHT,
        GRID_8_4_LIGHT,
        GRID_9_4_LIGHT,
        GRID_10_4_LIGHT,
        GRID_11_4_LIGHT,
        GRID_12_4_LIGHT,
        GRID_13_4_LIGHT,
        GRID_14_4_LIGHT,
        GRID_15_4_LIGHT,
        GRID_16_4_LIGHT,
        MUTE_4_LIGHT,
        GRID_1_5_LIGHT,
        GRID_2_5_LIGHT,
        GRID_3_5_LIGHT,
        GRID_4_5_LIGHT,
        GRID_5_5_LIGHT,
        GRID_6_5_LIGHT,
        GRID_7_5_LIGHT,
        GRID_8_5_LIGHT,
        GRID_9_5_LIGHT,
        GRID_10_5_LIGHT,
        GRID_11_5_LIGHT,
        GRID_12_5_LIGHT,
        GRID_13_5_LIGHT,
        GRID_14_5_LIGHT,
        GRID_15_5_LIGHT,
        GRID_16_5_LIGHT,
        MUTE_5_LIGHT,
        GRID_1_6_LIGHT,
        GRID_2_6_LIGHT,
        GRID_3_6_LIGHT,
        GRID_4_6_LIGHT,
        GRID_5_6_LIGHT,
        GRID_6_6_LIGHT,
        GRID_7_6_LIGHT,
        GRID_8_6_LIGHT,
        GRID_9_6_LIGHT,
        GRID_10_6_LIGHT,
        GRID_11_6_LIGHT,
        GRID_12_6_LIGHT,
        GRID_13_6_LIGHT,
        GRID_14_6_LIGHT,
        GRID_15_6_LIGHT,
        GRID_16_6_LIGHT,
        MUTE_6_LIGHT,
        GRID_1_7_LIGHT,
        GRID_2_7_LIGHT,
        GRID_3_7_LIGHT,
        GRID_4_7_LIGHT,
        GRID_5_7_LIGHT,
        GRID_6_7_LIGHT,
        GRID_7_7_LIGHT,
        GRID_8_7_LIGHT,
        GRID_9_7_LIGHT,
        GRID_10_7_LIGHT,
        GRID_11_7_LIGHT,
        GRID_12_7_LIGHT,
        GRID_13_7_LIGHT,
        GRID_14_7_LIGHT,
        GRID_15_7_LIGHT,
        GRID_16_7_LIGHT,
        MUTE_7_LIGHT,
        GRID_1_8_LIGHT,
        GRID_2_8_LIGHT,
        GRID_3_8_LIGHT,
        GRID_4_8_LIGHT,
        GRID_5_8_LIGHT,
        GRID_6_8_LIGHT,
        GRID_7_8_LIGHT,
        GRID_8_8_LIGHT,
        GRID_9_8_LIGHT,
        GRID_10_8_LIGHT,
        GRID_11_8_LIGHT,
        GRID_12_8_LIGHT,
        GRID_13_8_LIGHT,
        GRID_14_8_LIGHT,
        GRID_15_8_LIGHT,
        GRID_16_8_LIGHT,
        MUTE_8_LIGHT,
        PAGE_1_LIGHT,
        PAGE_2_LIGHT,
        RUN_LIGHT,
        RESET_LIGHT,
        CLOCK_LIGHT,
        SET_LENGTH_LIGHT,
        MIDI_LEARN_LIGHT,
        NUM_LIGHTS
    };

    Iverson()
    {
        config (NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam (GRID_1_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_2_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_3_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_4_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_5_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_6_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_7_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_8_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_9_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_10_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_11_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_12_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_13_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_14_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_15_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_16_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (MUTE_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (LENGTH_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_1_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_2_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_3_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_4_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_5_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_6_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_7_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_8_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_9_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_10_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_11_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_12_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_13_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_14_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_15_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_16_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (MUTE_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (LENGTH_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_1_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_2_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_3_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_4_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_5_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_6_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_7_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_8_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_9_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_10_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_11_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_12_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_13_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_14_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_15_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_16_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (MUTE_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (LENGTH_3_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_1_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_2_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_3_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_4_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_5_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_6_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_7_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_8_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_9_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_10_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_11_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_12_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_13_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_14_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_15_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_16_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (MUTE_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (LENGTH_4_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_1_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_2_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_3_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_4_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_5_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_6_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_7_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_8_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_9_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_10_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_11_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_12_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_13_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_14_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_15_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_16_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (MUTE_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (LENGTH_5_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_1_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_2_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_3_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_4_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_5_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_6_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_7_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_8_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_9_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_10_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_11_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_12_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_13_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_14_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_15_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_16_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (MUTE_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (LENGTH_6_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_1_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_2_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_3_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_4_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_5_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_6_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_7_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_8_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_9_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_10_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_11_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_12_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_13_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_14_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_15_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_16_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (MUTE_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (LENGTH_7_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_1_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_2_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_3_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_4_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_5_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_6_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_7_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_8_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_9_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_10_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_11_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_12_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_13_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_14_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_15_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (GRID_16_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (MUTE_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (LENGTH_8_PARAM, 0.f, 1.f, 0.f, "");
        configParam (PAGE_1_PARAM, 0.f, 1.f, 0.f, "");
        configParam (PAGE_2_PARAM, 0.f, 1.f, 0.f, "");
        configParam (RUN_PARAM, 0.f, 1.f, 0.f, "");
        configParam (RESET_PARAM, 0.f, 1.f, 0.f, "");
        configParam (CLOCK_PARAM, 0.f, 1.f, 0.f, "");
        configParam (SET_LENGTH_PARAM, 0.f, 1.f, 0.f, "");
        configParam (MIDI_LEARN_PARAM, 0.f, 1.f, 0.f, "");
    }

    void process (const ProcessArgs& args) override
    {
    }
};

struct IversonWidget : ModuleWidget
{
    IversonWidget (Iverson* module)
    {
        setModule (module);
        setPanel (APP->window->loadSvg (asset::plugin (pluginInstance, "res/Iverson.svg")));

        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (createParamCentered<LEDButton> (mm2px (Vec (9.807, 23.709)), module, Iverson::GRID_1_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (16.157, 23.709)), module, Iverson::GRID_2_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (22.507, 23.709)), module, Iverson::GRID_3_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (28.857, 23.709)), module, Iverson::GRID_4_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (41.027, 23.709)), module, Iverson::GRID_5_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (47.378, 23.709)), module, Iverson::GRID_6_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (53.728, 23.709)), module, Iverson::GRID_7_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (60.078, 23.709)), module, Iverson::GRID_8_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (72.248, 23.709)), module, Iverson::GRID_9_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (78.598, 23.709)), module, Iverson::GRID_10_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (84.948, 23.709)), module, Iverson::GRID_11_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (91.298, 23.709)), module, Iverson::GRID_12_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (103.469, 23.709)), module, Iverson::GRID_13_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (109.819, 23.709)), module, Iverson::GRID_14_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (116.169, 23.709)), module, Iverson::GRID_15_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (122.519, 23.709)), module, Iverson::GRID_16_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (134.161, 23.709)), module, Iverson::MUTE_1_PARAM));
        addParam (createParamCentered<sspo::SmallSnapKnob> (mm2px (Vec (147.212, 23.709)), module, Iverson::LENGTH_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (9.807, 32.065)), module, Iverson::GRID_1_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (16.157, 32.065)), module, Iverson::GRID_2_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (22.507, 32.065)), module, Iverson::GRID_3_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (28.857, 32.065)), module, Iverson::GRID_4_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (41.027, 32.065)), module, Iverson::GRID_5_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (47.378, 32.065)), module, Iverson::GRID_6_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (53.728, 32.065)), module, Iverson::GRID_7_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (60.078, 32.065)), module, Iverson::GRID_8_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (72.248, 32.065)), module, Iverson::GRID_9_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (78.598, 32.065)), module, Iverson::GRID_10_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (84.948, 32.065)), module, Iverson::GRID_11_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (91.298, 32.065)), module, Iverson::GRID_12_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (103.469, 32.065)), module, Iverson::GRID_13_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (109.819, 32.065)), module, Iverson::GRID_14_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (116.169, 32.065)), module, Iverson::GRID_15_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (122.519, 32.065)), module, Iverson::GRID_16_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (134.161, 32.065)), module, Iverson::MUTE_2_PARAM));
        addParam (createParamCentered<sspo::SmallSnapKnob> (mm2px (Vec (147.212, 32.065)), module, Iverson::LENGTH_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (9.807, 40.421)), module, Iverson::GRID_1_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (16.157, 40.421)), module, Iverson::GRID_2_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (22.507, 40.421)), module, Iverson::GRID_3_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (28.857, 40.421)), module, Iverson::GRID_4_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (41.027, 40.421)), module, Iverson::GRID_5_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (47.378, 40.421)), module, Iverson::GRID_6_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (53.728, 40.421)), module, Iverson::GRID_7_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (60.078, 40.421)), module, Iverson::GRID_8_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (72.248, 40.421)), module, Iverson::GRID_9_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (78.598, 40.421)), module, Iverson::GRID_10_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (84.948, 40.421)), module, Iverson::GRID_11_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (91.298, 40.421)), module, Iverson::GRID_12_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (103.469, 40.421)), module, Iverson::GRID_13_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (109.819, 40.421)), module, Iverson::GRID_14_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (116.169, 40.421)), module, Iverson::GRID_15_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (122.519, 40.421)), module, Iverson::GRID_16_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (134.161, 40.421)), module, Iverson::MUTE_3_PARAM));
        addParam (createParamCentered<sspo::SmallSnapKnob> (mm2px (Vec (147.212, 40.421)), module, Iverson::LENGTH_3_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (9.807, 48.776)), module, Iverson::GRID_1_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (16.157, 48.776)), module, Iverson::GRID_2_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (22.507, 48.776)), module, Iverson::GRID_3_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (28.857, 48.776)), module, Iverson::GRID_4_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (41.027, 48.776)), module, Iverson::GRID_5_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (47.378, 48.776)), module, Iverson::GRID_6_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (53.728, 48.776)), module, Iverson::GRID_7_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (60.078, 48.776)), module, Iverson::GRID_8_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (72.248, 48.776)), module, Iverson::GRID_9_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (78.598, 48.776)), module, Iverson::GRID_10_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (84.948, 48.776)), module, Iverson::GRID_11_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (91.298, 48.776)), module, Iverson::GRID_12_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (103.469, 48.776)), module, Iverson::GRID_13_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (109.819, 48.776)), module, Iverson::GRID_14_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (116.169, 48.776)), module, Iverson::GRID_15_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (122.519, 48.776)), module, Iverson::GRID_16_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (134.161, 48.776)), module, Iverson::MUTE_4_PARAM));
        addParam (createParamCentered<sspo::SmallSnapKnob> (mm2px (Vec (147.212, 48.776)), module, Iverson::LENGTH_4_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (9.807, 57.132)), module, Iverson::GRID_1_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (16.157, 57.132)), module, Iverson::GRID_2_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (22.507, 57.132)), module, Iverson::GRID_3_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (28.857, 57.132)), module, Iverson::GRID_4_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (41.027, 57.132)), module, Iverson::GRID_5_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (47.378, 57.132)), module, Iverson::GRID_6_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (53.728, 57.132)), module, Iverson::GRID_7_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (60.078, 57.132)), module, Iverson::GRID_8_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (72.248, 57.132)), module, Iverson::GRID_9_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (78.598, 57.132)), module, Iverson::GRID_10_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (84.948, 57.132)), module, Iverson::GRID_11_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (91.298, 57.132)), module, Iverson::GRID_12_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (103.469, 57.132)), module, Iverson::GRID_13_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (109.819, 57.132)), module, Iverson::GRID_14_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (116.169, 57.132)), module, Iverson::GRID_15_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (122.519, 57.132)), module, Iverson::GRID_16_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (134.161, 57.132)), module, Iverson::MUTE_5_PARAM));
        addParam (createParamCentered<sspo::SmallSnapKnob> (mm2px (Vec (147.212, 57.132)), module, Iverson::LENGTH_5_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (9.807, 65.488)), module, Iverson::GRID_1_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (16.157, 65.488)), module, Iverson::GRID_2_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (22.507, 65.488)), module, Iverson::GRID_3_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (28.857, 65.488)), module, Iverson::GRID_4_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (41.027, 65.488)), module, Iverson::GRID_5_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (47.378, 65.488)), module, Iverson::GRID_6_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (53.728, 65.488)), module, Iverson::GRID_7_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (60.078, 65.488)), module, Iverson::GRID_8_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (72.248, 65.488)), module, Iverson::GRID_9_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (78.598, 65.488)), module, Iverson::GRID_10_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (84.948, 65.488)), module, Iverson::GRID_11_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (91.298, 65.488)), module, Iverson::GRID_12_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (103.469, 65.488)), module, Iverson::GRID_13_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (109.819, 65.488)), module, Iverson::GRID_14_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (116.169, 65.488)), module, Iverson::GRID_15_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (122.519, 65.488)), module, Iverson::GRID_16_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (134.161, 65.488)), module, Iverson::MUTE_6_PARAM));
        addParam (createParamCentered<sspo::SmallSnapKnob> (mm2px (Vec (147.212, 65.488)), module, Iverson::LENGTH_6_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (9.807, 73.843)), module, Iverson::GRID_1_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (16.157, 73.843)), module, Iverson::GRID_2_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (22.507, 73.843)), module, Iverson::GRID_3_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (28.857, 73.843)), module, Iverson::GRID_4_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (41.027, 73.843)), module, Iverson::GRID_5_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (47.378, 73.843)), module, Iverson::GRID_6_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (53.728, 73.843)), module, Iverson::GRID_7_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (60.078, 73.843)), module, Iverson::GRID_8_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (72.248, 73.843)), module, Iverson::GRID_9_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (78.598, 73.843)), module, Iverson::GRID_10_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (84.948, 73.843)), module, Iverson::GRID_11_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (91.298, 73.843)), module, Iverson::GRID_12_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (103.469, 73.843)), module, Iverson::GRID_13_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (109.819, 73.843)), module, Iverson::GRID_14_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (116.169, 73.843)), module, Iverson::GRID_15_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (122.519, 73.843)), module, Iverson::GRID_16_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (134.161, 73.843)), module, Iverson::MUTE_7_PARAM));
        addParam (createParamCentered<sspo::SmallSnapKnob> (mm2px (Vec (147.212, 73.843)), module, Iverson::LENGTH_7_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (9.807, 82.199)), module, Iverson::GRID_1_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (16.157, 82.199)), module, Iverson::GRID_2_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (22.507, 82.199)), module, Iverson::GRID_3_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (28.857, 82.199)), module, Iverson::GRID_4_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (41.027, 82.199)), module, Iverson::GRID_5_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (47.378, 82.199)), module, Iverson::GRID_6_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (53.728, 82.199)), module, Iverson::GRID_7_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (60.078, 82.199)), module, Iverson::GRID_8_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (72.248, 82.199)), module, Iverson::GRID_9_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (78.598, 82.199)), module, Iverson::GRID_10_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (84.948, 82.199)), module, Iverson::GRID_11_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (91.298, 82.199)), module, Iverson::GRID_12_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (103.469, 82.199)), module, Iverson::GRID_13_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (109.819, 82.199)), module, Iverson::GRID_14_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (116.169, 82.199)), module, Iverson::GRID_15_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (122.519, 82.199)), module, Iverson::GRID_16_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (134.161, 82.199)), module, Iverson::MUTE_8_PARAM));
        addParam (createParamCentered<sspo::SmallSnapKnob> (mm2px (Vec (147.212, 82.199)), module, Iverson::LENGTH_8_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (9.807, 100.475)), module, Iverson::PAGE_1_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (22.507, 100.475)), module, Iverson::PAGE_2_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (134.161, 102.013)), module, Iverson::RUN_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (147.211, 102.013)), module, Iverson::RESET_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (160.262, 102.013)), module, Iverson::CLOCK_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (9.807, 112.101)), module, Iverson::SET_LENGTH_PARAM));
        addParam (createParamCentered<LEDButton> (mm2px (Vec (22.507, 112.101)), module, Iverson::MIDI_LEARN_PARAM));

        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (134.161, 112.101)), module, Iverson::RUN_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (147.212, 112.101)), module, Iverson::RESET_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (160.262, 112.101)), module, Iverson::CLOCK_INPUT));

        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 23.709)), module, Iverson::TRIGGER_1_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 32.065)), module, Iverson::TRIGGER_2_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 40.421)), module, Iverson::TRIGGER_3_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 48.776)), module, Iverson::TRIGGER_4_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 57.132)), module, Iverson::TRIGGER_5_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 65.488)), module, Iverson::TRIGGER_6_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 73.843)), module, Iverson::TRIGGER_7_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 82.199)), module, Iverson::TRIGGER_8_OUTPUT));

        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 23.709)), module, Iverson::GRID_1_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 23.709)), module, Iverson::GRID_2_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 23.709)), module, Iverson::GRID_3_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 23.709)), module, Iverson::GRID_4_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 23.709)), module, Iverson::GRID_5_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 23.709)), module, Iverson::GRID_6_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 23.709)), module, Iverson::GRID_7_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 23.709)), module, Iverson::GRID_8_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 23.709)), module, Iverson::GRID_9_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 23.709)), module, Iverson::GRID_10_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 23.709)), module, Iverson::GRID_11_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 23.709)), module, Iverson::GRID_12_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 23.709)), module, Iverson::GRID_13_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 23.709)), module, Iverson::GRID_14_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 23.709)), module, Iverson::GRID_15_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 23.709)), module, Iverson::GRID_16_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 23.709)), module, Iverson::MUTE_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 32.065)), module, Iverson::GRID_1_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 32.065)), module, Iverson::GRID_2_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 32.065)), module, Iverson::GRID_3_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 32.065)), module, Iverson::GRID_4_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 32.065)), module, Iverson::GRID_5_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 32.065)), module, Iverson::GRID_6_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 32.065)), module, Iverson::GRID_7_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 32.065)), module, Iverson::GRID_8_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 32.065)), module, Iverson::GRID_9_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 32.065)), module, Iverson::GRID_10_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 32.065)), module, Iverson::GRID_11_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 32.065)), module, Iverson::GRID_12_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 32.065)), module, Iverson::GRID_13_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 32.065)), module, Iverson::GRID_14_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 32.065)), module, Iverson::GRID_15_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 32.065)), module, Iverson::GRID_16_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 32.065)), module, Iverson::MUTE_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 40.421)), module, Iverson::GRID_1_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 40.421)), module, Iverson::GRID_2_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 40.421)), module, Iverson::GRID_3_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 40.421)), module, Iverson::GRID_4_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 40.421)), module, Iverson::GRID_5_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 40.421)), module, Iverson::GRID_6_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 40.421)), module, Iverson::GRID_7_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 40.421)), module, Iverson::GRID_8_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 40.421)), module, Iverson::GRID_9_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 40.421)), module, Iverson::GRID_10_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 40.421)), module, Iverson::GRID_11_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 40.421)), module, Iverson::GRID_12_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 40.421)), module, Iverson::GRID_13_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 40.421)), module, Iverson::GRID_14_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 40.421)), module, Iverson::GRID_15_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 40.421)), module, Iverson::GRID_16_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 40.421)), module, Iverson::MUTE_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 48.776)), module, Iverson::GRID_1_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 48.776)), module, Iverson::GRID_2_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 48.776)), module, Iverson::GRID_3_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 48.776)), module, Iverson::GRID_4_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 48.776)), module, Iverson::GRID_5_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 48.776)), module, Iverson::GRID_6_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 48.776)), module, Iverson::GRID_7_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 48.776)), module, Iverson::GRID_8_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 48.776)), module, Iverson::GRID_9_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 48.776)), module, Iverson::GRID_10_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 48.776)), module, Iverson::GRID_11_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 48.776)), module, Iverson::GRID_12_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 48.776)), module, Iverson::GRID_13_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 48.776)), module, Iverson::GRID_14_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 48.776)), module, Iverson::GRID_15_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 48.776)), module, Iverson::GRID_16_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 48.776)), module, Iverson::MUTE_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 57.132)), module, Iverson::GRID_1_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 57.132)), module, Iverson::GRID_2_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 57.132)), module, Iverson::GRID_3_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 57.132)), module, Iverson::GRID_4_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 57.132)), module, Iverson::GRID_5_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 57.132)), module, Iverson::GRID_6_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 57.132)), module, Iverson::GRID_7_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 57.132)), module, Iverson::GRID_8_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 57.132)), module, Iverson::GRID_9_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 57.132)), module, Iverson::GRID_10_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 57.132)), module, Iverson::GRID_11_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 57.132)), module, Iverson::GRID_12_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 57.132)), module, Iverson::GRID_13_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 57.132)), module, Iverson::GRID_14_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 57.132)), module, Iverson::GRID_15_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 57.132)), module, Iverson::GRID_16_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 57.132)), module, Iverson::MUTE_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 65.488)), module, Iverson::GRID_1_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 65.488)), module, Iverson::GRID_2_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 65.488)), module, Iverson::GRID_3_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 65.488)), module, Iverson::GRID_4_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 65.488)), module, Iverson::GRID_5_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 65.488)), module, Iverson::GRID_6_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 65.488)), module, Iverson::GRID_7_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 65.488)), module, Iverson::GRID_8_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 65.488)), module, Iverson::GRID_9_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 65.488)), module, Iverson::GRID_10_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 65.488)), module, Iverson::GRID_11_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 65.488)), module, Iverson::GRID_12_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 65.488)), module, Iverson::GRID_13_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 65.488)), module, Iverson::GRID_14_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 65.488)), module, Iverson::GRID_15_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 65.488)), module, Iverson::GRID_16_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 65.488)), module, Iverson::MUTE_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 73.843)), module, Iverson::GRID_1_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 73.843)), module, Iverson::GRID_2_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 73.843)), module, Iverson::GRID_3_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 73.843)), module, Iverson::GRID_4_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 73.843)), module, Iverson::GRID_5_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 73.843)), module, Iverson::GRID_6_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 73.843)), module, Iverson::GRID_7_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 73.843)), module, Iverson::GRID_8_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 73.843)), module, Iverson::GRID_9_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 73.843)), module, Iverson::GRID_10_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 73.843)), module, Iverson::GRID_11_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 73.843)), module, Iverson::GRID_12_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 73.843)), module, Iverson::GRID_13_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 73.843)), module, Iverson::GRID_14_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 73.843)), module, Iverson::GRID_15_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 73.843)), module, Iverson::GRID_16_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 73.843)), module, Iverson::MUTE_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 82.199)), module, Iverson::GRID_1_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 82.199)), module, Iverson::GRID_2_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 82.199)), module, Iverson::GRID_3_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 82.199)), module, Iverson::GRID_4_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 82.199)), module, Iverson::GRID_5_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 82.199)), module, Iverson::GRID_6_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 82.199)), module, Iverson::GRID_7_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 82.199)), module, Iverson::GRID_8_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 82.199)), module, Iverson::GRID_9_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 82.199)), module, Iverson::GRID_10_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 82.199)), module, Iverson::GRID_11_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 82.199)), module, Iverson::GRID_12_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 82.199)), module, Iverson::GRID_13_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 82.199)), module, Iverson::GRID_14_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 82.199)), module, Iverson::GRID_15_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 82.199)), module, Iverson::GRID_16_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 82.199)), module, Iverson::MUTE_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 100.475)), module, Iverson::PAGE_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 100.475)), module, Iverson::PAGE_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 102.013)), module, Iverson::RUN_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (147.211, 102.013)), module, Iverson::RESET_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (160.262, 102.013)), module, Iverson::CLOCK_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 112.101)), module, Iverson::SET_LENGTH_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 112.101)), module, Iverson::MIDI_LEARN_LIGHT));

        /*         // mm2px(Vec(60.747, 69.094))
        addChild (createWidget<Widget> (mm2px (Vec (4.515, 18.372))));
        // mm2px(Vec(60.747, 69.094))
        addChild (createWidget<Widget> (mm2px (Vec (66.957, 18.372))));
        // mm2px(Vec(39.881, 16.094))
        addChild (createWidget<Widget> (mm2px (Vec (38.646, 98.094))));
        // mm2px(Vec(39.881, 16.094))
        addChild (createWidget<Widget> (mm2px (Vec (85.02, 98.094)))); */
    }
};

Model* modelIverson = createModel<Iverson, IversonWidget> ("Iverson");