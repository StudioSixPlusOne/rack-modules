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

    struct DefaultTuningMenuItem : MenuItem
    {
        Hula* module;
        float tuning;
        void onAction (const event::Action& e) override
        {
            module->params[Comp::DEFAULT_TUNING_PARAM].setValue (tuning);
        }
    };

    void appendContextMenu (Menu* menu) override;
};

void HulaWidget::appendContextMenu (Menu* menu)
{
    auto* module = dynamic_cast<Hula*> (this->module);

    menu->addChild (new MenuEntry);

    auto* dcOffsetSlider = new ui::Slider;
    dcOffsetSlider->quantity = module->getParamQuantity (Comp::DC_OFFSET_PARAM);
    dcOffsetSlider->box.size.x = 200.0f;
    menu->addChild (dcOffsetSlider);

    auto* scaleSlider = new ui::Slider;
    scaleSlider->quantity = module->getParamQuantity (Comp::SCALE_PARAM);
    scaleSlider->box.size.x = 200.0f;
    menu->addChild (scaleSlider);

    auto* unisonSlider = new sspo::IntSlider;
    unisonSlider->quantity = module->getParamQuantity (Comp::UNISON_PARAM);
    unisonSlider->box.size.x = 200.0f;
    menu->addChild (unisonSlider);

    auto* oversampleSlider = new sspo::IntSlider;
    oversampleSlider->quantity = module->getParamQuantity (Comp::OVERSAMPLE_PARAM);
    oversampleSlider->box.size.x = 200.0f;
    menu->addChild (oversampleSlider);

    //Default tuning

    menu->addChild (new MenuEntry);

    MenuLabel* durationLabel = new MenuLabel();
    durationLabel->text = "Default Tuning";
    menu->addChild (durationLabel);

    DefaultTuningMenuItem* tuningAudioMenuItem = new DefaultTuningMenuItem;
    tuningAudioMenuItem->tuning = dsp::FREQ_C4;
    tuningAudioMenuItem->text = "Audio C4";
    tuningAudioMenuItem->module = module;
    tuningAudioMenuItem->rightText = CHECKMARK (module->hula->params[Comp::DEFAULT_TUNING_PARAM].getValue()
                                                == dsp::FREQ_C4);
    menu->addChild (tuningAudioMenuItem);

    DefaultTuningMenuItem* tuningLfoMenuItem = new DefaultTuningMenuItem;
    tuningLfoMenuItem->tuning = 2.0f;
    tuningLfoMenuItem->text = "Clock / Lfo 2hz";
    tuningLfoMenuItem->module = module;
    tuningLfoMenuItem->rightText = CHECKMARK (module->hula->params[Comp::DEFAULT_TUNING_PARAM].getValue()
                                              == 2.0f);
    menu->addChild (tuningLfoMenuItem);

    DefaultTuningMenuItem* tuningSlowLfoMenuItem = new DefaultTuningMenuItem;
    tuningSlowLfoMenuItem->tuning = 0.125f;
    tuningSlowLfoMenuItem->text = "Slow Lfo 0.125hz";
    tuningSlowLfoMenuItem->module = module;
    tuningSlowLfoMenuItem->rightText = CHECKMARK (module->hula->params[Comp::DEFAULT_TUNING_PARAM].getValue()
                                                  == 0.125f);
    menu->addChild (tuningSlowLfoMenuItem);
}

Model* modelHula = createModel<Hula, HulaWidget> ("Hula");
