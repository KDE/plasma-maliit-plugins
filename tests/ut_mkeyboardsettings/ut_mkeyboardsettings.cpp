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



#include "ut_mkeyboardsettings.h"
#include <QDebug>
#include <mgconfitem_stub.h>
#include <mkeyboardsettings.h>
#include <memory>

namespace
{
    const QString SettingsIMCorrectionSetting("/meegotouch/inputmethods/correctionenabled");
    const QString InputMethodLanguages("/meegotouch/inputmethods/languages");
};

// Test init/deinit..........................................................

void Ut_MKeyboardSettings::initTestCase()
{
}

void Ut_MKeyboardSettings::cleanupTestCase()
{
}

void Ut_MKeyboardSettings::init()
{
    MGConfItem layoutListSetting(InputMethodLanguages);
    QStringList layoutlist;
    layoutlist << "fi" << "ru" << "ar";
    layoutListSetting.set(QVariant(layoutlist));
}

void Ut_MKeyboardSettings::cleanup()
{
}

// Tests.....................................................................

void Ut_MKeyboardSettings::testNoCrash()
{
    std::auto_ptr<MKeyboardSettings> subject(new MKeyboardSettings);
    subject->readAvailableKeyboards();
}

void Ut_MKeyboardSettings::testAvailableKeyboards()
{
    std::auto_ptr<MKeyboardSettings> subject(new MKeyboardSettings);
    QVERIFY(subject->availableKeyboards().count() > 0);

    QStringList layoutList;
    layoutList << "fi" << "ru" << "ar" << "en_gb";
    foreach (const QString &layout, layoutList) {
        QVERIFY(subject->availableKeyboards().keys().contains(layout));
    }
}

void Ut_MKeyboardSettings::testSelectedKeyboards()
{
    MGConfItem layoutListSetting(InputMethodLanguages);
    QStringList layoutList;
    layoutList << "fi" << "ru" << "ar" << "en_gb";
    layoutListSetting.set(QVariant(layoutList));
    std::auto_ptr<MKeyboardSettings> subject(new MKeyboardSettings);
    foreach (const QString &layout, layoutList) {
        QVERIFY(subject->selectedKeyboards().keys().contains(layout));
    }
}

void Ut_MKeyboardSettings::testSetSelectedKeyboards()
{
    QStringList oldLayouts;
    oldLayouts << "fi" << "ar" << "en_gb";
    MGConfItem layoutListSetting(InputMethodLanguages);
    layoutListSetting.set(QVariant(oldLayouts));

    std::auto_ptr<MKeyboardSettings> subject(new MKeyboardSettings);
    QMap<QString, QString> availableKeyboards = subject->availableKeyboards();
    QVERIFY(availableKeyboards.count() > 3);

    QStringList newLayouts = availableKeyboards.keys();
    QStringList layoutTitles = availableKeyboards.values();
    //set all available keyboards as installed
    subject->setSelectedKeyboards(layoutTitles);

    QStringList expectedLayouts = layoutListSetting.value().toStringList();
    QVERIFY(expectedLayouts != oldLayouts);
    QCOMPARE(newLayouts.count(), expectedLayouts.count());
    foreach (const QString &layout, newLayouts) {
        expectedLayouts.contains(layout);
    }
}

void Ut_MKeyboardSettings::testErrorCorrection()
{
    MGConfItem errorCorrectionSetting(SettingsIMCorrectionSetting);

    std::auto_ptr<MKeyboardSettings> subject(new MKeyboardSettings);

    errorCorrectionSetting.set(QVariant(false));
    QCOMPARE(subject->errorCorrection(), errorCorrectionSetting.value().toBool());

    errorCorrectionSetting.set(QVariant(true));
    QCOMPARE(subject->errorCorrection(), errorCorrectionSetting.value().toBool());


    subject->setErrorCorrection(true);
    QCOMPARE(errorCorrectionSetting.value().toBool(), true);
    QCOMPARE(subject->errorCorrection(), errorCorrectionSetting.value().toBool());

    subject->setErrorCorrection(false);
    QCOMPARE(errorCorrectionSetting.value().toBool(), false);
    QCOMPARE(subject->errorCorrection(), errorCorrectionSetting.value().toBool());
}

QTEST_APPLESS_MAIN(Ut_MKeyboardSettings);

