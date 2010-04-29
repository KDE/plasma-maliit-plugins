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



#include "ut_toolbarmanager.h"
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
}

void Ut_ToolbarManager::cleanupTestCase()
{
    delete app;
    app = 0;
}

void Ut_ToolbarManager::init()
{
}

void Ut_ToolbarManager::cleanup()
{
}

void Ut_ToolbarManager::testLoadToolbar()
{
    std::auto_ptr<ToolbarManager> subject(new ToolbarManager);
    QList<qlonglong> toolbarIds;
    for (qlonglong i = 1; i <= 14; i ++)
        toolbarIds <<  i;
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
    // register all toolbars
    for (int i = 0; i < toolbarIds.count(); i++) {
        subject->registerToolbar(toolbarIds.at(i), toolbars.at(i));
        toolbarCount ++;
        QTest::qWait(50);
        //toolbar loop can only cache no more than MaximumToolbarCount toolbars
        if (toolbarCount < MaximumToolbarCount)
            QCOMPARE(subject->toolbarList().count(), toolbarCount);
        else
            QCOMPARE(subject->toolbarList().count(), MaximumToolbarCount);
    }

    for (int i = 0; i < toolbarIds.count(); i++) {
        QVERIFY(subject->loadToolbar(toolbarIds.at(i)));
        QTest::qWait(50);
        // the loaded toolbar is the current toolbar
        QCOMPARE(subject->currentToolbar(), toolbarIds.at(i));
        QVERIFY(subject->toolbarList().contains(toolbarIds.at(i)));
        if (i > MaximumToolbarCount) {
            // the rarely used toolbar will be removed from cached toolbars
            QVERIFY(!subject->toolbarList().contains(toolbarIds.at(i - MaximumToolbarCount)));
        }
    }

    toolbarCount = toolbarIds.count();
    foreach (qlonglong id, toolbarIds) {
        QVERIFY(subject->toolbars.contains(id));
        subject->unregisterToolbar(id);
        toolbarCount --;
        QCOMPARE(subject->toolbars.count(), toolbarCount);
        QVERIFY(!subject->toolbars.contains(id));
    }
}

QTEST_APPLESS_MAIN(Ut_ToolbarManager);

