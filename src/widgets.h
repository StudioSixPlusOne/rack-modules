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
#include "componentlibrary.hpp"

namespace sspo
{
    struct SmallKnob : RoundKnob
    {
        SmallKnob()
        {
            setSvg (APP->window->loadSvg (asset::plugin (pluginInstance,
                                                         "res/SspoSmallKnob.svg")));
        }
    };

    struct LargeKnob : RoundKnob
    {
        LargeKnob()
        {
            setSvg (APP->window->loadSvg (asset::plugin (pluginInstance,
                                                         "res/SspoLargeKnob.svg")));
        }
    };

    struct Knob : RoundKnob
    {
        Knob()
        {
            setSvg (APP->window->loadSvg (asset::plugin (pluginInstance,
                                                         "res/SspoKnob.svg")));
        }
    };

    struct SmallSnapKnob : RoundKnob
    {
        SmallSnapKnob()
        {
            snap = true;
            setSvg (APP->window->loadSvg (asset::plugin (pluginInstance,
                                                         "res/SspoSmallKnob.svg")));
        }
    };

    struct LargeSnapKnob : RoundKnob
    {
        LargeSnapKnob()
        {
            snap = true;
            setSvg (APP->window->loadSvg (asset::plugin (pluginInstance,
                                                         "res/SspoLargeKnob.svg")));
        }
    };

    struct SnapKnob : RoundKnob
    {
        SnapKnob()
        {
            snap = true;
            setSvg (APP->window->loadSvg (asset::plugin (pluginInstance,
                                                         "res/SspoKnob.svg")));
        }
    };

    struct NldKnob : RoundKnob
    {
        NldKnob()
        {
            snap = true;
            setSvg (APP->window->loadSvg (asset::plugin (pluginInstance,
                                                         "res/SspoKnob.svg")));
        }

    };

    struct PJ301MPort : app::SvgPort
    {
        PJ301MPort()
        {
            setSvg (APP->window->loadSvg (asset::plugin (pluginInstance,
                                                         "res/SspoPort.svg")));
        }
    };
} // namespace sspo
