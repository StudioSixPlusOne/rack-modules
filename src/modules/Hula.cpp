#include "plugin.hpp"
#include "Hula.h"
#include "WidgetComposite.h"
#include "ctrl/SqMenuItem.h"
#include "widgets.h"
#include <atomic>
#include <assert.h>
#include "AudioMath.h"
#include "widgets.h"

using Comp = HulaComp<WidgetComposite>;

struct Hula : Module
{
    std::shared_ptr<Comp> hula;

    Hula()
    {
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
        hula = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);
        onSampleRateChange();
        hula->init();
    }

    void process (const ProcessArgs& args) override
    {
        hula->step();
    }

    void onSampleRateChange() override
    {
        float rate = SqHelper::engineGetSampleRate();
        hula->setSampleRate (rate);
    }
};

// *************** UI

struct HulaWidget : ModuleWidget
{
    HulaWidget (Hula* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setPanel (this, "res/Hula.svg");

        addChild (createWidget<ScrewSilver> (Vec (0, 0)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 1 * RACK_GRID_WIDTH, 0)));
        addChild (createWidget<ScrewSilver> (Vec (0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild (createWidget<ScrewSilver> (Vec (box.size.x - 1 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addParam (createParamCentered<sspo::LargeKnob> (mm2px (Vec (15.235, 26.803)), module, Comp::RATIO_PARAM));
        addParam (createParamCentered<sspo::SnapKnob> (mm2px (Vec (9.26, 50.302)), module, Comp::SEMITONE_PARAM));
        addParam (createParamCentered<sspo::SnapKnob> (mm2px (Vec (21.392, 50.542)), module, Comp::OCTAVE_PARAM));
        //addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (25.135, 72.792)), module, Comp::DEPTH_CV_ATTENUVERTER_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (21.392, 73.032)), module, Comp::DEPTH_PARAM));
        //addParam (createParamCentered<RoundBlackKnob> (mm2px (Vec (25.135, 96.353)), module, Comp::FEEDBACK_CV_ATTENUVERTER_PARAM));
        addParam (createParamCentered<sspo::Knob> (mm2px (Vec (21.392, 96.594)), module, Comp::FEEDBACK_PARAM));

        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (9.26, 72.792)), module, Comp::DEPTH_CV_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (9.26, 96.353)), module, Comp::FEEDBACK_CV_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (6.169, 112.865)), module, Comp::VOCT_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (15.679, 112.865)), module, Comp::FM_INPUT));

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (25.188, 112.865)), module, Comp::MAIN_OUTPUT));
    }

    void appendContextMenu (Menu* menu) override;
};

void HulaWidget::appendContextMenu (Menu* menu)
{
    auto* module = dynamic_cast<Hula*> (this->module);

    menu->addChild (new MenuEntry);

    auto* unisonSlider = new sspo::IntSlider;
    unisonSlider->quantity = module->getParamQuantity (Comp::UNISON_PARAM);
    unisonSlider->box.size.x = 200.0f;
    menu->addChild (unisonSlider);
}

Model* modelHula = createModel<Hula, HulaWidget> ("Hula");
