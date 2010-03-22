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



#ifndef UT_DUIIMTOOLBAR
#define UT_DUIIMTOOLBAR

#include <QtTest/QTest>
#include <QObject>
class DuiApplication;
class DuiImToolbar;
class QKeyEvent;
class DuiVirtualKeyboardStyleContainer;

class Ut_DuiImToolbar : public QObject
{
    Q_OBJECT

private:
    DuiApplication *app;
    DuiImToolbar *m_subject;
    int keyEvents;
    DuiVirtualKeyboardStyleContainer *style;

private slots:
    //! initialize application and class
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();
    void testCopyPasteButton();

    void testShowToolbarWidget();
    void testHideToolbarWidget();
    void testShowGroup();
    void testHideGroup();
    void testSendString();
    void testKeySequenceString();
    void testCopy();
    void testPaste();
    void testRegion();
    void testShowHideIndicatorButton();
    void testIndicatorButton();
    void testSetIndicatorButtonState();

    void receiveKeyEvent(const QKeyEvent &);
};

#endif
