#include "plugin.hpp"
/* #include "common.hpp"
#include "module.hpp"
#include "componentlibrary.hpp" */
#include "widgets.h"
#include "Iverson.h"
#include "WidgetComposite.h"
#include "ctrl/SqMenuItem.h"

using Comp = IversonComp<WidgetComposite>;

struct Iverson : Module
{
    static constexpr int MAX_SEQUENCE_LENGTH = 64;
    std::shared_ptr<Comp> iverson;

    Iverson()
    {
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);

        iverson = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);
        onSampleRateChange();
        iverson->init();
    }
    void onSampleRateChange() override
    {
        float rate = SqHelper::engineGetSampleRate();
        iverson->setSampleRate (rate);
    }

    void process (const ProcessArgs& args) override
    {
        iverson->step();
    }

    /*****************************************************
User Interface
*****************************************************/
};

struct IversonWidget : ModuleWidget
{
    IversonWidget (Iverson* module)
    {
        setModule (module);

        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (30 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/Iverson.svg");

        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (9.807, 23.709)), module, Comp::GRID_1_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (16.157, 23.709)), module, Comp::GRID_2_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (22.507, 23.709)), module, Comp::GRID_3_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (28.857, 23.709)), module, Comp::GRID_4_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (41.027, 23.709)), module, Comp::GRID_5_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (47.378, 23.709)), module, Comp::GRID_6_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (53.728, 23.709)), module, Comp::GRID_7_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (60.078, 23.709)), module, Comp::GRID_8_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (72.248, 23.709)), module, Comp::GRID_9_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (78.598, 23.709)), module, Comp::GRID_10_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (84.948, 23.709)), module, Comp::GRID_11_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (91.298, 23.709)), module, Comp::GRID_12_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (103.469, 23.709)), module, Comp::GRID_13_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (109.819, 23.709)), module, Comp::GRID_14_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (116.169, 23.709)), module, Comp::GRID_15_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (122.519, 23.709)), module, Comp::GRID_16_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (134.161, 23.709)), module, Comp::MUTE_1_PARAM));
        addParam (SqHelper::createParamCentered<sspo::SmallSnapKnob> (icomp, mm2px (Vec (147.212, 23.709)), module, Comp::LENGTH_1_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (9.807, 32.065)), module, Comp::GRID_1_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (16.157, 32.065)), module, Comp::GRID_2_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (22.507, 32.065)), module, Comp::GRID_3_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (28.857, 32.065)), module, Comp::GRID_4_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (41.027, 32.065)), module, Comp::GRID_5_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (47.378, 32.065)), module, Comp::GRID_6_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (53.728, 32.065)), module, Comp::GRID_7_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (60.078, 32.065)), module, Comp::GRID_8_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (72.248, 32.065)), module, Comp::GRID_9_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (78.598, 32.065)), module, Comp::GRID_10_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (84.948, 32.065)), module, Comp::GRID_11_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (91.298, 32.065)), module, Comp::GRID_12_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (103.469, 32.065)), module, Comp::GRID_13_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (109.819, 32.065)), module, Comp::GRID_14_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (116.169, 32.065)), module, Comp::GRID_15_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (122.519, 32.065)), module, Comp::GRID_16_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (134.161, 32.065)), module, Comp::MUTE_2_PARAM));
        addParam (SqHelper::createParamCentered<sspo::SmallSnapKnob> (icomp, mm2px (Vec (147.212, 32.065)), module, Comp::LENGTH_2_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (9.807, 40.421)), module, Comp::GRID_1_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (16.157, 40.421)), module, Comp::GRID_2_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (22.507, 40.421)), module, Comp::GRID_3_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (28.857, 40.421)), module, Comp::GRID_4_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (41.027, 40.421)), module, Comp::GRID_5_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (47.378, 40.421)), module, Comp::GRID_6_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (53.728, 40.421)), module, Comp::GRID_7_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (60.078, 40.421)), module, Comp::GRID_8_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (72.248, 40.421)), module, Comp::GRID_9_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (78.598, 40.421)), module, Comp::GRID_10_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (84.948, 40.421)), module, Comp::GRID_11_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (91.298, 40.421)), module, Comp::GRID_12_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (103.469, 40.421)), module, Comp::GRID_13_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (109.819, 40.421)), module, Comp::GRID_14_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (116.169, 40.421)), module, Comp::GRID_15_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (122.519, 40.421)), module, Comp::GRID_16_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (134.161, 40.421)), module, Comp::MUTE_3_PARAM));
        addParam (SqHelper::createParamCentered<sspo::SmallSnapKnob> (icomp, mm2px (Vec (147.212, 40.421)), module, Comp::LENGTH_3_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (9.807, 48.776)), module, Comp::GRID_1_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (16.157, 48.776)), module, Comp::GRID_2_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (22.507, 48.776)), module, Comp::GRID_3_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (28.857, 48.776)), module, Comp::GRID_4_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (41.027, 48.776)), module, Comp::GRID_5_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (47.378, 48.776)), module, Comp::GRID_6_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (53.728, 48.776)), module, Comp::GRID_7_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (60.078, 48.776)), module, Comp::GRID_8_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (72.248, 48.776)), module, Comp::GRID_9_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (78.598, 48.776)), module, Comp::GRID_10_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (84.948, 48.776)), module, Comp::GRID_11_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (91.298, 48.776)), module, Comp::GRID_12_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (103.469, 48.776)), module, Comp::GRID_13_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (109.819, 48.776)), module, Comp::GRID_14_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (116.169, 48.776)), module, Comp::GRID_15_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (122.519, 48.776)), module, Comp::GRID_16_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (134.161, 48.776)), module, Comp::MUTE_4_PARAM));
        addParam (SqHelper::createParamCentered<sspo::SmallSnapKnob> (icomp, mm2px (Vec (147.212, 48.776)), module, Comp::LENGTH_4_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (9.807, 57.132)), module, Comp::GRID_1_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (16.157, 57.132)), module, Comp::GRID_2_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (22.507, 57.132)), module, Comp::GRID_3_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (28.857, 57.132)), module, Comp::GRID_4_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (41.027, 57.132)), module, Comp::GRID_5_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (47.378, 57.132)), module, Comp::GRID_6_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (53.728, 57.132)), module, Comp::GRID_7_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (60.078, 57.132)), module, Comp::GRID_8_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (72.248, 57.132)), module, Comp::GRID_9_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (78.598, 57.132)), module, Comp::GRID_10_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (84.948, 57.132)), module, Comp::GRID_11_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (91.298, 57.132)), module, Comp::GRID_12_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (103.469, 57.132)), module, Comp::GRID_13_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (109.819, 57.132)), module, Comp::GRID_14_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (116.169, 57.132)), module, Comp::GRID_15_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (122.519, 57.132)), module, Comp::GRID_16_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (134.161, 57.132)), module, Comp::MUTE_5_PARAM));
        addParam (SqHelper::createParamCentered<sspo::SmallSnapKnob> (icomp, mm2px (Vec (147.212, 57.132)), module, Comp::LENGTH_5_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (9.807, 65.488)), module, Comp::GRID_1_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (16.157, 65.488)), module, Comp::GRID_2_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (22.507, 65.488)), module, Comp::GRID_3_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (28.857, 65.488)), module, Comp::GRID_4_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (41.027, 65.488)), module, Comp::GRID_5_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (47.378, 65.488)), module, Comp::GRID_6_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (53.728, 65.488)), module, Comp::GRID_7_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (60.078, 65.488)), module, Comp::GRID_8_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (72.248, 65.488)), module, Comp::GRID_9_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (78.598, 65.488)), module, Comp::GRID_10_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (84.948, 65.488)), module, Comp::GRID_11_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (91.298, 65.488)), module, Comp::GRID_12_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (103.469, 65.488)), module, Comp::GRID_13_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (109.819, 65.488)), module, Comp::GRID_14_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (116.169, 65.488)), module, Comp::GRID_15_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (122.519, 65.488)), module, Comp::GRID_16_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (134.161, 65.488)), module, Comp::MUTE_6_PARAM));
        addParam (SqHelper::createParamCentered<sspo::SmallSnapKnob> (icomp, mm2px (Vec (147.212, 65.488)), module, Comp::LENGTH_6_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (9.807, 73.843)), module, Comp::GRID_1_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (16.157, 73.843)), module, Comp::GRID_2_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (22.507, 73.843)), module, Comp::GRID_3_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (28.857, 73.843)), module, Comp::GRID_4_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (41.027, 73.843)), module, Comp::GRID_5_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (47.378, 73.843)), module, Comp::GRID_6_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (53.728, 73.843)), module, Comp::GRID_7_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (60.078, 73.843)), module, Comp::GRID_8_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (72.248, 73.843)), module, Comp::GRID_9_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (78.598, 73.843)), module, Comp::GRID_10_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (84.948, 73.843)), module, Comp::GRID_11_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (91.298, 73.843)), module, Comp::GRID_12_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (103.469, 73.843)), module, Comp::GRID_13_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (109.819, 73.843)), module, Comp::GRID_14_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (116.169, 73.843)), module, Comp::GRID_15_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (122.519, 73.843)), module, Comp::GRID_16_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (134.161, 73.843)), module, Comp::MUTE_7_PARAM));
        addParam (SqHelper::createParamCentered<sspo::SmallSnapKnob> (icomp, mm2px (Vec (147.212, 73.843)), module, Comp::LENGTH_7_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (9.807, 82.199)), module, Comp::GRID_1_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (16.157, 82.199)), module, Comp::GRID_2_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (22.507, 82.199)), module, Comp::GRID_3_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (28.857, 82.199)), module, Comp::GRID_4_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (41.027, 82.199)), module, Comp::GRID_5_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (47.378, 82.199)), module, Comp::GRID_6_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (53.728, 82.199)), module, Comp::GRID_7_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (60.078, 82.199)), module, Comp::GRID_8_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (72.248, 82.199)), module, Comp::GRID_9_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (78.598, 82.199)), module, Comp::GRID_10_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (84.948, 82.199)), module, Comp::GRID_11_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (91.298, 82.199)), module, Comp::GRID_12_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (103.469, 82.199)), module, Comp::GRID_13_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (109.819, 82.199)), module, Comp::GRID_14_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (116.169, 82.199)), module, Comp::GRID_15_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (122.519, 82.199)), module, Comp::GRID_16_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (134.161, 82.199)), module, Comp::MUTE_8_PARAM));
        addParam (SqHelper::createParamCentered<sspo::SmallSnapKnob> (icomp, mm2px (Vec (147.212, 82.199)), module, Comp::LENGTH_8_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (9.807, 100.475)), module, Comp::PAGE_LEFT_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (22.507, 100.475)), module, Comp::PAGE_RIGHT_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (134.161, 102.013)), module, Comp::RUN_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (147.211, 102.013)), module, Comp::RESET_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (160.262, 102.013)), module, Comp::CLOCK_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (9.807, 112.101)), module, Comp::SET_LENGTH_PARAM));
        addParam (SqHelper::createParamCentered<LEDButton> (icomp, mm2px (Vec (22.507, 112.101)), module, Comp::MIDI_LEARN_PARAM));

        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (134.161, 112.101)), module, Comp::RUN_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (147.212, 112.101)), module, Comp::RESET_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (160.262, 112.101)), module, Comp::CLOCK_INPUT));

        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 23.709)), module, Comp::TRIGGER_1_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 32.065)), module, Comp::TRIGGER_2_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 40.421)), module, Comp::TRIGGER_3_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 48.776)), module, Comp::TRIGGER_4_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 57.132)), module, Comp::TRIGGER_5_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 65.488)), module, Comp::TRIGGER_6_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 73.843)), module, Comp::TRIGGER_7_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (160.262, 82.199)), module, Comp::TRIGGER_8_OUTPUT));

        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 23.709)), module, Comp::GRID_1_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 23.709)), module, Comp::GRID_2_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 23.709)), module, Comp::GRID_3_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 23.709)), module, Comp::GRID_4_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 23.709)), module, Comp::GRID_5_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 23.709)), module, Comp::GRID_6_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 23.709)), module, Comp::GRID_7_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 23.709)), module, Comp::GRID_8_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 23.709)), module, Comp::GRID_9_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 23.709)), module, Comp::GRID_10_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 23.709)), module, Comp::GRID_11_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 23.709)), module, Comp::GRID_12_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 23.709)), module, Comp::GRID_13_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 23.709)), module, Comp::GRID_14_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 23.709)), module, Comp::GRID_15_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 23.709)), module, Comp::GRID_16_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 23.709)), module, Comp::MUTE_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 32.065)), module, Comp::GRID_1_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 32.065)), module, Comp::GRID_2_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 32.065)), module, Comp::GRID_3_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 32.065)), module, Comp::GRID_4_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 32.065)), module, Comp::GRID_5_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 32.065)), module, Comp::GRID_6_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 32.065)), module, Comp::GRID_7_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 32.065)), module, Comp::GRID_8_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 32.065)), module, Comp::GRID_9_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 32.065)), module, Comp::GRID_10_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 32.065)), module, Comp::GRID_11_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 32.065)), module, Comp::GRID_12_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 32.065)), module, Comp::GRID_13_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 32.065)), module, Comp::GRID_14_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 32.065)), module, Comp::GRID_15_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 32.065)), module, Comp::GRID_16_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 32.065)), module, Comp::MUTE_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 40.421)), module, Comp::GRID_1_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 40.421)), module, Comp::GRID_2_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 40.421)), module, Comp::GRID_3_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 40.421)), module, Comp::GRID_4_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 40.421)), module, Comp::GRID_5_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 40.421)), module, Comp::GRID_6_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 40.421)), module, Comp::GRID_7_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 40.421)), module, Comp::GRID_8_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 40.421)), module, Comp::GRID_9_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 40.421)), module, Comp::GRID_10_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 40.421)), module, Comp::GRID_11_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 40.421)), module, Comp::GRID_12_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 40.421)), module, Comp::GRID_13_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 40.421)), module, Comp::GRID_14_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 40.421)), module, Comp::GRID_15_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 40.421)), module, Comp::GRID_16_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 40.421)), module, Comp::MUTE_3_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 48.776)), module, Comp::GRID_1_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 48.776)), module, Comp::GRID_2_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 48.776)), module, Comp::GRID_3_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 48.776)), module, Comp::GRID_4_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 48.776)), module, Comp::GRID_5_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 48.776)), module, Comp::GRID_6_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 48.776)), module, Comp::GRID_7_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 48.776)), module, Comp::GRID_8_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 48.776)), module, Comp::GRID_9_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 48.776)), module, Comp::GRID_10_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 48.776)), module, Comp::GRID_11_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 48.776)), module, Comp::GRID_12_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 48.776)), module, Comp::GRID_13_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 48.776)), module, Comp::GRID_14_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 48.776)), module, Comp::GRID_15_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 48.776)), module, Comp::GRID_16_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 48.776)), module, Comp::MUTE_4_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 57.132)), module, Comp::GRID_1_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 57.132)), module, Comp::GRID_2_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 57.132)), module, Comp::GRID_3_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 57.132)), module, Comp::GRID_4_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 57.132)), module, Comp::GRID_5_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 57.132)), module, Comp::GRID_6_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 57.132)), module, Comp::GRID_7_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 57.132)), module, Comp::GRID_8_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 57.132)), module, Comp::GRID_9_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 57.132)), module, Comp::GRID_10_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 57.132)), module, Comp::GRID_11_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 57.132)), module, Comp::GRID_12_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 57.132)), module, Comp::GRID_13_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 57.132)), module, Comp::GRID_14_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 57.132)), module, Comp::GRID_15_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 57.132)), module, Comp::GRID_16_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 57.132)), module, Comp::MUTE_5_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 65.488)), module, Comp::GRID_1_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 65.488)), module, Comp::GRID_2_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 65.488)), module, Comp::GRID_3_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 65.488)), module, Comp::GRID_4_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 65.488)), module, Comp::GRID_5_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 65.488)), module, Comp::GRID_6_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 65.488)), module, Comp::GRID_7_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 65.488)), module, Comp::GRID_8_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 65.488)), module, Comp::GRID_9_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 65.488)), module, Comp::GRID_10_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 65.488)), module, Comp::GRID_11_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 65.488)), module, Comp::GRID_12_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 65.488)), module, Comp::GRID_13_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 65.488)), module, Comp::GRID_14_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 65.488)), module, Comp::GRID_15_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 65.488)), module, Comp::GRID_16_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 65.488)), module, Comp::MUTE_6_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 73.843)), module, Comp::GRID_1_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 73.843)), module, Comp::GRID_2_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 73.843)), module, Comp::GRID_3_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 73.843)), module, Comp::GRID_4_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 73.843)), module, Comp::GRID_5_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 73.843)), module, Comp::GRID_6_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 73.843)), module, Comp::GRID_7_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 73.843)), module, Comp::GRID_8_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 73.843)), module, Comp::GRID_9_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 73.843)), module, Comp::GRID_10_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 73.843)), module, Comp::GRID_11_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 73.843)), module, Comp::GRID_12_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 73.843)), module, Comp::GRID_13_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 73.843)), module, Comp::GRID_14_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 73.843)), module, Comp::GRID_15_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 73.843)), module, Comp::GRID_16_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 73.843)), module, Comp::MUTE_7_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 82.199)), module, Comp::GRID_1_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (16.157, 82.199)), module, Comp::GRID_2_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 82.199)), module, Comp::GRID_3_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (28.857, 82.199)), module, Comp::GRID_4_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (41.027, 82.199)), module, Comp::GRID_5_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (47.378, 82.199)), module, Comp::GRID_6_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (53.728, 82.199)), module, Comp::GRID_7_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (60.078, 82.199)), module, Comp::GRID_8_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (72.248, 82.199)), module, Comp::GRID_9_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (78.598, 82.199)), module, Comp::GRID_10_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (84.948, 82.199)), module, Comp::GRID_11_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (91.298, 82.199)), module, Comp::GRID_12_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (103.469, 82.199)), module, Comp::GRID_13_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (109.819, 82.199)), module, Comp::GRID_14_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (116.169, 82.199)), module, Comp::GRID_15_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (122.519, 82.199)), module, Comp::GRID_16_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 82.199)), module, Comp::MUTE_8_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 100.475)), module, Comp::PAGE_1_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 100.475)), module, Comp::PAGE_2_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (134.161, 102.013)), module, Comp::RUN_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (147.211, 102.013)), module, Comp::RESET_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (160.262, 102.013)), module, Comp::CLOCK_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (9.807, 112.101)), module, Comp::SET_LENGTH_LIGHT));
        addChild (createLightCentered<MediumLight<RedLight>> (mm2px (Vec (22.507, 112.101)), module, Comp::MIDI_LEARN_LIGHT));

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