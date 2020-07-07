#include "plugin.hpp"
#include "LaLa.h"
#include "WidgetComposite.h"
#include "ctrl/SqMenuItem.h"
#include "widgets.h"

using Comp = LaLaComp<WidgetComposite>;

struct LaLa : Module
{
    std::shared_ptr<Comp> lala;

    LaLa()
    {
        config (Comp::NUM_PARAMS, Comp::NUM_INPUTS, Comp::NUM_OUTPUTS, Comp::NUM_LIGHTS);
        lala = std::make_shared<Comp> (this);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        SqHelper::setupParams (icomp, this);

        onSampleRateChange();
        lala->init();
    }

    void onSampleRateChange() override
    {
        float rate = SqHelper::engineGetSampleRate();
        lala->setSampleRate (rate);
    }

    void process (const ProcessArgs& args) override
    {
        lala->step();
    }
};

/*****************************************************
User Interface
*****************************************************/

struct LaLaWidget : ModuleWidget
{
    LaLaWidget (LaLa* module)
    {
        setModule (module);
        std::shared_ptr<IComposite> icomp = Comp::getDescription();
        box.size = Vec (8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        SqHelper::setPanel (this, "res/LaLa.svg");

        addParam (SqHelper::createParamCentered<sspo::LargeKnob> (icomp, mm2px (Vec (7.607, 28.962)), module, Comp::FREQ_PARAM));
        addParam (SqHelper::createParamCentered<sspo::Knob> (icomp, mm2px (Vec (7.658, 41.774)), module, Comp::FREQ_CV_PARAM));

        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.65, 52.668)), module, Comp::FREQ_CV_INPUT));
        addInput (createInputCentered<sspo::PJ301MPort> (mm2px (Vec (7.62, 69.806)), module, Comp::MAIN_INPUT));

        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (7.62, 87.289)), module, Comp::HIGH_OUTPUT));
        addOutput (createOutputCentered<sspo::PJ301MPort> (mm2px (Vec (7.62, 105.809)), module, Comp::LOW_OUTPUT));
    }
};

Model* modelLaLa = createModel<LaLa, LaLaWidget> ("LaLa");