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
}
