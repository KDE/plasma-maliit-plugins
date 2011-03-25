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



#ifndef UT_MVIRTUALKEYBOARD_H
#define UT_MVIRTUALKEYBOARD_H

#include "layoutdata.h"
#include <QObject>
#include <QSharedPointer>
#include <QtTest/QTest>

class MApplication;
class MVirtualKeyboard;
class MVirtualKeyboardStyleContainer;
class MImAbstractKeyArea;
class QGraphicsScene;
class LayoutSection;
class MSceneWindow;


class Ut_MVirtualKeyboard : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    MVirtualKeyboard *m_vkb;
    MSceneWindow *vkbParent;
    MVirtualKeyboardStyleContainer *vkbStyleContainer;
    const LayoutData::SharedLayoutSection functionkeySection;
    int numFunctionKeys;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void clickBackspaceTest();
    void clickSpaceTest();
    void setShiftStateTest();
    void clickHyphenTest();
    void clickPunctQuesTest();
    void clickPunctDotTest();
    void testStateReset();
    void testShiftLevelChange_data();
    void testShiftLevelChange();
    void flickRightHandlerTest();
    void flickLeftHandlerTest();
    void loadSymbolViewTemporarilyTest();
    void errorCorrectionTest();
    void setKeyboardType();
    void longPressBackSpace();
    void bug_130644();
    void symbolKeyTestLowercase();
    void symbolKeyTestCapsLock();
    void interceptPress();
    void bug_137295();
    void testReactionMaps();
    void flickUpHandlerTest_data();
    void flickUpHandlerTest();

signals:
    void fade(int);

private:
    void rotateToAngle(M::OrientationAngle angle);
};

#endif

