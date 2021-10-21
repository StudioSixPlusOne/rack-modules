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

#include <rack.hpp>

using Input = ::rack::engine::Input;
using Output = ::rack::engine::Output;
using Param = ::rack::engine::Param;
using Light = ::rack::engine::Light;
using Module = ::rack::engine::Module;

/**
 * Base class for composites embedable in a VCV Widget
 * This is used for "real" implementations
 */
class WidgetComposite
{
public:

    using Port = ::rack::engine::Port;
    
    WidgetComposite(::rack::engine::Module * parent) :
        inputs(parent->inputs),
        outputs(parent->outputs),
        params(parent->params),
        lights(parent->lights),
        paramQuantities(parent->paramQuantities)
    {
    }
    virtual ~WidgetComposite() {}
    virtual void step()
    {
    };
    float engineGetSampleRate()
    {
        return ::rack::engine::Engine().getSampleRate();
    }
    
    float engineGetSampleTime()
    {
        return ::rack::engine::Engine().getSampleTime();
    }
    //protected:
    std::vector<Input>& inputs;
    std::vector<Output>& outputs;
    std::vector<Param>& params;
    std::vector<Light>& lights;
    std::vector<ParamQuantity*>& paramQuantities;

private:
};
