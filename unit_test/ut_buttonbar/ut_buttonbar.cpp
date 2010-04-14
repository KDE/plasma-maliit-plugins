/* * This file is part of m-keyboard *
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

#include "ut_buttonbar.h"
#include "buttonbar.h"

#include <MApplication>
#include <MButton>

#include <QGraphicsLayout>

void Ut_ButtonBar::initTestCase()
{
    MApplication::setLoadMInputContext(false);
    static char *argv[1] = { (char *) "ut_buttonbar" };
    static int argc = 1;
    app = new MApplication(argc, argv);
}

void Ut_ButtonBar::cleanupTestCase()
{
    delete app;
}

void Ut_ButtonBar::init()
{
    subject = new ButtonBar(true);
}

void Ut_ButtonBar::cleanup()
{
    delete subject;
    subject = 0;
}

// Just a simple sanity test for adding buttons.
void Ut_ButtonBar::testInsert_data()
{
    QTest::addColumn<int>("initialButtonCount");
    QTest::addColumn<int>("insertionIndex");

    QTest::newRow("First insertion") << 0 << 0;
    QTest::newRow("Prepend") << 3 << 0;
    QTest::newRow("Append") << 3 << 3;
    QTest::newRow("Middle insertion") << 3 << 1;

    QTest::newRow("Out of bounds 1") << 0 << 1;
    QTest::newRow("Out of bounds 2") << 1 << 2;
    QTest::newRow("Out of bounds 3") << 1 << 3;
    QTest::newRow("Negative index")  << 3 << -2;
}

void Ut_ButtonBar::testInsert()
{
    QFETCH(int, initialButtonCount);
    QFETCH(int, insertionIndex);

    Q_ASSERT(initialButtonCount >= 0);

    QList<MButton *> buttons;
    for (int i = 0; i < initialButtonCount; ++i) {
        MButton *button = new MButton;
        buttons.append(button);
        subject->insert(subject->count(), button);
    }

    MButton *button = new MButton;
    subject->insert(insertionIndex, button);

    // Insertion happens only if we have valid index which is
    // 0 <= index <= buttonCount.
    if (insertionIndex >= 0 && insertionIndex <= initialButtonCount) {
        buttons.insert(insertionIndex, button);
    }

    QCOMPARE(subject->count(), buttons.count());
    for (int i = 0; i < initialButtonCount; ++i) {
        QCOMPARE(subject->buttonAt(i), buttons.at(i));
    }

    qDeleteAll(buttons);
}

void Ut_ButtonBar::testRemove_data()
{
    QTest::addColumn<int>("initialButtonCount");
    QTest::addColumn<int>("removeIndex");

    // Invalid removeIndex is converted to NULL pointer before testing.
    QTest::newRow("Invalid 1") << 0 << 0;
    QTest::newRow("Invalid 2") << 3 << 5;
    QTest::newRow("First")     << 3 << 0;
    QTest::newRow("Middle")    << 3 << 1;
    QTest::newRow("End")       << 3 << 2;
}

void Ut_ButtonBar::testRemove()
{
    QFETCH(int, initialButtonCount);
    QFETCH(int, removeIndex);

    Q_ASSERT(initialButtonCount >= 0);

    QList<MButton *> buttons;
    for (int i = 0; i < initialButtonCount; ++i) {
        MButton *button = new MButton;
        buttons.append(button);
        subject->append(button);
    }
    QCOMPARE(subject->count(), buttons.count());

    MButton *buttonToRemove = 0;
    if (removeIndex >= 0 && removeIndex < buttons.count()) {
        buttonToRemove = buttons.at(removeIndex);
        buttons.removeAt(removeIndex);
    }

    subject->remove(buttonToRemove);

    // Make sure contents match.
    QCOMPARE(subject->count(), buttons.count());
    for (int i = 0; i < buttons.count(); ++i) {
        QCOMPARE(subject->buttonAt(i), buttons.at(i));
    }

    qDeleteAll(buttons);
    if (buttonToRemove) {
        delete buttonToRemove;
        buttonToRemove = 0;
    }
}


void Ut_ButtonBar::testLayoutContent_data()
{
    QTest::addColumn<int>("buttonCount");

    // Number of layout items is number of buttons + number of dividers.
    QTest::newRow("No buttons")   << 0;
    QTest::newRow("One button")   << 1;
    QTest::newRow("Two buttons")  << 2;
    QTest::newRow("Three buttons") << 3;
}

void Ut_ButtonBar::testLayoutContent()
{
    QFETCH(int, buttonCount);

    QVector<MButton *> buttons(buttonCount);

    for (int i = 0; i < buttonCount; ++i) {
        MButton *button = new MButton;
        buttons[i] = button;
        subject->insert(subject->count(), button);
    }

    QCOMPARE(subject->layout()->count(), buttonCount);

    // Skip dividers and check that buttons match.
    for (int i = 0; i < buttonCount; ++i) {
        QCOMPARE(subject->layout()->itemAt(i),
                 static_cast<QGraphicsLayoutItem *>(buttons[i]));
    }

    qDeleteAll(buttons);
}


void Ut_ButtonBar::testIndexOf()
{
    MButton *button1 = new MButton;
    MButton *button2 = new MButton;
    MButton *button3 = new MButton;
    int index = -1;

    subject->insert(0, button1);
    subject->insert(1, button2);

    index = subject->indexOf(button1);
    QCOMPARE(index, 0);

    index = subject->indexOf(button2);
    QCOMPARE(index, 1);

    index = subject->indexOf(button3);
    QCOMPARE(index, -1);

    index = subject->indexOf(0);
    QCOMPARE(index, -1);
}


QTEST_APPLESS_MAIN(Ut_ButtonBar);
