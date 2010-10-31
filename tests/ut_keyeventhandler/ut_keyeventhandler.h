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
#include "mimkeymodel.h"

class MApplication;
class MVirtualKeyboardStyleContainer;
class MImAbstractKeyArea;
class KeyboardData;
class MImAbstractKey;
class KeyEventHandler;

class Ut_KeyEventHandler : public QObject
{
    Q_OBJECT

private:
    MApplication *app;
    MVirtualKeyboardStyleContainer *style;
    MImAbstractKeyArea *keyArea;
    KeyboardData *keyboard;
    KeyEventHandler *subject;
    const MImAbstractKey *space;
    const MImAbstractKey *shift;

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
    const MImAbstractKey *findKey(MImKeyBinding::KeyAction);
};

#endif // UT_KEYEVENTHANDLER_H

