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

#ifndef UT_KEYEVENTHANDLER_H
#define UT_KEYEVENTHANDLER_H

#include <QtTest/QtTest>
#include <QObject>
#include "mnamespace.h"
#include "vkbdatakey.h"

class MApplication;
class MVirtualKeyboardStyleContainer;
class KeyButtonArea;
class KeyboardData;
class IKeyButton;
class KeyEventHandler;

class Ut_KeyEventHandler : public QObject
{
    Q_OBJECT

private:
    MApplication *app;
    MVirtualKeyboardStyleContainer *style;
    KeyButtonArea *keyArea;
    KeyboardData *keyboard;
    KeyEventHandler *subject;
    const IKeyButton *space;
    const IKeyButton *shift;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testKeyPress();
    void testKeyRelease();
    void testKeyClick();

private:
    QSize defaultLayoutSize();
    const IKeyButton *findKey(KeyBinding::KeyAction);
};

#endif // UT_KEYEVENTHANDLER_H

