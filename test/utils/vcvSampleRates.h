/*
* Copyright (c) 2023 Dave French <contact/dot/dave/dot/french3/at/googlemail/dot/com>
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

#include <array>

namespace Sspo
{
    static std::array<float, 14> sampleRates = { 11025.0f,
                                                 12000.0f,
                                                 22050.0f,
                                                 24000.0f,
                                                 44100.0f,
                                                 48000.0f,
                                                 88200.0f,
                                                 96000.0f,
                                                 176400.0f,
                                                 192000.0f,
                                                 352800.0f,
                                                 384000.0f,
                                                 705600.0f,
                                                 768000.0f };
}