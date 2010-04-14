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



#ifndef UT_SYMBOLVIEW_H
#define UT_SYMBOLVIEW_H

#include <QtTest/QTest>
#include <QObject>

#include <MNamespace>

class MApplication;
class MSceneWindow;
class MVirtualKeyboard;
class MVirtualKeyboardStyleContainer;
class SymbolView;

class Ut_SymbolView : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    MVirtualKeyboardStyleContainer *style;
    SymbolView *subject;
    MSceneWindow *parent;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testReactiveButtonAreas_data();
    void testReactiveButtonAreas();
    void testReactiveWholeScreen_data();
    void testReactiveWholeScreen();

    void testOthers();
    void testChangeToOpenMode();
    void testChangeTab_data();
    void testChangeTab();
    void testHideWithFlick_data();
    void testHideWithFlick();

private:
    void rotateToAngle(M::OrientationAngle angle);
};

#endif
