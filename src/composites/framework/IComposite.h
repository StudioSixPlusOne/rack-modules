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
 * Interface that must be implemented by all composites.
 * Enables compiling for v1
 */
class IComposite
{
public:
    class Config
    {
    public:
        Config(float a, float b, float c, const char* n, const char* u 
        , const float base = 0.0f, const float dm = 1.0f, const float offset = 0.0f)
        {
            min=a;
            max=b;
            def=c;
            name=n;
            unit=u;
            displayBase = base;
            displayMultiplier = dm;
            displayOffset = offset;
        }
        float min=0;
        float max=0;
        float def=0;
        const char* name=nullptr; 
        const char* unit = nullptr;
        float displayBase = 0.0f;
        float displayMultiplier = 1.0f;
        float displayOffset = 0.0f;
        // When you add more fields here, make sure 
        // to add them to testIComposite.cpp
        bool active = true;
    };
    virtual Config getParam(int i)=0;
    virtual int getNumParams()=0;
    virtual ~IComposite() {};
};