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

#include "AudioMath.h"

#include <assert.h>
#include <iostream>
extern int _mdb;        // MIDI reverence count

/**
 * Our own little assert library, loosely inspired by Chai Assert.
 *
 * Will print information on failure, then generate a "real" assertion
 */


#define assertEQEx(actual, expected, msg) if (actual != expected) { \
    std::cout << "assertEq failed " << msg << " actual value =" << \
    actual << " expected=" << expected << std::endl; \
    assert(false); }

#define assertEQ(actual, expected) assertEQEx(actual, expected, "")

#define assertNEEx(actual, expected, msg) if (actual == expected) { \
    std::cout << "assertNE failed " << msg << " did not expect " << \
    actual << " to be == to " << expected << std::endl; \
    assert(false); }

#define assertNE(actual, expected) assertNEEx(actual, expected, "")

#define assertClose(actual, expected, diff) if (!AudioMath::closeTo(actual, expected, diff)) { \
    std::cout << "assertClose failed actual value =" << \
    actual << " expected=" << expected << std::endl << std::flush; \
    assert(false); }


// assert less than
#define assertLT(actual, expected) if ( actual >= expected) { \
    std::cout << "assertLt " << expected << " actual value = " << \
    actual << std::endl ; \
    assert(false); }

// assert less than or equal to
#define assertLE(actual, expected) if ( actual > expected) { \
    std::cout << "assertLE " << expected << " actual value = " << \
    actual << std::endl ; \
    assert(false); }

// assert greater than 
#define assertGT(actual, expected) if ( actual <= expected) { \
    std::cout << "assertGT " << expected << " actual value = " << \
    actual << std::endl ; \
    assert(false); }
// assert greater than or equal to
#define assertGE(actual, expected) if ( actual < expected) { \
    std::cout << "assertGE " << expected << " actual value = " << \
    actual << std::endl ; \
    assert(false); }

#ifndef NDEBUG
#define assertEvCount(x) assertEQ(MidiEvent::_count, x)
#define assertNoMidi() assertEvCount(0); assertEQ(_mdb, 0)
#else
#define assertEvCount(x)  ((void)0)
#define assertNoMidi()  ((void)0)
#endif
// leave space after macro