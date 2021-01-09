/*
 * Copyright (c) 2019, 2021 Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>
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
namespace sspo
{
    class ClockDuration
    {
    public:
        int process (float clock)
        {
            if (syncTrigger.process (clock))
            {
                framesSinceLastTrigger++;
                lastClockDuration = framesSinceLastTrigger;
                framesSinceLastTrigger = 0;
            }
            else
            {
                framesSinceLastTrigger++;
            }
            return lastClockDuration;
        }

    private:
        int lastClockDuration = 1000; //number if samples between two last concecutaive rising edge
        int framesSinceLastTrigger;
        dsp::SchmittTrigger syncTrigger;
    };
} // namespace sspo