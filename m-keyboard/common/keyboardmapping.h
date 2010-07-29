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

#ifndef KEYBOARDMAPPING_H
#define KEYBOARDMAPPING_H

class QString;

enum HardwareKeyboardLayout {
    CommonEnglish,
    ArabicLatin,
    ChineseHK,
    ChinesePrcSeap,
    ChineseTw,
    Czech,
    DanishNorwegian,
    FinnishSwedish,
    French,
    German,
    Italian,
    Portuguese,
    RussianLatin,
    Spanish,
    Swiss,

    NumberOfHardwareKeyboardTypes,
    InvalidHardwareKeyboard = NumberOfHardwareKeyboardTypes
};

enum HardwareSymbolVariant {
    HwSymbolVariantUs,
    HwSymbolVariantEuro,
    HwSymbolVariantArabic,
    HwSymbolVariantChinese,

    NumberOfHardwareSymbolVariants
};

// The array below follows mapping as defined in
// table 3 in Hardware Keyboard UI Specification.
static const HardwareSymbolVariant HwkbLayoutToSymVariant[NumberOfHardwareKeyboardTypes] = {
    HwSymbolVariantUs,      // CommonEnglish
    HwSymbolVariantArabic,  // ArabicLatin
    HwSymbolVariantChinese, // ChineseHK,
    HwSymbolVariantChinese, // ChinesePrcSeap,
    HwSymbolVariantChinese, // ChineseTw,
    HwSymbolVariantEuro,    // Czech,
    HwSymbolVariantEuro,    // DanishNorwegian,
    HwSymbolVariantEuro,    // FinnishSwedish,
    HwSymbolVariantEuro,    // French,
    HwSymbolVariantEuro,    // German,
    HwSymbolVariantEuro,    // Italian,
    HwSymbolVariantEuro,    // Portuguese,
    HwSymbolVariantEuro,    // RussianLatin,
    HwSymbolVariantEuro,    // Spanish,
    HwSymbolVariantEuro     // Swiss,
};

//! Maps xkb layout name to HardwareKeyboardLayout enum. If xkb name
//! is not known CommonEnglish is returned as default.
HardwareKeyboardLayout xkbLayoutType(const QString &xkbLayoutName);

#endif // KEYBOARDMAPPING_H
