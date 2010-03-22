/* * This file is part of dui-keyboard *
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



#include "ut_vkbdatakey.h"
#include <vkbdatakey.h>

#include <DuiApplication>
#include <DuiTheme>

#include <QSignalSpy>

void Ut_VKBDataKey::initTestCase()
{
    static int argc = 1;
    static char *app_name[1] = { (char *) "ut_vkbdatakey" };

    // Avoid waiting if im server is not responding
    DuiApplication::setLoadDuiInputContext(false);
    app = new DuiApplication(argc, app_name);
    DuiTheme::instance()->loadCSS("/usr/share/dui/virtual-keyboard/css/864x480.css");
}

void Ut_VKBDataKey::cleanupTestCase()
{
    delete app;
    app = 0;
}

void Ut_VKBDataKey::init()
{
    subject = new VKBDataKey;
}

void Ut_VKBDataKey::cleanup()
{
    delete subject;
    subject = 0;
}

void Ut_VKBDataKey::testCreate()
{
    QVERIFY(subject != 0);
}

void Ut_VKBDataKey::testAccent()
{
    const QString label = "a";
    QChar accentedChars[] = { 0xE0, 0xE1, };
    QChar accents[] = { 0x60, 0xB4, 0x5E, };
    QList<QChar> testAccents;
    QList<QChar> testExpected;

    testAccents
            << accents[0]   //set valid accent
            << QChar()      //set empty accent
            << accents[2]   //set invalid accent: it is not defined for this key
            << QChar(0xA8); //set invalid accent: there is no correponding label

    testExpected
            << accentedChars[0]
            << label[0]
            << label[0]
            << label[0];

    KeyBinding *noShiftBinding = new KeyBinding;
    subject->bindings[VKBDataKey::NoShift] = noShiftBinding;
    noShiftBinding->keyLabel = label;
    noShiftBinding->accentedLabels = QString(accentedChars,
                                     sizeof(accentedChars) / sizeof(accentedChars[0]));
    noShiftBinding->accents = QString(accents,
                                      sizeof(accents) / sizeof(accents[0]));

    for (int i = 0; i < testAccents.count(); ++i) {
        QCOMPARE(subject->binding(false)->accented(testAccents.at(i)).at(0),
                 testExpected.at(i));
        QCOMPARE(subject->toKeyEvent(QEvent::KeyRelease, testAccents.at(i), false).text().at(0),
                 testExpected.at(i));
    }
    QCOMPARE(subject->binding(true), static_cast<KeyBinding *>(0));
}

QTEST_APPLESS_MAIN(Ut_VKBDataKey);

