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

// An empty test, can be used as a template

#include <assert.h>
#include <stdio.h>
#include "TriggerSequencer.h"

static void testTrue()
{
    assert (true && "Test true");
}

static void testFalse()
{
    assert (! false && "Test false");
}

void testTriggerSequencer()
{
    printf ("test Trigger Sequencer\n");
    //    testTrue();
    //    testFalse();
}
