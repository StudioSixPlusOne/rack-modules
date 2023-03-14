/* 
 * Copyright (c) 2020, 2023 Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>
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
#include "ui/Slider.hpp"
#include <string>
#include "dsp/WaveShaper.h"

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

    struct ExtraLargeKnob : RoundKnob
    {
        ExtraLargeKnob()
        {
            setSvg (APP->window->loadSvg (asset::plugin (pluginInstance,
                                                         "res/SspoExtraLargeKnob.svg")));
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

    struct TwoWaySwitch : app::SvgSwitch
    {
        TwoWaySwitch()
        {
            shadow->opacity = 0.0;
            addFrame (Svg::load (asset::plugin (pluginInstance, "res/SspoSwitch01.svg")));
            addFrame (Svg::load (asset::plugin (pluginInstance, "res/SspoSwitch03.svg")));
        }
    };

    struct ThreeWaySwitch : app::SvgSwitch
    {
        ThreeWaySwitch()
        {
            shadow->opacity = 0.0;
            addFrame (Svg::load (asset::plugin (pluginInstance, "res/SspoSwitch01.svg")));
            addFrame (Svg::load (asset::plugin (pluginInstance, "res/SspoSwitch02.svg")));
            addFrame (Svg::load (asset::plugin (pluginInstance, "res/SspoSwitch03.svg")));
        }
    };

    struct FourWaySwitch : app::SvgSwitch
    {
        FourWaySwitch()
        {
            shadow->opacity = 0.0;
            addFrame (Svg::load (asset::plugin (pluginInstance, "res/Sspo4WaySwitch01.svg")));
            addFrame (Svg::load (asset::plugin (pluginInstance, "res/Sspo4WaySwitch02.svg")));
            addFrame (Svg::load (asset::plugin (pluginInstance, "res/Sspo4WaySwitch03.svg")));
            addFrame (Svg::load (asset::plugin (pluginInstance, "res/Sspo4WaySwitch04.svg")));
        }
    };

    struct IntSlider : ui::Slider
    {
        void draw (const DrawArgs& args) override
        {
            BNDwidgetState state = BND_DEFAULT;
            if (APP->event->hoveredWidget == this)
                state = BND_HOVER;
            if (APP->event->draggedWidget == this)
                state = BND_ACTIVE;

            float progress = quantity ? quantity->getScaledValue() : 0.0f;
            std::string text = quantity ? quantity->getLabel() + " : " : "";
            text += quantity ? std::to_string (int (quantity->getValue())) : "";

            bndSlider (args.vg,
                       0.0,
                       0.0,
                       box.size.x,
                       box.size.y,
                       BND_CORNER_NONE,
                       state,
                       progress,
                       text.c_str(),
                       NULL);
        }
    };

    struct NldSlider : ui::Slider
    {
        void draw (const DrawArgs& args) override
        {
            BNDwidgetState state = BND_DEFAULT;
            if (APP->event->hoveredWidget == this)
                state = BND_HOVER;
            if (APP->event->draggedWidget == this)
                state = BND_ACTIVE;

            float progress = quantity ? quantity->getScaledValue() : 0.0f;
            std::string text = quantity ? quantity->getLabel() + " : " : "";
            text += quantity
                        ? sspo::AudioMath::WaveShaper::nld.getShapeName (int (quantity->getValue()))
                        : "";

            bndSlider (args.vg,
                       0.0,
                       0.0,
                       box.size.x,
                       box.size.y,
                       BND_CORNER_NONE,
                       state,
                       progress,
                       text.c_str(),
                       NULL);
        }
    };

    inline NVGcolor green()
    {
        return nvgRGB (0x00, 128, 0x00);
    }

    inline NVGcolor ledDisplayGreen()
    {
        return nvgRGB (0x00, 200, 0x00);
    }

} // namespace sspo
