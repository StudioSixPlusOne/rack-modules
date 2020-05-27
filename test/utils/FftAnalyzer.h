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

#include <assert.h>

#include "asserts.h"
#include "AudioMath.h"
#include "FFT.h"
#include "FFTData.h"

using namespace sspo::AudioMath;

namespace sspo
{
    namespace FftAnalyzer
    {
        struct FilterSlope
        {
            float cornerGain = 0.0f;
            float slope = 0.0;
        };

        inline FilterSlope getSlopeLowpass (const FFTDataCpx& response,
                                            const FFTDataCpx driacResponse,
                                            float fc,
                                            float sr)
        {
            FilterSlope ret;
            int cornerBin = FFT::freqToBin (fc, sr, response.size());
            int slopeBin = cornerBin * 4; //measure two octave
            assert (slopeBin < response.size());
            float driacCornerResponse = driacResponse.getAbs (cornerBin);
            float driacSlopeResponse = driacResponse.getAbs (slopeBin);
            float cornerResponse = response.getAbs (cornerBin);
            float slopeResponse = response.getAbs (slopeBin);
            ret.cornerGain = float (db (cornerResponse) - db (driacCornerResponse));
            ret.slope = float (db (slopeResponse) - db (driacSlopeResponse)) / 2.0f;
            return ret;
        }

        inline FilterSlope getSlopeHighpass (const FFTDataCpx& response,
                                             const FFTDataCpx& driacResponse,
                                             float fc,
                                             float sr)
        {
            FilterSlope ret;
            int cornerBin = FFT::freqToBin (fc, sr, response.size());
            int slopeBin = cornerBin / 4; //measure two octave
            assert (slopeBin > 2);
            float driacCornerResponse = driacResponse.getAbs (cornerBin);
            float driacSlopeResponse = driacResponse.getAbs (slopeBin);
            float cornerResponse = response.getAbs (cornerBin);
            float slopeResponse = response.getAbs (slopeBin);
            ret.cornerGain = float (db (cornerResponse) - db (driacCornerResponse));
            ret.slope = float (db (slopeResponse) - db (driacSlopeResponse)) / 2.0f;
            return ret;
        }

    } // namespace FftAnalyzer
} // namespace sspo