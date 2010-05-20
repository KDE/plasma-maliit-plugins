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



#ifndef UT_MIMTOOLBAR
#define UT_MIMTOOLBAR

#include <QtTest/QTest>
#include <QObject>
class MApplication;
class MImToolbar;
class QKeyEvent;
class MVirtualKeyboardStyleContainer;

class Ut_MImToolbar : public QObject
{
    Q_OBJECT

private:
    MApplication *app;
    MImToolbar *m_subject;
    int keyEvents;
    MVirtualKeyboardStyleContainer *style;

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
    void testSetToolbarItemAttribute();

    void receiveKeyEvent(const QKeyEvent &);
};

#endif
