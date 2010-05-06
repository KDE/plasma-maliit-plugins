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

#include "ut_keyeventhandler.h"
#include "keyeventhandler.h"
#include "mvirtualkeyboardstyle.h"
#include "singlewidgetbuttonarea.h"
#include "singlewidgetbutton.h"
#include "flickupbutton.h"
#include "mbuttonarea.h"
#include "keyboarddata.h"
#include "vkbdatakey.h"
#include "mplainwindow.h"
#include "popupbase.h"

#include <MApplication>
#include <MScene>
#include <MSceneManager>
#include <MTheme>

#include <QDir>
#include <QGraphicsLayout>
#include <QGraphicsSceneMouseEvent>
#include <QTouchEvent>

namespace
{
//    const int LongPressTime = 1000; // same as in keybuttonarea.cpp
}

Q_DECLARE_METATYPE(KeyEvent);
Q_DECLARE_METATYPE(IKeyButton*);


void Ut_KeyEventHandler::initTestCase()
{
    static int argc = 1;
    static char *app_name[1] = { (char *) "ut_keybuttonarea" };

    // Avoid waiting if im server is not responding
    MApplication::setLoadMInputContext(false);
    app = new MApplication(argc, app_name);

    MTheme::instance()->loadCSS("/usr/share/meegotouch/virtual-keyboard/css/864x480.css");
    style = new MVirtualKeyboardStyleContainer;
    style->initialize("MVirtualKeyboard", "MVirtualKeyboardView", 0);

    qRegisterMetaType<KeyEvent>();
    qRegisterMetaType<IKeyButton*>();

    new MPlainWindow; // Create singleton
}

void Ut_KeyEventHandler::cleanupTestCase()
{
    delete MPlainWindow::instance();
    delete style;
    style = 0;
    delete app;
    app = 0;
}

void Ut_KeyEventHandler::init()
{
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en.xml"));
    keyArea = new SingleWidgetButtonArea(style, keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::functionkeySection),
                        KeyButtonArea::ButtonSizeEqualExpanding,
                        false, 0);
    MPlainWindow::instance()->scene()->addItem(keyArea);

    keyArea->resize(defaultLayoutSize());

    subject = new KeyEventHandler();

    space = findKey(KeyBinding::ActionSpace);
    QVERIFY(space);

    shift = findKey(KeyBinding::ActionShift);
    QVERIFY(shift);
}

void Ut_KeyEventHandler::cleanup()
{
    delete subject;
    delete keyArea;
    delete keyboard;
    subject = 0;
    keyArea = 0;
    keyboard = 0;
    space = 0;
    shift = 0;
}

void Ut_KeyEventHandler::testKeyPress()
{
    QSignalSpy spyKeyPressed(subject, SIGNAL(keyPressed(const KeyEvent&)));
    QSignalSpy spyShiftPressed(subject, SIGNAL(shiftPressed(bool)));

    QVERIFY(spyKeyPressed.isValid());
    QVERIFY(spyShiftPressed.isValid());

    subject->handleKeyPress(space, "", false);

    QCOMPARE(spyKeyPressed.count(), 1);
    QCOMPARE(spyShiftPressed.count(), 0);

    subject->handleKeyPress(shift, "", false);

    QCOMPARE(spyKeyPressed.count(), 2);
    QCOMPARE(spyShiftPressed.count(), 1);
    QVERIFY(spyShiftPressed.first().first().toBool() == true);
}

void Ut_KeyEventHandler::testKeyRelease()
{
    QSignalSpy spyKeyReleased(subject, SIGNAL(keyReleased(const KeyEvent&)));
    QSignalSpy spyShiftReleased(subject, SIGNAL(shiftPressed(bool)));

    QVERIFY(spyKeyReleased.isValid());
    QVERIFY(spyShiftReleased.isValid());

    subject->handleKeyPress(space, "", false);
    subject->handleKeyRelease(space, "", false);

    QCOMPARE(spyKeyReleased.count(), 1);
    QCOMPARE(spyShiftReleased.count(), 0);

    subject->handleKeyRelease(shift, "", false);

    QCOMPARE(spyKeyReleased.count(), 2);
    QCOMPARE(spyShiftReleased.count(), 0);

    subject->handleKeyPress(shift, "", false);
    spyShiftReleased.clear();
    subject->handleKeyRelease(shift, "", false);

    QCOMPARE(spyKeyReleased.count(), 3);
    QCOMPARE(spyShiftReleased.count(), 1);
    QVERIFY(spyShiftReleased.first().first().toBool() == false);
}

void Ut_KeyEventHandler::testKeyClick()
{
    QSignalSpy spyKeyClicked(subject, SIGNAL(keyClicked(const KeyEvent&)));

    QVERIFY(spyKeyClicked.isValid());

    subject->handleKeyPress(shift, "", false);
    subject->handleKeyRelease(shift, "", false);
    subject->handleKeyClick(shift, "", false);

    QCOMPARE(spyKeyClicked.count(), 1);
    QVERIFY(spyKeyClicked.first().first().value<KeyEvent>().qtKey() == Qt::Key_Shift);
    spyKeyClicked.clear();

    subject->handleKeyPress(shift, "", false);
    subject->handleKeyPress(space, "", false);
    subject->handleKeyRelease(space, "", false);
    subject->handleKeyClick(space, "", false);
    subject->handleKeyRelease(shift, "", false);
    subject->handleKeyClick(shift, "", false);

    QCOMPARE(spyKeyClicked.count(), 1);
    QVERIFY(spyKeyClicked.first().first().value<KeyEvent>().qtKey() != Qt::Key_Shift);
}

QSize Ut_KeyEventHandler::defaultLayoutSize()
{
    // Take visible scene size as layout size, but reduce keyboard's paddings first from its width.
    // The height value is ignored since KeyButtonAreas determine their own height.
    return MPlainWindow::instance()->visibleSceneSize()
            - QSize((*style)->paddingLeft() + (*style)->paddingRight(), 0);
}

// Helper method to get key
const IKeyButton *Ut_KeyEventHandler::findKey(KeyBinding::KeyAction action)
{
    SingleWidgetButtonArea *buttonArea = dynamic_cast<SingleWidgetButtonArea *>(keyArea);
    Q_ASSERT(buttonArea);

    foreach (const SingleWidgetButtonArea::ButtonRow &row, buttonArea->rowList) {
        foreach (SingleWidgetButton *button, row.buttons) {
            if (button->binding().action() == action) {
                return button;
            }
        }
    }
    return 0;
}


QTEST_APPLESS_MAIN(Ut_KeyEventHandler);
