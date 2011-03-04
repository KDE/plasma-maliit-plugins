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



#include "ut_mimkeymodel.h"
#include "utils.h"
#include <mimkeymodel.h>

#include <MApplication>
#include <MTheme>

#include <QSignalSpy>
#include <QDebug>


Q_DECLARE_METATYPE(Qt::Key)


void Ut_MImKeyModel::initTestCase()
{
    static int argc = 2;
    static char *app_name[2] = { (char *) "ut_mimkeymodel",
                                 (char *) "-software" };

    disableQtPlugins();
    app = new MApplication(argc, app_name);
}

void Ut_MImKeyModel::cleanupTestCase()
{
    delete app;
    app = 0;
}

void Ut_MImKeyModel::init()
{
    subject = new MImKeyModel;
}

void Ut_MImKeyModel::cleanup()
{
    delete subject;
    subject = 0;
}

void Ut_MImKeyModel::testCreate()
{
    QVERIFY(subject != 0);
}

void Ut_MImKeyModel::testAccent()
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

    MImKeyBinding *noShiftBinding = new MImKeyBinding;
    subject->setBinding(*noShiftBinding, false);
    noShiftBinding->keyLabel = label;
    noShiftBinding->accented_labels = QString(accentedChars,
                                     sizeof(accentedChars) / sizeof(accentedChars[0]));
    noShiftBinding->accents = QString(accents,
                                      sizeof(accents) / sizeof(accents[0]));

    for (int i = 0; i < testAccents.count(); ++i) {
        QCOMPARE(subject->binding(false)->accented(testAccents.at(i)).at(0),
                 testExpected.at(i));
        QCOMPARE(subject->toKeyEvent(QEvent::KeyRelease, testAccents.at(i), false).text().at(0),
                 testExpected.at(i));
    }
    QCOMPARE(subject->binding(true), static_cast<MImKeyBinding *>(0));
}

void Ut_MImKeyModel::testKeyCode_data()
{
    QTest::addColumn<QString>("label");
    QTest::addColumn<Qt::Key>("keyCode");
    QTest::newRow("a") << "a" << Qt::Key_A;
    QTest::newRow("d") << "d" << Qt::Key_D;
    QTest::newRow("o") << "o" << Qt::Key_O;
    QTest::newRow("b") << "b" << Qt::Key_B;
    QTest::newRow("e") << "e" << Qt::Key_E;
}

void Ut_MImKeyModel::testKeyCode()
{
    QFETCH(QString, label);
    QFETCH(Qt::Key, keyCode);

    MImKeyBinding *noShiftBinding = new MImKeyBinding;
    subject->setBinding(*noShiftBinding, false);
    noShiftBinding->keyLabel = label;

    QCOMPARE(static_cast<Qt::Key>(subject->toKeyEvent(QEvent::KeyPress, false).toQKeyEvent().key()),
             keyCode);
}

void Ut_MImKeyModel::testOverrideBinding()
{
    MImKeyBinding *defaultBinding = new MImKeyBinding("default");
    subject->setBinding(*defaultBinding, false);
    subject->setBinding(*defaultBinding, true);
    QVERIFY(subject->binding(false)->keyLabel == "default");
    QVERIFY(subject->binding(true)->keyLabel == "default");

    // Set override and check
    MImKeyBinding override("override");
    subject->overrideBinding(&override, false);
    QVERIFY(subject->binding(false)->keyLabel == "override");
    QVERIFY(subject->binding(true)->keyLabel == "default");

    // remove override and check
    subject->overrideBinding(0, false);
    QVERIFY(subject->binding(false)->keyLabel == "default");
    QVERIFY(subject->binding(true)->keyLabel == "default");
}

QTEST_APPLESS_MAIN(Ut_MImKeyModel);

