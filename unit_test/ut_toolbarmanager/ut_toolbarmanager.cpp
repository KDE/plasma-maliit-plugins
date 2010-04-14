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



#include "ut_toolbarmanager.h"
#include "mvirtualkeyboardstyle.h"
#include <mimtoolbar.h>
#include <toolbarmanager.h>
#include <toolbardata.h>
#include <MApplication>
#include <QDebug>


namespace {
    const int MaximumToolbarCount = 10;
}

// overwrite ToolbarData::loadNokiaToolbarXml to avoid loading actual xml file.
bool ToolbarData::loadNokiaToolbarXml(const QString &fileName)
{
    Q_UNUSED(fileName);
    toolbarFileName = fileName;
    return true;
}

void Ut_ToolbarManager::initTestCase()
{
    // Avoid waiting if im server is not responding
    MApplication::setLoadMInputContext(false);

    static char *argv[1] = {(char *) "ut_toolbarmanager"};
    static int argc = 1;
    app = new MApplication(argc, argv);
    style = new MVirtualKeyboardStyleContainer;
    style->initialize("MVirtualKeyboard", "MVirtualKeyboardView", 0);
    m_parent = new MImToolbar(*style);
}

void Ut_ToolbarManager::cleanupTestCase()
{
    delete m_parent;
    m_parent = 0;
    delete style;
    style = 0;
    delete app;
    app = 0;
}

void Ut_ToolbarManager::init()
{
    subject = new ToolbarManager(m_parent);
}

void Ut_ToolbarManager::cleanup()
{
    delete subject;
    subject = 0;
}

void Ut_ToolbarManager::testLoadToolbar()
{
    QStringList toolbars;
    toolbars << "testToolbar1"
             << "testToolbar2"
             << "testToolbar3"
             << "testToolbar4"
             << "testToolbar5"
             << "testToolbar6"
             << "testToolbar7"
             << "testToolbar8"
             << "testToolbar9"
             << "testToolbar10"
             << "testToolbar11"
             << "testToolbar12"
             << "testToolbar13"
             << "testToolbar14";
    int toolbarCount = 0;
    foreach (const QString &toolbar, toolbars) {
        QVERIFY(subject->loadToolbar(toolbar));
        toolbarCount ++;
        QTest::qWait(50);
        //toolbar loop can only cache no more than MaximumToolbarCount toolbars
        if (toolbarCount < MaximumToolbarCount)
            QCOMPARE(subject->toolbarList().count(), toolbarCount);
        else
            QCOMPARE(subject->toolbarList().count(), MaximumToolbarCount);
    }
    //the most rarely used toolbar will be removed if reach MaximumToolbarCount
    QVERIFY(subject->toolbarList().count() == MaximumToolbarCount);
    QVERIFY(!subject->toolbarList().contains(toolbars.at(0)));
    QVERIFY(!subject->toolbarList().contains(toolbars.at(1)));
    QVERIFY(!subject->toolbarList().contains(toolbars.at(2)));
    QVERIFY(!subject->toolbarList().contains(toolbars.at(3)));
    for (int i = 4; i < toolbars.count(); i++)
        QVERIFY(subject->toolbarList().contains(toolbars.at(i)));
}

QTEST_APPLESS_MAIN(Ut_ToolbarManager);

