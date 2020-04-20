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

#include <functional>
#include "SqHelper.h"
#include "SqUI.h"
#include "rack.hpp"

/**
 * This menu item takes generic lambdas,
 * so can be used for anything
 **/
struct SqMenuItem : ::rack::MenuItem {
    void onAction(const ::rack::event::Action& e) override {
        _onActionFn();
    }

    void step() override {
        ::rack::MenuItem::step();
        rightText = CHECKMARK(_isCheckedFn());
    }

    SqMenuItem(std::function<bool()> isCheckedFn,
               std::function<void()> clickFn) : _isCheckedFn(isCheckedFn),
                                                _onActionFn(clickFn) {
    }

private:
    std::function<bool()> _isCheckedFn;
    std::function<void()> _onActionFn;
};

struct SqMenuItemAccel : ::rack::MenuItem {
    void onAction(const ::rack::event::Action& e) override {
        _onActionFn();
    }

    SqMenuItemAccel(const char* accelLabel, std::function<void()> clickFn) : _onActionFn(clickFn) {
        rightText = accelLabel;
    }

private:
    std::function<void()> _onActionFn;
};

struct ManualMenuItem : SqMenuItem {
    ManualMenuItem(const char* menuText, const char* url) : SqMenuItem(
                                                                []() { return false; },
                                                                [url]() { SqHelper::openBrowser(url); }) {
        this->text = menuText;
    }
};

/**
 * menu item that toggles a boolean param.
 */
struct SqMenuItem_BooleanParam2 : ::rack::MenuItem {
    SqMenuItem_BooleanParam2(::rack::engine::Module* mod, int id) : paramId(id),
                                                                    module(mod) {
    }

    void onAction(const sq::EventAction& e) override {
        const float newValue = isOn() ? 0 : 1;
        ::rack::appGet()->engine->setParam(module, paramId, newValue);
        e.consume(this);
    }

    void step() override {
        rightText = CHECKMARK(isOn());
    }

private:
    bool isOn() {
        return ::rack::appGet()->engine->getParam(module, paramId) > .5;
    }
    const int paramId;
    ::rack::engine::Module* const module;
};

struct SqMenuItem_BooleanParam : ::rack::MenuItem {
    SqMenuItem_BooleanParam(::rack::ParamWidget* widget) : widget(widget) {
    }

    void onAction(const sq::EventAction& e) override {
        const float newValue = isOn() ? 0 : 1;
        if (widget->paramQuantity) {
            widget->paramQuantity->setValue(newValue);
        }

        sq::EventChange ec;
        widget->onChange(ec);
        e.consume(this);
    }

    void step() override {
        rightText = CHECKMARK(isOn());
    }

private:
    bool isOn() {
        bool ret = false;
        if (widget->paramQuantity) {
            ret = widget->paramQuantity->getValue() > .5f;
        }

        return ret;
    }
    ::rack::ParamWidget* const widget;
};