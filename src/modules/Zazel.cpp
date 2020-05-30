#include "plugin.hpp"
#include "Zazel.h"
#include "WidgetComposite.h"
#include "ctrl/SqMenuItem.h"

using Comp = ZazelComp<WidgetComposite>;

struct Zazel : Module
{
    std::shared_ptr<Comp> zazel;

    Zazel()
    {
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
        zazel = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);
        onSampleRateChange();
        zazel->init();
    }

    void onSampleRateChange() override
    {
        float rate = SqHelper::engineGetSampleRate();
        zazel->setSampleRate (rate);
    }

    void process (const ProcessArgs& args) override
    {
        zazel->step();
    }
};

/*****************************************************
User Interface
*****************************************************/

struct ZazelWidget : ModuleWidget
{
    ZazelWidget (Zazel* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/Zazel.svg");

        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (SqHelper::createParamCentered<RoundLargeBlackKnob> (icomp, mm2px (Vec (48.161, 58.514)), module, Comp::START_PARAM));
        addParam (SqHelper::createParamCentered<RoundBlackKnob> (icomp, mm2px (Vec (28.925, 40.324)), module, Comp::EASING_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<RoundBlackKnob> (icomp, mm2px (Vec (28.925, 58.514)), module, Comp::START_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<RoundLargeBlackKnob> (icomp, mm2px (Vec (48.161, 40.324)), module, Comp::EASING_PARAM));
        addParam (SqHelper::createParamCentered<RoundBlackKnob> (icomp, mm2px (Vec (28.925, 76.704)), module, Comp::END_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<RoundLargeBlackKnob> (icomp, mm2px (Vec (48.161, 76.704)), module, Comp::END_PARAM));
        addParam (SqHelper::createParamCentered<RoundBlackKnob> (icomp, mm2px (Vec (28.925, 94.894)), module, Comp::DURATION_ATTENUVERTER_PARAM));
        addParam (SqHelper::createParamCentered<RoundLargeBlackKnob> (icomp, mm2px (Vec (48.161, 94.894)), module, Comp::DURATION_PARAM));
        addParam (SqHelper::createParamCentered<CKSS> (icomp, mm2px (Vec (5.05, 112.575)), module, Comp::ONESHOT_PARAM));

        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.689, 40.324)), module, Comp::EASING_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.689, 58.514)), module, Comp::START_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.689, 76.704)), module, Comp::END_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (9.689, 94.894)), module, Comp::DURATION_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (40.697, 112.422)), module, Comp::STOP_CONT_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (16.93, 112.575)), module, Comp::CLOCK_INPUT));
        addInput (createInputCentered<PJ301MPort> (mm2px (Vec (28.814, 112.575)), module, Comp::START_CONT_INPUT));

        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (52.581, 112.422)), module, Comp::MAIN_OUTPUT));

        // mm2px(Vec(30.408, 14.084))
        addChild (createWidget<Widget> (mm2px (Vec (5.591, 14.19))));
        // mm2px(Vec(14.142, 14.084))
        addChild (createWidget<Widget> (mm2px (Vec (40.315, 14.19))));
    }
};

Model* modelZazel = createModel<Zazel, ZazelWidget> ("Zazel");