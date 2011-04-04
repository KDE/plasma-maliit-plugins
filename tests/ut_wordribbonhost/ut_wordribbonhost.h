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

#ifndef UT_WORDRIBBONHOST_H
#define UT_WORDRIBBONHOST_H

#include <QtTest/QtTest>
#include <QObject>

class MApplication;
class MSceneWindow;
class MButton;
class WordRibbonHost;
class MKeyboardHost;
class MInputMethodHostStub;

class Ut_WordRibbonHost : public QObject
{
    Q_OBJECT


private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testsetCandidates();

    void testShowEngineWidget();
    void testHideEngineWidget();

    void testSetTitle();

    void testSetPageIndex();

    void testFetchCandidates();

    void testHandleNavigationKey();

    void testOpenDialog();
private:
    bool compareCandidates(QStringList totalCandidateList);

    MApplication *app;
    WordRibbonHost *subject;

    MSceneWindow *parentWindow;
    MInputMethodHostStub *inputMethodHost;
    QWidget *mainWindow;
    MKeyboardHost *keyboardHost;

};

#endif // UT_WIDGETBAR_H
