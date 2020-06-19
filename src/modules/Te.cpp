#include "plugin.hpp"

struct Te : Module
{
    enum ParamIds
    {
        NUM_PARAMS
    };
    enum InputIds
    {
        NUM_INPUTS
    };
    enum OutputIds
    {
        TRIG_OUTPUT,
        SHUFFLE_OUTPUT,
        A_OUTPUT,
        B_OUTPUT,
        RNG_OUTPUT,
        RESET_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    Te()
    {
        config (NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    void process (const ProcessArgs& args) override
    {
    }
};

struct TeWidget : ModuleWidget
{
    TeWidget (Te* module)
    {
        setModule (module);
        setPanel (APP->window->loadSvg (asset::plugin (pluginInstance, "res/Te.svg")));

        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (7.65, 21.237)), module, Te::TRIG_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (7.62, 39.49)), module, Te::SHUFFLE_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (7.62, 57.68)), module, Te::A_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (7.62, 75.87)), module, Te::B_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (7.62, 94.06)), module, Te::RNG_OUTPUT));
        addOutput (createOutputCentered<PJ301MPort> (mm2px (Vec (7.62, 112.581)), module, Te::RESET_OUTPUT));
    }
};

Model* modelTe = createModel<Te, TeWidget> ("Te");