/*
 * This file is part of meego-keyboard 
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list 
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list 
 * of conditions and the following disclaimer in the documentation and/or other materials 
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be 
 * used to endorse or promote products derived from this software without specific 
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL 
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#include "keyboardmapping.h"

#include <QString>

struct XkbLookupPair
{
    const char *xkbName;
    HardwareKeyboardLayout type;
};

// This table maps xkb layout names to hwkb layout types that are used in MeeGo Touch.
const int NumberOfXkbLayoutNames = 10;
static const XkbLookupPair xkbTypeLookupTable[NumberOfXkbLayoutNames] = {
    { "us", CommonEnglish },
    { "ara", ArabicLatin },
    { "cn", ChinesePrcSeap },
    { "da", DanishNorwegian },
    { "fi", FinnishSwedish },
    { "fr", French },
    { "de", German },
    { "it", Italian },
    { "ru", RussianLatin },
    { "es", Spanish }
};

HardwareKeyboardLayout xkbLayoutType(const QString &xkbLayoutName)
{
    HardwareKeyboardLayout type = CommonEnglish;

    for (int i = 0; i < NumberOfXkbLayoutNames; ++i) {
        if (xkbLayoutName == xkbTypeLookupTable[i].xkbName) {
            type = xkbTypeLookupTable[i].type;
            break;
        }
    }
    return type;
}
