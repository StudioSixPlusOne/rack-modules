#include "plugin.hpp"
#include "ctrl/SqHelper.h"

Plugin* pluginInstance = nullptr;

void init (::rack::Plugin* p)
{
    pluginInstance = p;

    p->addModel (modelKSDelay);
    p->addModel (modelMaccomo);
    p->addModel (modelPolyShiftRegister);
    p->addModel (modelTe);
    p->addModel (modelCombFilter);
    p->addModel (modelLaLa);
    p->addModel (modelEva);
    p->addModel (modelZazel);
    p->addModel (modelIverson);
    p->addModel (modelIversonJr);
    p->addModel (modelZilah);
    p->addModel (modelHula);
    p->addModel (modelAmburgh);
    p->addModel (modelBascom);
    p->addModel (modelBascomExpander);
    p->addModel (modelMix);
    p->addModel (modelBose);
    p->addModel (modelDuffy);
    p->addModel (modelFarini);
    p->addModel (modelLalaStereo);
    p->addModel (modelThru);
    p->addModel (modelPatchNotes);
    p->addModel (modelMARS_Basic_Comb_Filter);
    // ADD ADDMODEL
}
