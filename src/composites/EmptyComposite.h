/*
 * Copyright (c) 2020 Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#pragma once

#include "IComposite.h"

namespace rack
{
    namespace engine
    {
        struct Module;
    }
} // namespace rack
using Module = ::rack::engine::Module;
using namespace rack;

template <class TBase>
class MaccomoDescription : public IComposite
{
public:
    Config getParam (int i) override;
    int getNumParams() override;
};

/**
 * Complete Maccomo composite
 *
 * If TBase is WidgetComposite, this class is used as the implementation part of the KSDelay module.
 * If TBase is TestComposite, this class may stand alone for unit tests.
 */

template <class TBase>
class MaccomoComp : public TBase
{
public:
    CombFilterComp (Module* module) : TBase (module)
    {
    }

    CombFilterComp() : TBase()
    {
    }

    virtual ~CombFilterComp()
    {
    }

    /** Implement IComposite
     */
    static std::shared_ptr<IComposite> getDescription()
    {
        return std::make_shared<CombFilterDescription<TBase>>();
    }

    void setSampleRate (float rate)
    {
        //TODO
    }

    // must be called after setSampleRate
    void init()
    {
        //TODO
    }

    //TODO params io enums

    //TODO member variables

    void step() override;
};

template <class TBase>
inline void MaccomoComp<TBase>::step()
{
    //TODO step
}

template <class TBase>
int MaccomoDescription<TBase>::getNumParams()
{
    return CombFilterComp<TBase>::NUM_PARAMS;
}

template <class TBase>
IComposite::Config MaccomoDescription<TBase>::getParam (int i)
{
    IComposite::Config ret = { 0.0f, 1.0f, 0.0f, "Code type", "unit", 0.0f, 1.0f, 0.0f };
    switch (i)
    {
        //TODO
        case CombFilterComp<TBase>::FREQUENCY_PARAM:
            ret = { -4.0f, 4.0f, 0.0f, "Frequency", " ", 0, 1, 0.0f };
            break;
        default:
            assert (false);
    }
    return ret;
}