/*
 MIT License

Copyright (c) 2018 squinkylabs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

#pragma once

/**
 * This was a collection of utilties that work on both VCV 1.0
 * and VCV 0.6.n. Now that 1.0 is out, and this code base requires 1.0,
 * we have removed the 0.6 versions. 
 */
#include <string>
#include "IComposite.h"
#include "rack.hpp"

extern ::rack::plugin::Plugin* pluginInstance;
class SqHelper
{
public:
    static bool contains (const struct ::rack::math::Rect& r, const ::rack::math::Vec& pos)
    {
        return r.isContaining (pos);
    }
    using SvgWidget = ::rack::widget::SvgWidget;
    using SvgSwitch = ::rack::app::SvgSwitch;

    static void setSvg (SvgWidget* widget, std::shared_ptr<::rack::Svg> svg)
    {
        widget->setSvg (svg);
    }
    static void setSvg (::rack::app::SvgKnob* knob, std::shared_ptr<::rack::Svg> svg)
    {
        knob->setSvg (svg);
    }

    /**
     * loads SVG from plugin's assets,
     * unless pathIsAbsolute is ture
     */
    static std::shared_ptr<::rack::Svg> loadSvg (const char* path, bool pathIsAbsolute = false)
    {
        if (pathIsAbsolute)
        {
            return APP->window->loadSvg (path);
        }
        else
        {
            return APP->window->loadSvg (
                SqHelper::assetPlugin (pluginInstance, path));
        }
    }

    static void setPanel (::rack::app::ModuleWidget* widget, const char* path)
    {
        widget->setPanel (APP->window->loadSvg (::rack::asset::plugin (pluginInstance, path)));
    }

    static void openBrowser (const char* url)
    {
        //return ::rack::asset::plugin(plugin, filename);
    }
    static std::string assetPlugin (::rack::plugin::Plugin* plugin, const std::string& filename)
    {
        return ::rack::asset::plugin (plugin, filename);
    }
    static float engineGetSampleRate()
    {
        return APP->engine->getSampleRate();
    }
    static float engineGetSampleTime()
    {
        return APP->engine->getSampleTime();
    }

    template <typename T>
    static T* createParam (
        std::shared_ptr<IComposite> dummy,
        const ::rack::math::Vec& pos,
        ::rack::engine::Module* module,
        int paramId)
    {
        return ::rack::createParam<T> (pos, module, paramId);
    }

    template <typename T>
    static T* createParamCentered (
        std::shared_ptr<IComposite> dummy,
        const ::rack::math::Vec& pos,
        ::rack::engine::Module* module,
        int paramId)
    {
        return ::rack::createParamCentered<T> (pos, module, paramId);
    }

    static const NVGcolor COLOR_WHITE;
    static const NVGcolor COLOR_GREY;
    static const NVGcolor COLOR_BLACK;
    static const NVGcolor COLOR_SQUINKY;

    static void setupParams (
        std::shared_ptr<IComposite> comp,
        ::rack::engine::Module* module)
    {
        const int n = comp->getNumParams();
        for (int i = 0; i < n; ++i)
        {
            auto param = comp->getParam (i);
            std::string paramName (param.name);
            // module->params[i].config(param.min, param.max, param.def, paramName);
            module->configParam (i, param.min, param.max, param.def, paramName, param.unit, param.displayBase, param.displayMultiplier, param.displayOffset);
        }
    }

    static float getValue (::rack::app::ParamWidget* widget)
    {
        return (widget->getParamQuantity()) ? widget->getParamQuantity()->getValue() : 0;
    }

    static void setValue (::rack::app::ParamWidget* widget, float v)
    {
        if (widget->getParamQuantity())
        {
            widget->getParamQuantity()->setValue (v);
        }
    }
};

#define DECLARE_MANUAL(TEXT, URL)                                         \
    void appendContextMenu (Menu* theMenu) override                       \
    {                                                                     \
        ::rack::ui::MenuLabel* spacerLabel = new ::rack::ui::MenuLabel(); \
        theMenu->addChild (spacerLabel);                                  \
        ManualMenuItem* manual = new ManualMenuItem (TEXT, URL);          \
        theMenu->addChild (manual);                                       \
    }
