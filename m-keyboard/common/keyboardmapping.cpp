/* * This file is part of meego-keyboard *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
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
