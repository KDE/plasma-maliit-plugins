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

#include "ut_widgetbar.h"
#include "widgetbar.h"
#include "mtoolbarbutton.h"
#include "mtoolbarlabel.h"

#include <MApplication>
#include <MButton>
#include <MLabel>

#include <QGraphicsLayout>
#include <mtoolbaritem.h>

void Ut_WidgetBar::initTestCase()
{
    MApplication::setLoadMInputContext(false);
    static char *argv[2] = { (char *) "ut_buttonbar",
                             (char *) "-local-theme" };
    static int argc = 2;
    app = new MApplication(argc, argv);
}

void Ut_WidgetBar::cleanupTestCase()
{
    delete app;
}

void Ut_WidgetBar::init()
{
    subject = new WidgetBar(true);
}

void Ut_WidgetBar::cleanup()
{
    delete subject;
    subject = 0;
}

// Just a simple sanity test for adding buttons.
void Ut_WidgetBar::testInsert_data()
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

void Ut_WidgetBar::testInsert()
{
    QFETCH(int, initialButtonCount);
    QFETCH(int, insertionIndex);

    Q_ASSERT(initialButtonCount >= 0);

    QList<MButton *> buttons;
    for (int i = 0; i < initialButtonCount; ++i) {
        MButton *button = new MButton;
        buttons.append(button);
        subject->insert(subject->count(), button, true);
    }

    MButton *button = new MButton;
    subject->insert(insertionIndex, button, true);

    // Insertion happens only if we have valid index which is
    // 0 <= index <= buttonCount.
    if (insertionIndex >= 0 && insertionIndex <= initialButtonCount) {
        buttons.insert(insertionIndex, button);
    }

    QCOMPARE(subject->count(), buttons.count());
    for (int i = 0; i < initialButtonCount; ++i) {
        QCOMPARE(subject->widgetAt(i), buttons.at(i));
    }

    qDeleteAll(buttons);
}

void Ut_WidgetBar::testRemove_data()
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

void Ut_WidgetBar::testRemove()
{
    QFETCH(int, initialButtonCount);
    QFETCH(int, removeIndex);

    Q_ASSERT(initialButtonCount >= 0);

    QList<MButton *> buttons;
    for (int i = 0; i < initialButtonCount; ++i) {
        MButton *button = new MButton;
        buttons.append(button);
        subject->append(button, true);
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
        QCOMPARE(subject->widgetAt(i), buttons.at(i));
    }

    qDeleteAll(buttons);
    if (buttonToRemove) {
        delete buttonToRemove;
        buttonToRemove = 0;
    }
}


void Ut_WidgetBar::testLayoutContent_data()
{
    QTest::addColumn<int>("buttonCount");

    // Number of layout items is number of buttons + number of dividers.
    QTest::newRow("No buttons")   << 0;
    QTest::newRow("One button")   << 1;
    QTest::newRow("Two buttons")  << 2;
    QTest::newRow("Three buttons") << 3;
}

void Ut_WidgetBar::testLayoutContent()
{
    QFETCH(int, buttonCount);

    QVector<MButton *> buttons(buttonCount);

    for (int i = 0; i < buttonCount; ++i) {
        MButton *button = new MButton;
        buttons[i] = button;
        subject->insert(subject->count(), button, true);
    }

    QCOMPARE(subject->layout()->count(), buttonCount);

    // Skip dividers and check that buttons match.
    for (int i = 0; i < buttonCount; ++i) {
        QCOMPARE(subject->layout()->itemAt(i),
                 static_cast<QGraphicsLayoutItem *>(buttons[i]));
    }

    qDeleteAll(buttons);
}


void Ut_WidgetBar::testIndexOf()
{
    MButton *button1 = new MButton;
    MButton *button2 = new MButton;
    MButton *button3 = new MButton;
    MLabel *label1 = new MLabel;
    int index = -1;

    subject->insert(0, button1, true);
    subject->insert(1, button2, true);
    subject->insert(2, label1, true);

    index = subject->indexOf(button1);
    QCOMPARE(index, 0);

    index = subject->indexOf(button2);
    QCOMPARE(index, 1);

    index = subject->indexOf(label1);
    QCOMPARE(index, 2);

    index = subject->indexOf(button3);
    QCOMPARE(index, -1);

    index = subject->indexOf(0);
    QCOMPARE(index, -1);
}

void Ut_WidgetBar::testLayoutUpdates()
{
    QSharedPointer<MToolbarItem> buttonItem1(new MToolbarItem("buttonItem1", MInputMethod::ItemButton));
    QSharedPointer<MToolbarItem> buttonItem2(new MToolbarItem("buttonItem2", MInputMethod::ItemButton));
    QSharedPointer<MToolbarItem> buttonItem3(new MToolbarItem("buttonItem3", MInputMethod::ItemButton));
    QSharedPointer<MToolbarItem> labelItem(new MToolbarItem("labelItem", MInputMethod::ItemLabel));

    MToolbarButton *button1 = new MToolbarButton(buttonItem1);
    MToolbarButton *button2 = new MToolbarButton(buttonItem2);
    MToolbarButton *button3 = new MToolbarButton(buttonItem3);
    MToolbarLabel *label1 = new MToolbarLabel(labelItem);

    subject->insert(0, button1, true);
    subject->insert(1, button2, true);
    subject->insert(2, label1, true);
    subject->insert(3, button3, true);

    QCOMPARE(subject->layout()->count(), 4);

    labelItem->setVisible(false);
    buttonItem2->setVisible(false);
    buttonItem1->setVisible(false);

    QCOMPARE(subject->layout()->count(), 1);
    QCOMPARE(subject->layoutIndexOf(button3), 0);

    buttonItem2->setVisible(true);
    QCOMPARE(subject->layoutIndexOf(button2), 0);

    labelItem->setVisible(true);
    QCOMPARE(subject->layoutIndexOf(button2), 0);
    QCOMPARE(subject->layoutIndexOf(label1), 1);

    buttonItem1->setVisible(true);
    QCOMPARE(subject->layoutIndexOf(button1), 0);
    QCOMPARE(subject->layoutIndexOf(button2), 1);
    QCOMPARE(subject->layoutIndexOf(label1), 2);
    QCOMPARE(subject->layoutIndexOf(button3), 3);
}

QTEST_APPLESS_MAIN(Ut_WidgetBar);
